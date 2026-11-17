#include "ft_ping.h"

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Entry point of the ft_ping application.
 *
 * Parses command-line arguments, resolves the IPv4 target,
 * creates an ICMP raw socket, and releases its resources.
 *
 * Packet transmission will be implemented in the next step.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 0 if help was requested.
 * @return 1 if initialization fails or ping is not implemented.
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

    printf("Target: %s\n", ctx.options.target);
    printf("IPv4: %s\n", ctx.target.ip_string);
    printf("Verbose: %s\n",
           ctx.options.verbose ? "yes" : "no");

    printf("ICMP raw socket created (fd=%d)\n", ctx.socket_fd);

    if (close(ctx.socket_fd) == -1)
    {
        perror("ft_ping: close");
        return 1;
    }

    ctx.socket_fd = -1;

    printf("Socket closed successfully.\n");
    printf("ft_ping: packet transmission not implemented yet\n");

    return 1;
}