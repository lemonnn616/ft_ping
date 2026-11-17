#include "ft_ping.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Checks whether the current short option requests help.
 *
 * Distinguishes a valid -? option from unknown options,
 * since getopt_long() uses '?' for both cases.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return 1 if the current option requests help, otherwise 0.
 */
static int is_short_help(int argc, char **argv)
{
    const char *arg = argv[optind - 1];

    if (arg[0] == '-' && arg[1] == '-')
        return 0;

    if (arg[0] != '-' && optind < argc)
        arg = argv[optind];

    return (arg != NULL &&
            arg[0] == '-' &&
            arg[1] != '-' &&
            strchr(arg + 1, '?') != NULL);
}

/**
 * @brief Parses command-line options and validates the target.
 *
 * Supports -v, --verbose, -?, and --help.
 * Stores the target hostname and verbose flag in options.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param options Pointer to the configuration structure.
 *
 * @return PARSE_OK if parsing succeeds.
 * @return PARSE_HELP if help is requested.
 * @return PARSE_ERROR if an invalid argument is found.
 */
t_parse_result parse_options(
    int argc,
    char **argv,
    t_ping_options *options
)
{
    static const struct option long_options[] = {
        {"verbose", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    int opt;

    options->payload_size = PING_DEFAULT_PAYLOAD_SIZE;
    opterr = 0;

    while ((opt = getopt_long(argc, argv, "v?",
                             long_options, NULL)) != -1)
    {
        if (opt == 'v')
            options->verbose = 1;
        else if (opt == 'h')
            return PARSE_HELP;
        else if (opt == '?' && optopt == 0 &&
                 is_short_help(argc, argv))
            return PARSE_HELP;
        else
        {
            fprintf(stderr, "ft_ping: invalid option\n");
            return PARSE_ERROR;
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, "ft_ping: missing host\n");
        return PARSE_ERROR;
    }

    if (optind + 1 != argc)
    {
        fprintf(stderr, "ft_ping: too many hosts\n");
        return PARSE_ERROR;
    }

    options->target = argv[optind];

    return PARSE_OK;
}