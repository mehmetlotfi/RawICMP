# http dos 

HTTP-based DDoS attack simulator

**[برای مشاهده توضیحات فارسی اینجا را کلیک کنید](README-fa.md)**


The person executing the code bears responsibility for any misuse; this code is written and released solely for educational purposes regarding TCP (HTTP) socket programming and concurrent programming.

## License

This project is distributed under the **GNU General Public License v3.0 (GPLv3)**.

## Building and Installation

### 1. Check Dependencies

Before compiling the project, run the `configure` script to verify that all required dependencies are installed.

```bash
chmod +x ./configure
./configure
```

If the script reports `OK`, you can continue to the next step.

Otherwise, one or more required components (such as the **GCC compiler** or **POSIX Threads (pthread)** support) are missing or not correctly installed on your system. Resolve the reported issue and run the script again.

### 2. Build

Compile the project by running:

```bash
make
```

### 3. Install (Optional)

To install the compiled binary into your system's executable path, run:

```bash
sudo make install
```

After installation, the program can be executed from any directory.


## Usage

Provide the following parameters to the program:

### thread_count

This argument specifies how many threads should simultaneously send GET packets to the target web server.  
(The higher the number, the greater the destructive power of the program, provided you have sufficient hardware resources.)

### host | ip

This argument specifies the address of the target web server. You can provide either an IP address or a domain name.  
If a domain is given, it will be resolved using the system's default DNS settings. If you provide an IP address, it will be used directly.

### port

This argument specifies the port on which the web server (whose address you provided in the second argument) is listening for HTTP traffic. The attack will be launched against that port. The default port for HTTP is 80 and for HTTPS is 443.

### Example

For example, you can open two different terminals (shells) on your own machine. In the first terminal, run:

```bash
python3 -m http.server
```

This command will serve a directory listing website from the current directory on port 8000, binding to all available network interfaces on your system.
In the second terminal, run your compiled program against 127.0.0.1:8000 with an appropriate number of threads, and monitor your system resources using a system monitoring tool.


### Note
At any time, if you wish to stop the attack, you can send the ```SIGINT``` signal to the program to terminate it. You can send this signal by pressing ```Ctrl+C``` on the running program.
# Mehmet Lotfi
