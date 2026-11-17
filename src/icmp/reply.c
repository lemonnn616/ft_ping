#include "ft_ping.h"

#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int is_duplicate;

/**
 * @brief Stores metadata extracted from an ICMP Echo Reply.
 *
 * Contains the ICMP message size, sequence number,
 * and IPv4 TTL needed for matching and output.
 */
typedef struct s_reply_info
{
    size_t icmp_length;
    uint16_t sequence;
    uint8_t ttl;
} t_reply_info;

/**
 * @brief Parses and validates an incoming ICMP Echo Reply.
 *
 * Checks IPv4 headers, source address, ICMP type,
 * identifier, and checksum.
 *
 * @param packet Received IPv4 packet bytes.
 * @param count Number of received bytes.
 * @param ctx Context containing expected target information.
 * @param reply Output structure receiving validated metadata.
 * @return 1 for a valid matching reply, 0 otherwise.
 */
static int parse_echo_reply(const uint8_t *packet, ssize_t count,
                            const t_ping_context *ctx, t_reply_info *reply)
{
    struct iphdr ip;
    struct icmphdr icmp;
    size_t ip_header_size;
    size_t ip_total_size;

    if (count < (ssize_t)sizeof(ip))
        return 0;

    memcpy(&ip, packet, sizeof(ip));

    if (ip.version != 4 || ip.ihl < 5 ||
        ip.protocol != IPPROTO_ICMP)
        return 0;

    if (ip.saddr != ctx->target.address.sin_addr.s_addr)
        return 0;

    ip_header_size = (size_t)ip.ihl * 4;
    ip_total_size = ntohs(ip.tot_len);

    if (ip_total_size > (size_t)count ||
        ip_total_size < ip_header_size + sizeof(icmp))
        return 0;

    memcpy(&icmp, packet + ip_header_size, sizeof(icmp));

    if (icmp.type != ICMP_ECHOREPLY || icmp.code != 0 ||
        ntohs(icmp.un.echo.id) != ctx->identifier)
        return 0;

    if (icmp_checksum(packet + ip_header_size,
                      ip_total_size - ip_header_size) != 0)
        return 0;

    reply->icmp_length = ip_total_size - ip_header_size;
    reply->sequence = ntohs(icmp.un.echo.sequence);
    reply->ttl = ip.ttl;

    return 1;
}

/**
 * @brief Receives and processes one available ICMP packet.
 *
 * Reads from the socket without blocking, validates the
 * response, and matches it to a previously sent request.
 *
 * Duplicate and unrelated packets are ignored.
 *
 * @param ctx Application state with request timestamps.
 * @return 0 for processed or ignored packets, -1 on error.
 */
int receive_icmp_reply(t_ping_context *ctx)
{
    uint8_t packet[65535];
    ssize_t count;
    struct timespec received_at;
    t_reply_info reply;
    t_ping_request *request;
    double rtt_ms;

    count = recvfrom(ctx->socket_fd, packet, sizeof(packet),
                     MSG_DONTWAIT, NULL, NULL);

    if (count < 0)
    {
        if (errno == EINTR || errno == EAGAIN ||
            errno == EWOULDBLOCK)
            return 0;

        perror("ft_ping: recvfrom");
        return -1;
    }

    if (get_monotonic_time(&received_at) != 0)
        return -1;

    if (!parse_echo_reply(packet, count, ctx, &reply))
        return 0;

    request = &ctx->requests[reply.sequence];

    if (!request->sent)
        return 0;

    rtt_ms = elapsed_milliseconds(&request->sent_at, &received_at);

    is_duplicate = request->received;

    if (is_duplicate)
    {
        ctx->stats.duplicates++;
    }
    else
    {
        request->received = 1;
        ctx->stats.received++;
    }

    update_rtt_stats(&ctx->stats, rtt_ms);

    printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms%s\n",
        reply.icmp_length,
        ctx->target.ip_string,
        (unsigned)reply.sequence,
        (unsigned)reply.ttl,
        rtt_ms,
        is_duplicate ? " (DUP!)" : "");

    return 0;
}