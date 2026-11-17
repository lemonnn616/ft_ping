#include "ft_ping.h"

#include <stdio.h>

/**
 * @brief Entry point of the ft_ping application.
 *
 * Initializes application options, parses command-line
 * arguments, and handles help or parsing errors.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 0 when help is displayed successfully.
 * @return 1 when the network layer is not yet implemented.
 * @return 2 when command-line arguments are invalid.
 */
int main(int argc, char **argv)
{
    t_ping_options options = {0};
    t_parse_result result;

    result = parse_options(argc, argv, &options);

    if (result == PARSE_HELP)
    {
        print_help();
        return 0;
    }

    if (result == PARSE_ERROR)
        return 2;

    printf("Target: %s\n", options.target);
    printf("Verbose: %s\n", options.verbose ? "yes" : "no");

    fprintf(stderr, "ft_ping: network layer not implemented yet\n");

    return 1;
}