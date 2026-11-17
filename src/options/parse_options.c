#include "ft_ping.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#define OPT_TTL 1000

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
 * @brief Parses and validates an IPv4 TTL value.
 *
 * Accepts only decimal integers in the range 1..255.
 *
 * @param value Input TTL string.
 * @param ttl Output parameter receiving the parsed TTL.
 * @return 0 on success, -1 for an invalid value.
 */
static int parse_ttl(const char *value, int *ttl)
{
    char *end;
    long parsed;

    if (value == NULL || *value == '\0')
        return -1;

    errno = 0;
    parsed = strtol(value, &end, 10);

    if (errno != 0 || *end != '\0' ||
        parsed < 1 || parsed > 255)
        return -1;

    *ttl = (int)parsed;

    return 0;
}

/**
 * @brief Parses command-line options and validates the target.
 *
 * Supports -v, --verbose, -?, --help, and --ttl.
 * Stores the target hostname and parsed settings in options.
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
        {"ttl", required_argument, NULL, OPT_TTL},
        {NULL, 0, NULL, 0}
    };

    int opt;

    options->payload_size = PING_DEFAULT_PAYLOAD_SIZE;
    options->ttl = 0;
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
        else if (opt == OPT_TTL)
        {
            if (parse_ttl(optarg, &options->ttl) != 0)
            {
                fprintf(stderr,
                        "ft_ping: invalid TTL (expected 1..255)\n");
                return PARSE_ERROR;
            }
        }
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

