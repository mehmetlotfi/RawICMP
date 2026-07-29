# raw icmp

ICMP-based DDoS attack simulator

**[برای مشاهده توضیحات فارسی اینجا را کلیک کنید](README-fa.md)**


The person executing the code bears responsibility for any misuse; this code is written and released solely for educational purposes regarding ICMP socket programming and concurrent programming.

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

### 3. Install

To install the compiled binary into your system's executable path, run:

```bash
sudo make install
```

After installation, the program can be executed from any directory.


## Usage

To use the program, provide the following parameters:

### target_ip


Destination IP address (target)


### duration_in_minutes


Time of the attack


### thread_count


Number of threads involved
(In this program, each thread individually starts sending ICMP packets to the destination IP.)


# Mehmet Lotfi
