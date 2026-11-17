#include "ft_ping.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

/**
 * @brief Creates an IPv4 raw socket for ICMP communication.
 *
 * Requests a raw IPv4 socket from the Linux kernel.
 * The socket is configured to use the ICMP protocol.
 *
 * The caller is responsible for closing the descriptor.
 *
 * @return Socket descriptor on success.
 * @return -1 if socket creation fails.
 */
int create_icmp_socket(void)
{
    int fd;

    fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (fd == -1)
    {
        perror("ft_ping: socket");
        return -1;
    }

    return fd;
}

/**
 * @brief Configures the TTL of outgoing IPv4 packets.
 *
 * Uses IP_TTL to configure the socket.
 * A value of zero keeps the kernel's default TTL.
 *
 * @param fd Raw ICMP socket descriptor.
 * @param ttl TTL from 1 to 255, or zero for default.
 * @return 0 on success, -1 on failure.
 */
int configure_icmp_ttl(int fd, int ttl)
{
    if (ttl == 0)
        return 0;

    if (setsockopt(fd, IPPROTO_IP, IP_TTL,
                   &ttl, sizeof(ttl)) == -1)
    {
        perror("ft_ping: setsockopt(IP_TTL)");
        return -1;
    }

    return 0;
}