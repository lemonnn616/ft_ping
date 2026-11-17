#include "ft_ping.h"

#include <stdio.h>

/**
 * @brief Prints usage information and supported options.
 *
 * Displays the command syntax and currently implemented
 * command-line flags.
 *
 * @return Nothing (void).
 */
void print_help(void)
{
    printf("Usage: ft_ping [OPTION...] HOST\n\n");

    printf("Options:\n");
    printf("  -v, --verbose    Verbose output\n");
    printf("  -?, --help       Display this help message\n");
    printf("      --ttl=N      Set IPv4 time-to-live\n");
}