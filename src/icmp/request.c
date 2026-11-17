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
 * @brief Sends one ICMP Echo Request to the IPv4 destination.
 *
 * Builds an ICMP message and transmits it using the raw socket.
 * Linux generates the IPv4 header automatically.
 *
 * @param ctx Application context containing the socket,
 * destination, identifier, sequence, and payload size.
 * @return 0 if all ICMP bytes were sent, -1 on failure.
 */
int send_icmp_request(t_ping_context *ctx)
{
    uint8_t *packet;
    size_t packet_size;
    ssize_t sent;

    packet = build_echo_request(ctx, &packet_size);
    if (packet == NULL)
        return -1;

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

    return 0;
}