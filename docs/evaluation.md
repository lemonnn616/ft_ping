# ft_ping Evaluation Commands

## Mandatory

Build: `make re`

Executable: `test -x ./ft_ping`

Reference: `~/inetutils-2.0/ping/ping -V`

-?: `./ft_ping -?`

-v: `sudo ./ft_ping -v -c 3 127.0.0.1`

IPv4: `sudo ./ft_ping -c 3 127.0.0.1`

Hostname: `sudo ./ft_ping -c 3 localhost`

FQDN: `sudo ./ft_ping -c 3 codam.nl`

Continuous / SIGINT: `sudo timeout --signal=INT 4s ./ft_ping 127.0.0.1`

Statistics: `sudo ./ft_ping -c 5 127.0.0.1`

Packet loss: `sudo ./ft_ping -c 2 -W 2 192.0.2.1`

ICMP Time Exceeded (routed network only): `sudo ./ft_ping -v -c 3 --ttl 1 1.1.1.1`

ICMP Destination Unreachable (network-dependent): `sudo ./ft_ping -v -c 3 -W 2 192.0.2.1`

## Bonus

-c: `sudo ./ft_ping -c 5 127.0.0.1`

-i: `sudo ./ft_ping -c 5 -i 0.2 127.0.0.1`

-s: `sudo ./ft_ping -c 3 -s 128 127.0.0.1`

-w: `sudo ./ft_ping -w 3 127.0.0.1`

-W: `sudo ./ft_ping -c 2 -W 2 192.0.2.1`

-n: `sudo ./ft_ping -n -c 3 localhost`

-p: `sudo ./ft_ping -c 3 -s 24 -p a1b2 127.0.0.1`

--ttl: `sudo ./ft_ping -c 3 --ttl 1 127.0.0.1`

## Errors

Missing host: `./ft_ping`

Too many hosts: `./ft_ping 127.0.0.1 1.1.1.1`

Unknown option: `./ft_ping --unknown 127.0.0.1`

Invalid hostname: `./ft_ping -c 1 nonexistent.invalid`

Unsupported IPv6: `./ft_ping -c 1 ::1`

Invalid --ttl: `./ft_ping --ttl 256 127.0.0.1`

Invalid -c: `./ft_ping -c 0 127.0.0.1`

Invalid -i: `./ft_ping -i abc 127.0.0.1`

Invalid -s: `./ft_ping -s 65508 127.0.0.1`

Invalid -w: `./ft_ping -w 0 127.0.0.1`

Invalid -W: `./ft_ping -W abc 127.0.0.1`

Invalid -p hex: `./ft_ping -p zz 127.0.0.1`

Invalid -p length: `./ft_ping -p abc 127.0.0.1`

Invalid -p overflow: `./ft_ping -p 000102030405060708090a0b0c0d0e0f10 127.0.0.1`
