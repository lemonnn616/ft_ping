#include "ft_ping.h"

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Returns a human-readable description of an ICMP error.
 *
 * @param type Outer ICMP type.
 * @param code Outer ICMP code.
 * @return Static string describing the error.
 */
static const char *icmp_error_description(uint8_t type, uint8_t code)
{
    if (type == ICMP_TIMXCEED)
    {
        if (code == 0)
            return "Time to live exceeded";
        if (code == 1)
            return "Fragment reassembly time exceeded";

        return "Time exceeded";
    }

    if (type == ICMP_PARAMETERPROB)
        return "Parameter problem";

    if (type == ICMP_DEST_UNREACH)
    {
        switch (code)
        {
            case 0: return "Network unreachable";
            case 1: return "Host unreachable";
            case 2: return "Protocol unreachable";
            case 3: return "Port unreachable";
            case 4: return "Fragmentation needed";
            case 5: return "Source route failed";
            default: return "Destination unreachable";
        }
    }

    return "Unknown ICMP error";
}

/**
 * @brief Processes an ICMP error related to our Echo Request.
 *
 * Validates the outer IPv4 and ICMP headers, verifies the
 * outer ICMP checksum, and extracts the quoted original
 * IPv4 and ICMP headers.
 *
 * Matches the quoted identifier and sequence number
 * against previously transmitted requests.
 *
 * Related errors are printed once when verbose is enabled.
 *
 * @param packet Received IPv4 packet bytes.
 * @param count Number of bytes received.
 * @param ctx Context containing the target and request table.
 * @return 1 if the error relates to our request, 0 otherwise.
 */
int process_icmp_error_packet(const uint8_t *packet, ssize_t count,
                              t_ping_context *ctx)
{
    struct iphdr outer_ip;
    struct iphdr quoted_ip;
    struct icmphdr outer_icmp;
    struct icmphdr quoted_icmp;

    size_t outer_ihl;
    size_t outer_len;
    size_t quote_offset;
    size_t quoted_ihl;

    uint16_t sequence;
    t_ping_request *request;
    char router[INET_ADDRSTRLEN];

    if (count < (ssize_t)sizeof(outer_ip))
        return 0;

    memcpy(&outer_ip, packet, sizeof(outer_ip));

    if (outer_ip.version != 4 || outer_ip.ihl < 5 ||
        outer_ip.protocol != IPPROTO_ICMP)
        return 0;

    outer_ihl = (size_t)outer_ip.ihl * 4;
    outer_len = ntohs(outer_ip.tot_len);

    if (outer_len > (size_t)count ||
        outer_len < outer_ihl + sizeof(outer_icmp))
        return 0;

    memcpy(&outer_icmp,
           packet + outer_ihl,
           sizeof(outer_icmp));

    if (outer_icmp.type != ICMP_DEST_UNREACH &&
        outer_icmp.type != ICMP_TIMXCEED &&
        outer_icmp.type != ICMP_PARAMETERPROB)
        return 0;

    if (icmp_checksum(packet + outer_ihl,
                      outer_len - outer_ihl) != 0)
        return 0;

    quote_offset = outer_ihl + sizeof(outer_icmp);

    if (outer_len - quote_offset < sizeof(quoted_ip))
        return 0;

    memcpy(&quoted_ip,
           packet + quote_offset,
           sizeof(quoted_ip));

    if (quoted_ip.version != 4 ||
        quoted_ip.ihl < 5 ||
        quoted_ip.protocol != IPPROTO_ICMP ||
        quoted_ip.daddr != ctx->target.address.sin_addr.s_addr)
        return 0;

    quoted_ihl = (size_t)quoted_ip.ihl * 4;

    if (outer_len - quote_offset <
        quoted_ihl + sizeof(quoted_icmp))
        return 0;

    memcpy(&quoted_icmp,
           packet + quote_offset + quoted_ihl,
           sizeof(quoted_icmp));

    if (quoted_icmp.type != ICMP_ECHO ||
        quoted_icmp.code != 0 ||
        ntohs(quoted_icmp.un.echo.id) != ctx->identifier)
        return 0;

    sequence = ntohs(quoted_icmp.un.echo.sequence);
    request = &ctx->requests[sequence];

    if (!request->sent)
        return 0;

    if (request->error_reported)
        return 1;

    request->error_reported = 1;

    if (!ctx->options.verbose)
        return 1;

    if (inet_ntop(AF_INET, &outer_ip.saddr,
                  router, sizeof(router)) == NULL)
        snprintf(router, sizeof(router), "unknown");

    printf("From %s: icmp_seq=%u %s (type=%u, code=%u)\n",
           router,
           (unsigned)sequence,
           icmp_error_description(outer_icmp.type, outer_icmp.code),
           (unsigned)outer_icmp.type,
           (unsigned)outer_icmp.code);

    printf("  Quoted destination: %s, original TTL: %u\n",
           ctx->target.ip_string,
           (unsigned)quoted_ip.ttl);

    return 1;
}