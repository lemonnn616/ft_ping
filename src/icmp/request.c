#include "ft_ping.h"

#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

/**
 * @brief Builds a complete ICMP Echo Request message.
 *
 * Allocates memory for an ICMP header and payload,
 * fills the identifier and sequence number,
 * and calculates the checksum.
 *
 * The caller must free the returned buffer.
 *
 * @param ctx Current ping state and configuration.
 * @param packet_size Output parameter receiving packet size.
 * @return Allocated packet buffer, or NULL on failure.
 */
static uint8_t *build_echo_request(const t_ping_context *ctx,
                                    size_t *packet_size)
{
    uint8_t *packet;
    struct icmphdr *header;
    size_t i;

    if (ctx->options.payload_size > PING_MAX_PAYLOAD_SIZE)
    {
        fprintf(stderr, "ft_ping: payload size is too large\n");
        return NULL;
    }

    *packet_size = sizeof(struct icmphdr) + ctx->options.payload_size;

    packet = calloc(1, *packet_size);
    if (packet == NULL)
    {
        perror("ft_ping: calloc");
        return NULL;
    }

    header = (struct icmphdr *)packet;

    header->type = ICMP_ECHO;
    header->code = 0;
    header->un.echo.id = htons(ctx->identifier);
    header->un.echo.sequence = htons(ctx->sequence);

    i = sizeof(*header);
    while (i < *packet_size)
    {
        packet[i] = (uint8_t)(i - sizeof(*header));
        i++;
    }

    header->checksum = icmp_checksum(packet, *packet_size);

    return packet;
}

/**
 * @brief Sends an ICMP Echo Request and records its timestamp.
 *
 * Builds the packet, records the send time, and transmits
 * it through the raw socket.
 *
 * After successful transmission, updates the request
 * tracking table and advances the sequence number.
 *
 * @param ctx Application context containing socket,
 * destination, and request tracking information.
 * @return 0 on successful transmission, -1 on error.
 */
int send_icmp_request(t_ping_context *ctx)
{
    uint8_t *packet;
    size_t packet_size;
    struct timespec sent_at;
    ssize_t sent;
    t_ping_request *request;

    packet = build_echo_request(ctx, &packet_size);
    if (packet == NULL)
        return -1;

    if (get_monotonic_time(&sent_at) != 0)
    {
        free(packet);
        return -1;
    }

    sent = sendto(ctx->socket_fd, packet, packet_size, 0,
                  (const struct sockaddr *)&ctx->target.address,
                  sizeof(ctx->target.address));

    free(packet);

    if (sent < 0)
    {
        perror("ft_ping: sendto");
        return -1;
    }

    if ((size_t)sent != packet_size)
    {
        fprintf(stderr, "ft_ping: incomplete ICMP send\n");
        return -1;
    }

    request = &ctx->requests[ctx->sequence];

    request->sent_at = sent_at;
    request->sent = 1;
    request->received = 0;

    ctx->stats.transmitted++;
    ctx->sequence++;

    return 0;
}