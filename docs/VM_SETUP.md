# Debian / VirtualBox development environment

These are the **development VM settings** used for the 42 / Codam ft_ping v5.1 project, not universal requirements for running the binary. The program itself requires Linux with IPv4 raw socket support.

## Baseline VM

| Setting | Value |
| --- | --- |
| Hypervisor | Oracle VirtualBox 7.2.22 |
| Guest OS | Debian 12.15 amd64 |
| Hostname | `ftping-vm` |
| Local account | `ftping` |
| Virtual CPUs | 4 |
| RAM | 4096 MB |
| Disk | 40 GB, dynamically allocated VDI |
| Network adapter | NAT |
| EFI | Disabled |
| Desktop environment | None (command-line system) |
| Language / compiler | C / GCC |

When working from two computers, create a separate Debian VM with the same baseline configuration on each machine. **No Vagrant configuration is used.** Git synchronizes the source code; VM settings and installed packages must be matched manually.

## Packages

On a fresh Debian VM:

```sh
sudo apt update
sudo apt install -y build-essential git gdb wget xz-utils tcpdump
```

`gdb` and `tcpdump` are optional debugging tools. `build-essential` supplies the C compiler and `make`.

## Get and build ft_ping

```sh
git clone https://github.com/lemonnn616/ft_ping.git
cd ft_ping
make re
sudo ./ft_ping -c 3 127.0.0.1
```

The program creates `socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)`, so ordinary user accounts may receive a `Permission denied` / `Operation not permitted` socket error. For straightforward VM evaluation, use `sudo`.

## Build the exact GNU inetutils 2.0 reference

The Debian system `ping` may be **iputils**, not GNU inetutils. Build the required reference separately, without replacing the system command:

```sh
cd ~
wget https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.xz
tar -xf inetutils-2.0.tar.xz
cd inetutils-2.0
./configure --disable-servers
make -j"$(nproc)"
./ping/ping -V
```

Expected version:

```text
ping (GNU inetutils) 2.0
```

Compare the two programs from any directory:

```sh
sudo ~/ft_ping/ft_ping -c 5 127.0.0.1
sudo ~/inetutils-2.0/ping/ping -c 5 127.0.0.1
```

The reference is for **running and comparing behavior**. The `ft_ping` implementation must not invoke or embed the system ping.

## Networking and troubleshooting

- NAT provides convenient outbound connectivity, but it can change ICMP behavior. In particular, `--ttl 1` might not trigger Time Exceeded on an external address.
- A reply's printed `ttl=` is the TTL of the **received** IP packet, not the outbound TTL set using `--ttl`.
- Check ICMP traffic with `sudo tcpdump -ni any icmp` when needed.
- Test routing-dependent ICMP errors on a network topology where a router can actually decrement the TTL and send the expected error.
- Keep the two VMs' software versions, VM configuration, and project revision aligned.

## Two-machine Git hygiene

Generate a **separate SSH key per VM** and add each public key to your GitHub account. Do not place private keys, shell credentials, or VM secrets in this repository.

```sh
git pull --ff-only
make re
```

## Evaluation

Use [evaluation.md](evaluation.md) for one-command mandatory, bonus, and invalid-input checks, and [architecture.md](architecture.md) for protocol and implementation details.
