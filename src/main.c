#include "ft_ping.h"

#include <stdio.h>

/**
 * @brief Entry point of the ft_ping application.
 *
 * Parses command-line arguments and resolves the destination
 * to an IPv4 address. Packet transmission will be implemented
 * in a later development stage.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 0 if help was requested.
 * @return 1 if resolution fails or ping is not implemented.
 * @return 2 if command-line arguments are invalid.
 */
int main(int argc, char **argv)
{
    t_ping_options options = {0};
    t_ping_target target = {0};
    t_parse_result result;

    result = parse_options(argc, argv, &options);

    if (result == PARSE_HELP)
    {
        print_help();
        return 0;
    }

    if (result == PARSE_ERROR)
        return 2;

    if (resolve_target(options.target, &target) != 0)
        return 1;

    printf("Target: %s\n", options.target);
    printf("IPv4: %s\n", target.ip_string);
    printf("Verbose: %s\n", options.verbose ? "yes" : "no");

    fprintf(stderr, "ft_ping: ICMP not implemented yet\n");

    return 1;
}