#include "ft_ping.h"

#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>


/**
 * @brief Checks whether a received packet is our Echo Reply.
 *
 * Validates the IPv4 header, packet lengths, source address,
 * ICMP type, code, identifier, sequence, and checksum.
 *
 * @param packet Received IPv4 packet bytes.
 * @param length Number of received bytes.
 * @param ctx Context containing the expected reply information.
 * @param icmp_length Output length of the ICMP message.
 * @param ttl Output TTL from the received IPv4 header.
 *
 * @return 1 if the reply is valid and matches our request.
 * @return 0 otherwise.
 */
static int is_expected_reply(const uint8_t *packet, ssize_t length,
                             const t_ping_context *ctx,
                             size_t *icmp_length, uint8_t *ttl)
{
    struct iphdr ip;
    struct icmphdr icmp;
    size_t ip_header_size;
    size_t ip_total_size;

    if (length < (ssize_t)sizeof(ip))
        return 0;

    memcpy(&ip, packet, sizeof(ip));

    if (ip.version != 4 || ip.ihl < 5 ||
        ip.protocol != IPPROTO_ICMP)
        return 0;

    if (ip.saddr != ctx->target.address.sin_addr.s_addr)
        return 0;

    ip_header_size = (size_t)ip.ihl * 4;
    ip_total_size = ntohs(ip.tot_len);

    if (ip_total_size > (size_t)length ||
        ip_total_size < ip_header_size + sizeof(icmp))
        return 0;

    memcpy(&icmp, packet + ip_header_size, sizeof(icmp));

    if (icmp.type != ICMP_ECHOREPLY || icmp.code != 0)
        return 0;

    if (ntohs(icmp.un.echo.id) != ctx->identifier ||
        ntohs(icmp.un.echo.sequence) != ctx->sequence)
        return 0;

    if (icmp_checksum(packet + ip_header_size,
                      ip_total_size - ip_header_size) != 0)
        return 0;

    *icmp_length = ip_total_size - ip_header_size;
    *ttl = ip.ttl;

    return 1;
}

/**
 * @brief Waits for one valid ICMP Echo Reply with a timeout.
 *
 * Polls the raw ICMP socket and receives incoming packets.
 * Unrelated packets are ignored without resetting the deadline.
 *
 * @param ctx Application context containing the socket,
 * target address, identifier, and sequence number.
 * @param timeout_ms Maximum waiting time in milliseconds.
 *
 * @return 0 when a matching reply is received.
 * @return 1 when the timeout expires.
 * @return -1 on a system call error.
 */
int receive_icmp_reply(const t_ping_context *ctx, int timeout_ms)
{
    uint8_t packet[65535];
    struct pollfd pfd = {
        .fd = ctx->socket_fd,
        .events = POLLIN
    };

    int64_t deadline;
    int64_t now;
    int ready;
    ssize_t count;
    size_t icmp_length;
    uint8_t ttl;
    struct timespec received_at;
    double rtt_ms;

    now = monotonic_milliseconds();
    if (now < 0)
        return -1;

    deadline = now + timeout_ms;

    while (1)
    {
        now = monotonic_milliseconds();
        if (now < 0)
            return -1;

        if (now >= deadline)
            return 1;

        ready = poll(&pfd, 1, (int)(deadline - now));

        if (ready == 0)
            return 1;

        if (ready == -1)
        {
            if (errno == EINTR)
                continue;

            perror("ft_ping: poll");
            return -1;
        }

        if (!(pfd.revents & POLLIN))
        {
            fprintf(stderr, "ft_ping: socket poll error\n");
            return -1;
        }

        count = recvfrom(ctx->socket_fd, packet,
                         sizeof(packet), 0, NULL, NULL);

        if (count == -1)
        {
            if (errno == EINTR)
                continue;

            perror("ft_ping: recvfrom");
            return -1;
        }

        if (get_monotonic_time(&received_at) != 0)
            return -1;

        if (!is_expected_reply(packet, count, ctx,
                            &icmp_length, &ttl))
            continue;

        rtt_ms = elapsed_milliseconds(&ctx->sent_at, &received_at);

        printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
            icmp_length,
            ctx->target.ip_string,
            (unsigned)ctx->sequence,
            (unsigned)ttl,
            rtt_ms);

        return 0;
    }
}