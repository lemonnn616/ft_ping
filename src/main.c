#include "ft_ping.h"

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Entry point of the ft_ping application.
 *
 * Parses arguments, resolves the target, creates a raw socket,
 * and sends one ICMP Echo Request.
 *
 * Receiving ICMP replies will be implemented in the next step.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 0 if help was requested.
 * @return 1 on initialization or transmission failure,
 * or when reaching the unimplemented reply-handling stage.
 * @return 2 if command-line arguments are invalid.
 */
int main(int argc, char **argv)
{
    t_ping_context ctx = {0};
    t_parse_result result;

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

    ctx.socket_fd = create_icmp_socket();
    if (ctx.socket_fd == -1)
        return 1;

    ctx.identifier = (uint16_t)getpid();
    ctx.sequence = 1;

    if (send_icmp_request(&ctx) != 0)
    {
        close(ctx.socket_fd);
        return 1;
    }

    printf("Sent ICMP Echo Request to %s\n",
           ctx.target.ip_string);

    printf("Identifier: %u, Sequence: %u\n",
           ctx.identifier, ctx.sequence);

    if (close(ctx.socket_fd) == -1)
    {
        perror("ft_ping: close");
        return 1;
    }

    fprintf(stderr, "ft_ping: reply handling not implemented yet\n");

    return 1;
}