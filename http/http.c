/*
 *  RawICMP 
 *  Copyright (C) 2026  Mehmet Lotfi
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "arpa/inet.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "unistd.h"
#include "pthread.h"
#include "signal.h"
#include "errno.h"

static volatile sig_atomic_t stop_flag = 0;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned long long total_requests = 0;
static char resolved_ip [INET_ADDRSTRLEN];
static int target_port;

void sigint_handler (int sig)
{
	(void) sig;
	stop_flag = 1;
}

int resolve_host (const char * host , char * ip , size_t size)
{
	struct in_addr addr;
	if (inet_pton (AF_INET , host , &addr) == 1)
	{
		strncpy (ip , host , size - 1);
		ip [size - 1] = 0;
		return 0;
	}
	struct addrinfo hints , * result , * p;
	memset (&hints , 0 , sizeof (hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo (host , NULL , &hints , &result) != 0)
		return -1;
	for (p = result ; p != NULL ; p = p -> ai_next)
	{
		struct sockaddr_in * addr4 = (struct sockaddr_in *) p -> ai_addr;
		if (inet_ntop(AF_INET, &addr4->sin_addr, ip, size) != NULL)
		{
			freeaddrinfo (result);
			return 0;
		}
	}
	freeaddrinfo (result);
	return -1;
}

int connect_host (const char * ip , int port)
{
	int sock = socket (AF_INET , SOCK_STREAM , 0);
	if (sock < 0)
		return -1;
	struct sockaddr_in addr;
	memset (&addr , 0 , sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	if (inet_pton (AF_INET , ip , &addr.sin_addr) != 1)
	{
		close (sock);
		return -1;
	}
	if (connect (sock , (struct sockaddr *) &addr , sizeof (addr)) < 0)
	{
		close (sock);
		return -1;
	}
	return sock;
}

int send_http_get (int sock , const char * host)
{
	char request [1024];
	snprintf (request , sizeof (request) , "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n" , host);
	return send (sock , request , strlen (request) , 0);
}

void * worker_thread (void * arg)
{
	(void) arg;
	while (!stop_flag)
	{
		int sock = connect_host (resolved_ip , target_port);
		if (sock < 0)
		{
			usleep (100);
			continue;
		}
		if (send_http_get (sock , resolved_ip) < 0)
		{
			close (sock);
			continue;
		}
		close (sock);
		pthread_mutex_lock (&count_mutex);
		total_requests++;
		pthread_mutex_unlock (&count_mutex);
	}
	return NULL;
}

void * reporter_thread (void *arg)
{
	(void) arg;
	unsigned long long last_count = 0;
	while (!stop_flag)
	{
		sleep (1);
		if (stop_flag)
			break;
		pthread_mutex_lock (&count_mutex);
		unsigned long long current = total_requests;
		pthread_mutex_unlock (&count_mutex);
		unsigned long long diff = current - last_count;
		last_count = current;
		printf ("\r%llu requests/sec" , diff);
		fflush (stdout);
	}
	return NULL;
}

int main (int argc , char ** argv)
{
	if (argc != 4)
	{
		fprintf (stderr , "[-] usage: %s <thread_count> <host|ip> <port>\n" , argv [0]);
		return 1;
	}
	long thread_count = strtol (argv [1] , NULL , 10);
	if (thread_count <= 0)
	{
		fprintf (stderr , "[-] Invalid thread count\n");
		return 1;
	}
	if (resolve_host (argv [2] , resolved_ip , sizeof (resolved_ip)) != 0)
	{
		fprintf (stderr , "[-] Failed to resolve host\n");
		return 1;
	}
	target_port = atoi (argv [3]);
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction (SIGINT , &sa , NULL) == -1)
	{
		fprintf (stderr , "[-] ERROR in sigaction function\n");
		return 1;
	}
	pthread_t * workers = malloc (sizeof (pthread_t) * thread_count);
	if (!workers)
	{
		fprintf (stderr , "[-] ERROR in malloc function\n");
		return 1;
	}
	pthread_t reporter;
	for (long i = 0 ; i < thread_count; i++)
	{
		if (pthread_create (&workers [i] , NULL , worker_thread , NULL) != 0)
		{
			fprintf (stderr , "[-] ERROR in pthread_create function (works)\n");
			stop_flag = 1;
			for (long j = 0 ; j < i ; j++)
				pthread_join (workers [j] , NULL);
			free (workers);
			return 1;
		}
	}
	if (pthread_create (&reporter , NULL , reporter_thread , NULL) != 0)
	{
		fprintf (stderr , "[-] ERROR in pthread_create function (reporter)\n");
		stop_flag = 1;
		for (long i = 0 ; i < thread_count ; i++)
			pthread_join (workers [i] , NULL);
		free (workers);
		return 1;
	}
	while (!stop_flag)
		pause();
	for (long i = 0 ; i < thread_count ; i++)
		pthread_cancel (workers [i]);
	pthread_cancel (reporter);
	for (long i = 0 ; i < thread_count ; i++)
		pthread_join (workers [i] , NULL);
	pthread_join (reporter , NULL);
	printf ("\nTotal requests: %llu\n" , total_requests);
	free (workers);
	printf ("exiting.\n");
	return 0;
}
