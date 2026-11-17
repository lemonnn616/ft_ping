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
    printf("  -c, --count=N       Stop after N requests\n");
    printf("  -i, --interval=SEC  Time between requests\n");
    printf("  -s, --size=BYTES    ICMP payload size\n");
    printf("  -n, --numeric       Numeric-only reply addresses\n");
    printf("  -p, --pattern=HEX   Repeat 1..16 hex bytes in payload\n");
    printf("  -w, --timeout=N     Stop sending after N seconds\n");
    printf("  -W, --linger=N      Wait up to N seconds for replies\n");
    printf("      --ttl=N      Set IPv4 time-to-live\n");
}