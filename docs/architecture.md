# Architecture

This document describes the implementation currently present in the repository. The behavioral reference for the 42 / Codam **ft_ping v5.1** assignment is **GNU inetutils 2.0**.

## System overview

```text
User CLI
   │
   ▼
parse_options() ──► t_ping_options
   │
   ▼
resolve_target() ──► IPv4 sockaddr_in + printable address
   │
   ▼
prepare_reply_display() ──► cached label (-n skips reverse DNS)
   │
   ▼
create_icmp_socket() / configure_icmp_ttl()
   │
   ▼
run_ping_loop()
   ├── monotonic time / send schedule / timeout
   ├── send_icmp_request() ──► build Echo Request ──► checksum ──► sendto()
   ├── poll() ──► receive_icmp_reply() ──► recvfrom(MSG_DONTWAIT)
   │                                       ├── Echo Reply → validate → RTT / counters
   │                                       └── ICMP Error → quoted request validation
   └── stop on SIGINT, SIGTERM, count, or timeout
   │
   ▼
print_statistics() ──► close() / free()
```

The application uses **one event loop**, not separate worker threads. The kernel builds outgoing IPv4 headers when the application writes an ICMP message to a `SOCK_RAW` / `IPPROTO_ICMP` socket.

## Source map

| Path | Responsibility |
| --- | --- |
| `src/main.c` | Initialization, socket lifecycle, request table, shutdown |
| `src/options/parse_options.c` | `getopt_long()`, numeric validation, option storage |
| `src/options/print_help.c` | Supported CLI usage |
| `src/network/resolve.c` | `getaddrinfo(AF_INET)`, IP conversion, cached display label |
| `src/network/socket.c` | `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`, `setsockopt(IP_TTL)` |
| `src/icmp/request.c` | Echo Request construction, payload pattern, `sendto()` |
| `src/icmp/checksum.c` | ICMP Internet checksum |
| `src/icmp/reply.c` | IPv4 / Echo Reply validation, RTT, duplicates |
| `src/icmp/error.c` | Destination Unreachable, Time Exceeded, Parameter Problem |
| `src/runtime/timing.c` | Monotonic timestamps and RTT differences |
| `src/runtime/signals.c` | `sigaction()`, signal-safe stop flag |
| `src/runtime/ping_loop.c` | `poll()`, sending cadence, count/timeout/linger |
| `src/output/statistics.c` | Loss counters and online RTT statistics |
| `include/ft_ping.h` | Shared configuration, state, and function declarations |

## IPv4 and ICMP packets

A normal outgoing request is composed of an **8-byte ICMP header** followed by a configurable payload:

```text
+----------------------+------------------------+
| ICMP Echo Request    | Payload (-s BYTES)     |
| type=8, code=0       | Default: 56 bytes      |
| checksum             |                        |
| identifier, sequence |                        |
+----------------------+------------------------+
```

The program sets the identifier to the process ID truncated to 16 bits, and starts the sequence number at **0**. It generates the ICMP checksum after filling the packet. Linux supplies the outer IPv4 header (and honors the `IP_TTL` socket option).

For `-p`, the parser decodes hexadecimal pairs into 1–16 binary bytes and `request.c` repeats them in the payload. When the payload is at least eight bytes, its first eight bytes retain their default values and only the remainder is patterned. These bytes are **not** the application's RTT timing source.

## Receiving and validating responses

`recvfrom(MSG_DONTWAIT)` provides a packet that includes the outer IPv4 header. The reply handler checks the IPv4 version, header length and protocol, expected source address, ICMP type/code, echo identifier, and ICMP checksum before using the sequence number.

The request table contains **65,536 entries**, indexed by the 16-bit sequence number. Each entry records a monotonic send timestamp and flags for whether the request was sent, already answered, or already associated with an ICMP error.

For a matching Echo Reply:

1. Read a monotonic receipt timestamp.
2. Find the corresponding request entry by sequence.
3. Compute `RTT = received_at - sent_at`.
4. Update unique received count or duplicate count.
5. Print reply size, numeric or cached hostname, sequence, TTL, and RTT.

Reverse DNS does **not** run inside the receive handler. `-n` forces numeric output; without it, a hostname display string may be prepared once during initialization.

### ICMP errors

An ICMP error carries a quotation of the original triggering packet:

```text
Outer IPv4 header (typically: router address)
  └── Outer ICMP error (type 3, 11, or 12)
        └── Quoted original IPv4 header
              └── Quoted original ICMP Echo Request header
                    └── original identifier + sequence
```

The error handler validates the message length and checksum, then matches the quoted destination, ICMP Echo Request type, identifier, and sequence to an outstanding request. Related errors are shown with `-v` and do **not** count as successful Echo Replies. The same error is suppressed after it has been reported once for that sequence.

The originating router's source address may differ from the target's address; that is expected for messages such as Time Exceeded.

## Scheduling and time limits

`run_ping_loop()` uses `CLOCK_MONOTONIC` timestamps and `poll()`, so incoming packets can be processed without sleeping through the next scheduled transmission.

- **Default interval:** 1000 ms; `-i` changes the cadence.
- **`-c N`:** stop scheduling new sends after N attempts.
- **`-w N`:** absolute execution deadline measured from loop startup.
- **`-W N`:** after a count-limited run, use a final receive-only window. Its deadline is based on the next scheduled send slot.
- **Signals:** `SIGINT` / `SIGTERM` handlers set `volatile sig_atomic_t`; cleanup runs in the main control flow.

The scheduler clears recoverable socket errors with `getsockopt(SO_ERROR)`. Irrecoverable system-call failures are reported and can terminate the program.

## Statistics

The program tracks:

- Successfully submitted Echo Requests (transmitted).
- Unique matching Echo Replies (received).
- Additional matching replies for already answered requests (duplicates).
- Packet loss from transmitted and unique received counts.
- RTT minimum, maximum, mean, and population standard deviation.

RTT samples are aggregated with **Welford's online algorithm**, using `mean_rtt` and `m2_rtt`; all samples do not have to be retained in memory:

```text
n += 1
delta = rtt - mean
mean += delta / n
m2 += delta * (rtt - mean)
stddev = sqrt(m2 / n)
```

The calculation uses a monotonic clock rather than the wall clock, so adjusting the system date does not invalidate RTT measurement.

## Boundaries and known limitations

- Only IPv4 is supported; ICMPv6 is out of scope.
- Reverse DNS is cached in advance; its output need not match the reference's resolver behavior.
- Sequence numbers wrap at 65,536. The request table is reused, and extremely long sessions are not fully protected against delayed responses from a previous sequence cycle.
- The outgoing payload does not contain the same embedded timing representation as GNU inetutils 2.0.
- Only a selected set of ICMP errors and inetutils command-line options are implemented.
- `-w` and `-W` behavior was adjusted against observed inetutils 2.0 runs, but edge conditions still need independent testing.
- Delivering ICMP Time Exceeded depends on the surrounding network; some VirtualBox NAT configurations may answer even with a low outgoing TTL.

For a concise, command-only checklist, see [Evaluation commands](evaluation.md).
