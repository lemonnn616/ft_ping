# ft_ping Architecture

## Overview

ft_ping is a C implementation of the GNU inetutils 2.0
ping utility.

The project implements ICMP Echo Request and Echo Reply
over IPv4 using Linux raw sockets.

## Components

### Options
Command-line parsing and validation.

### Network
DNS resolution, IPv4 addresses and socket management.

### ICMP
Packet creation, checksum, reply parsing and ICMP errors.

### Runtime
Event loop, packet scheduling, timeouts and signals.

### Output
Packet information, errors and statistics.

## Execution flow

1. Parse command-line arguments.
2. Resolve the target hostname to IPv4.
3. Create and configure an ICMP raw socket.
4. Initialize runtime state and statistics.
5. Start the ping event loop.
6. Send ICMP Echo Requests.
7. Receive and validate ICMP responses.
8. Calculate RTT and update statistics.
9. Handle termination conditions.
10. Print final statistics.
11. Release resources.

## Design decisions

- Language: C.
- Protocol: ICMP over IPv4.
- Reference: GNU inetutils 2.0.
- Socket type: SOCK_RAW.
- Timing: CLOCK_MONOTONIC.
- Event handling: poll().
- No use of the system ping implementation.
- Mandatory and bonus features share the same core logic.