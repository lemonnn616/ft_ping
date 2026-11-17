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