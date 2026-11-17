# ft_ping VM configuration

## Virtual machine
- Hypervisor: Oracle VirtualBox 7.2.22
- OS: Debian 12.15 amd64
- Hostname: ftping-vm
- Username: ftping
- CPU: 4 vCPU
- RAM: 4096 MB
- Storage: 40 GB, VDI, dynamically allocated
- Network: NAT
- EFI: disabled
- Desktop environment: none

## Development
- Language: C
- Compiler: GCC
- Debugger: GDB
- Version control: Git
- Target reference: GNU inetutils 2.0

## Notes
Both virtual machines should use identical
packages, project configuration and settings.

Each VM uses its own SSH authentication key.