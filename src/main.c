#include "ft_ping.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>

/**
 * @brief Entry point of the ft_ping application.
 *
 * Initializes options, resolves the destination, opens
 * the ICMP socket, allocates request tracking memory,
 * installs signal handlers, and runs the ping loop.
 *
 * Releases all resources before exiting.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 0 after normal termination or help output.
 * @return 1 on initialization or runtime error.
 * @return 2 for invalid command-line arguments.
 */
int main(int argc, char **argv)
{
    t_ping_context ctx = {0};
    t_parse_result result;
    int status;

    ctx.socket_fd = -1;

    result = parse_options(argc, argv, &ctx.options);

    if (result == PARSE_HELP)
    {
        print_help();
        return 0;
    }

    if (result == PARSE_ERROR)
        return 2;

    if (resolve_target(ctx.options.target, &ctx.target) != 0)
        return 1;

    prepare_reply_display(&ctx.options, &ctx.target);

    ctx.socket_fd = create_icmp_socket();
    if (ctx.socket_fd == -1)
        return 1;

    if (configure_icmp_ttl(ctx.socket_fd,
                        ctx.options.ttl) != 0)
    {
        close(ctx.socket_fd);
        return 1;
    }

    ctx.requests = calloc(PING_SEQUENCE_COUNT,
                          sizeof(*ctx.requests));

    if (ctx.requests == NULL)
    {
        perror("ft_ping: calloc");
        close(ctx.socket_fd);
        return 1;
    }

    ctx.identifier = (uint16_t)getpid();
    ctx.sequence = 0;

    if (install_signal_handlers() != 0)
    {
        free(ctx.requests);
        close(ctx.socket_fd);
        return 1;
    }

    printf("PING %s (%s): %zu data bytes",
           ctx.options.target,
           ctx.target.ip_string,
           ctx.options.payload_size);

    if (ctx.options.verbose)
        printf(", id 0x%04x = %u",
               (unsigned)ctx.identifier,
               (unsigned)ctx.identifier);

    putchar('\n');

    status = run_ping_loop(&ctx);

    print_statistics(&ctx);

    if (close(ctx.socket_fd) == -1)
    {
        perror("ft_ping: close");
        status = -1;
    }

    free(ctx.requests);

    if (status != 0)
        return 1;

    return ctx.stats.received > 0 ? 0 : 1;
}