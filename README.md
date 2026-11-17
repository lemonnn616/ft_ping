# ft_ping

**An IPv4 ICMP ping utility written in C, developed for the 42 / Codam ft_ping (v5.1) project.**

`ft_ping` builds ICMP Echo Requests, sends them through a Linux raw socket, validates replies and ICMP errors, measures round-trip time (RTT), and reports packet-loss statistics. GNU **inetutils 2.0** is the behavioral reference for the project.

**C · IPv4 / ICMP · Linux raw sockets · `poll()` · `CLOCK_MONOTONIC`**

## Highlights

- IPv4 destinations supplied as addresses, hostnames, or fully qualified domain names (FQDNs).
- ICMP Echo Request construction and checksum calculation.
- Parsing and validation of Echo Replies, including identifier, sequence number, source address, and checksum.
- Verbose diagnostics for related ICMP errors (Destination Unreachable, Time Exceeded, and Parameter Problem).
- Non-blocking receive loop with `poll()`, configurable send interval, count, and time limits.
- Monotonic RTT measurements, packet loss, duplicate detection, and min/avg/max/stddev reporting.
- Graceful `SIGINT` / `SIGTERM` shutdown and Makefile dependency tracking.
- Eight additional command-line behaviors: `-c`, `-i`, `-s`, `-w`, `-W`, `-n`, `-p`, and `--ttl`.

## Requirements

- **Linux** with IPv4 raw socket support (developed on Debian 12).
- A C compiler, `make`, and the standard C development headers.
- Permission to create a raw ICMP socket: typically `sudo` or `CAP_NET_RAW`.

A virtual machine is required for the **42 project evaluation**; it is not a technical requirement of the program itself. See [VM setup](docs/VM_SETUP.md).

## Build

```sh
git clone https://github.com/lemonnn616/ft_ping.git
cd ft_ping
make
```

Useful targets:

```sh
make        # Build (incremental)
make clean  # Remove object and dependency files
make fclean # Remove build files and executable
make re     # Clean rebuild
```

The project is compiled with `-Wall -Wextra -Werror -g3`. The linker uses `-lm` for RTT statistics.

## Quick start

```sh
sudo ./ft_ping 127.0.0.1
sudo ./ft_ping -c 5 localhost
sudo ./ft_ping -v -c 3 codam.nl
sudo ./ft_ping -c 5 -i 0.2 -s 128 1.1.1.1
```

Without a count or time limit, the program keeps sending requests until interrupted with **Ctrl+C**. A typical run displays each reply and ends with transmission, loss, and RTT statistics.

### Example output

```text
$ sudo ./ft_ping -c 3 127.0.0.1
PING 127.0.0.1 (127.0.0.1): 56 data bytes
64 bytes from 127.0.0.1: icmp_seq=0 ttl=64 time=0.096 ms
64 bytes from 127.0.0.1: icmp_seq=1 ttl=64 time=0.173 ms
64 bytes from 127.0.0.1: icmp_seq=2 ttl=64 time=0.138 ms
--- 127.0.0.1 ping statistics ---
3 packets transmitted, 3 packets received, 0% packet loss
round-trip min/avg/max/stddev = 0.096/0.136/0.173/0.031 ms
```

RTT values and other network-dependent fields will vary.

## Command-line options

```text
Usage: ft_ping [OPTION...] HOST
```

| Option | Purpose | Example |
| --- | --- | --- |
| `-?`, `--help` | Display usage information | `./ft_ping -?` |
| `-v`, `--verbose` | Show additional ICMP diagnostics | `sudo ./ft_ping -v -c 3 127.0.0.1` |
| `-c N`, `--count=N` | Limit the number of send attempts | `sudo ./ft_ping -c 5 127.0.0.1` |
| `-i SEC`, `--interval=SEC` | Set the send interval in seconds | `sudo ./ft_ping -c 5 -i 0.2 127.0.0.1` |
| `-s BYTES`, `--size=BYTES` | Set the ICMP payload size | `sudo ./ft_ping -c 3 -s 128 127.0.0.1` |
| `-w SEC`, `--timeout=SEC` | Set an overall time limit | `sudo ./ft_ping -w 3 127.0.0.1` |
| `-W SEC`, `--linger=SEC` | Set the wait period after a count-limited send phase | `sudo ./ft_ping -c 2 -W 2 192.0.2.1` |
| `-n`, `--numeric` | Print numeric reply addresses | `sudo ./ft_ping -n -c 3 localhost` |
| `-p HEX`, `--pattern=HEX` | Repeat a 1–16-byte hexadecimal pattern in the payload | `sudo ./ft_ping -c 3 -s 24 -p a1b2 127.0.0.1` |
| `--ttl N` | Set outgoing IPv4 TTL (1–255) | `sudo ./ft_ping -c 3 --ttl 1 127.0.0.1` |

`-i` accepts fractional seconds. `-p` accepts 2–32 hexadecimal characters in complete byte pairs. When the payload is at least 8 bytes, the first 8 bytes remain unchanged and the pattern repeats through the remainder.

## Architecture at a glance

```text
CLI options ──► IPv4 resolution ──► raw ICMP socket
                                        │
                          ┌─────────────▼────────────┐
                          │ poll() / monotonic clock │
                          └─────────┬───────┬────────┘
                                    │       │
                            ICMP send    IPv4 receive
                                    │       │
                              checksum    ├── Echo Reply → RTT / stats
                                          └── ICMP error → diagnostics
```

The application uses one event loop rather than a thread per request. The Linux kernel supplies the outer IPv4 header; the program constructs and validates ICMP messages.

For packet layouts, runtime state, scheduling, statistics, and design trade-offs, see [Architecture](docs/architecture.md).

## Testing and evaluation

- [Evaluation commands](docs/evaluation.md): mandatory requirements, each implemented bonus option, and invalid-input cases.
- [VM setup](docs/VM_SETUP.md): reproducible Debian / VirtualBox development environment.
- [Architecture](docs/architecture.md): implementation details and known limitations.

For the reference implementation, build and run **GNU inetutils 2.0** separately and compare output and behavior. The system `ping` on many Linux distributions comes from **iputils**, which is *not* the ft_ping v5.1 reference.

```sh
sudo ./ft_ping -c 5 127.0.0.1
sudo ~/inetutils-2.0/ping/ping -c 5 127.0.0.1
```

RTT values naturally vary across runs. Packet/error behavior can also depend on routing and VM networking: for example, VirtualBox NAT may not produce a Time Exceeded response for `--ttl 1` on an external destination.

## Project layout

```text
ft_ping/
├── Makefile
├── include/
│   └── ft_ping.h
├── src/
│   ├── main.c
│   ├── options/     # CLI parsing and help
│   ├── network/     # IPv4 resolution and raw socket setup
│   ├── icmp/        # Checksums, requests, replies, ICMP errors
│   ├── runtime/     # Clock, signals, event loop
│   └── output/      # Final statistics
└── docs/
    ├── architecture.md
    ├── evaluation.md
    └── VM_SETUP.md
```

## Scope and limitations

- **IPv4 only**; this is not an IPv6 ping implementation.
- **Subset of inetutils 2.0**: unlisted flags and some boundary-case behaviors are not implemented.
- **Raw socket privileges** are required.
- **Reverse DNS** (when enabled) is performed before the receive loop, not while processing packet replies.
- **Sequence numbers are 16-bit**; long-running sessions that wrap the sequence space are not fully disambiguated.
- `-p` reserves the first eight payload bytes for compatibility with the reference's timing area, but RTT timestamps are actually tracked in process memory. Packet contents are therefore not a byte-for-byte copy of inetutils.
- Exact output and unusual network-error scenarios still require independent comparison and testing.

## Educational context

This project was developed as part of the **42 / Codam** curriculum to explore ICMP, IPv4 packet formats, raw sockets, event-driven I/O, and network troubleshooting. It is an independent C implementation; it does **not** invoke the system `ping` command.

**Licensing:** no `LICENSE` file has been selected or added yet. Publicly readable source code is not automatically licensed for reuse or redistribution.
