#include "ft_ping.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Resolves a hostname or IPv4 string to an IPv4 address.
 *
 * Uses getaddrinfo() with IPv4 and ICMP constraints.
 * Copies the resolved address into the target structure
 * and converts it into a human-readable IPv4 string.
 *
 * @param hostname Hostname or IPv4 address to resolve.
 * @param target Output structure for the resolved address.
 *
 * @return 0 on success, -1 on failure.
 */
int resolve_target(const char *hostname, t_ping_target *target)
{
    struct addrinfo hints = {0};
    struct addrinfo *results = NULL;
    int err;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    err = getaddrinfo(hostname, NULL, &hints, &results);

    if (err != 0)
    {
        if (err == EAI_SYSTEM)
            perror("ft_ping: getaddrinfo");
        else
            fprintf(stderr, "ft_ping: %s: %s\n",
                    hostname, gai_strerror(err));
        return -1;
    }

    memcpy(
        &target->address,
        results->ai_addr,
        sizeof(target->address)
    );

    if (inet_ntop(
            AF_INET,
            &target->address.sin_addr,
            target->ip_string,
            sizeof(target->ip_string)
        ) == NULL)
    {
        perror("ft_ping: inet_ntop");
        freeaddrinfo(results);
        return -1;
    }

    freeaddrinfo(results);

    return 0;
}
/**
 * @brief Caches the source label before the ping receive loop.
 *
 * -n uses numeric IPv4. Otherwise, a reverse DNS lookup is
 * attempted once for hostname targets; if it fails, use the
 * originally requested hostname. No DNS runs on packet receipt.
 *
 * @param options Parsed command-line options.
 * @param target Resolved IPv4 destination and display text.
 */
void prepare_reply_display(const t_ping_options *options,
                           t_ping_target *target)
{
    struct in_addr numeric_address;
    char hostname[256];
    int result;

    snprintf(target->reply_display, sizeof(target->reply_display),
             "%s", target->ip_string);

    if (options->numeric ||
        inet_pton(AF_INET, options->target, &numeric_address) == 1)
        return;

    result = getnameinfo((const struct sockaddr *)&target->address,
                         sizeof(target->address),
                         hostname, sizeof(hostname),
                         NULL, 0, NI_NAMEREQD);

    if (result == 0)
        snprintf(target->reply_display, sizeof(target->reply_display),
                 "%s (%s)", hostname, target->ip_string);
    else
        snprintf(target->reply_display, sizeof(target->reply_display),
                 "%s (%s)", options->target, target->ip_string);
}
