#include "ft_ping.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>


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
 * @brief Parses a non-negative unsigned integer.
 *
 * Rejects signs, invalid characters, empty values,
 * and values outside the unsigned 64-bit range.
 *
 * @param value Input string.
 * @param result Parsed number.
 * @return 0 on success, -1 on error.
 */
static int parse_uint64(const char *value, uint64_t *result)
{
    char *end;
    unsigned long long parsed;

    if (value == NULL || value[0] < '0' || value[0] > '9')
        return -1;

    errno = 0;
    parsed = strtoull(value, &end, 10);

    if (errno != 0 || *end != '\0')
        return -1;

    *result = (uint64_t)parsed;
    return 0;
}

/**
 * @brief Parses a sending interval expressed in seconds.
 *
 * Converts the input into whole milliseconds.
 *
 * @param value Interval string.
 * @param interval_ms Parsed interval in milliseconds.
 * @return 0 on success, -1 on error.
 */
static int parse_interval(const char *value, int *interval_ms)
{
    char *end;
    double seconds;
    double milliseconds;

    if (value == NULL || *value == '\0')
        return -1;

    errno = 0;
    seconds = strtod(value, &end);

    if (errno != 0 || *end != '\0' || !isfinite(seconds))
        return -1;

    milliseconds = seconds * 1000.0;

    if (milliseconds < 1.0 || milliseconds > INT_MAX)
        return -1;

    *interval_ms = (int)(milliseconds + 0.5);

    return 0;
}

/**
 * @brief Parses a duration expressed in whole seconds.
 *
 * Converts seconds to milliseconds and prevents integer
 * overflow when the result is used by poll().
 *
 * @param value Input duration string.
 * @param result_ms Parsed duration in milliseconds.
 * @param allow_zero Whether zero is an accepted value.
 * @return 0 on success, -1 on invalid input.
 */
static int parse_seconds(const char *value, int *result_ms,
                         int allow_zero)
{
    uint64_t seconds;

    if (parse_uint64(value, &seconds) != 0)
        return -1;

    if (!allow_zero && seconds == 0)
        return -1;

    if (seconds > INT_MAX / 1000)
        return -1;

    *result_ms = (int)(seconds * 1000);

    return 0;
}

/**
 * @brief Converts a hexadecimal digit to its value.
 *
 * @param digit ASCII hexadecimal character.
 * @return Value 0..15, or -1 if invalid.
 */
static int hex_digit(char digit)
{
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + 10;
    return -1;
}

/**
 * @brief Parses a one-to-sixteen-byte hexadecimal pad pattern.
 *
 * Requires an even number of hexadecimal digits and stores
 * binary bytes for repeating inside each outgoing Echo Request.
 *
 * @param input Hexadecimal string from -p.
 * @param options Options receiving the parsed bytes.
 * @return 0 on success, -1 on invalid input.
 */
static int parse_pattern(const char *input, t_ping_options *options)
{
    size_t length;
    size_t i;
    int high;
    int low;

    if (input == NULL)
        return -1;

    length = strlen(input);
    if (length == 0 || length > PING_PATTERN_MAX_BYTES * 2 ||
        length % 2 != 0)
        return -1;

    for (i = 0; i < length / 2; i++)
    {
        high = hex_digit(input[i * 2]);
        low = hex_digit(input[i * 2 + 1]);
        if (high < 0 || low < 0)
            return -1;

        options->pattern[i] = (uint8_t)((high << 4) | low);
    }

    options->pattern_length = length / 2;
    return 0;
}

/**
 * @brief Parses command-line options and validates the target.
 *
 * Supports ICMP scheduling, numeric output, and hex patterns.
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
        {"numeric", no_argument, NULL, 'n'},
        {"pattern", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {"ttl", required_argument, NULL, OPT_TTL},
        {"count", required_argument, NULL, 'c'},
        {"interval", required_argument, NULL, 'i'},
        {"size", required_argument, NULL, 's'},
        {"timeout", required_argument, NULL, 'w'},
        {"linger", required_argument, NULL, 'W'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    uint64_t number;

    options->payload_size = PING_DEFAULT_PAYLOAD_SIZE;
    options->interval_ms = PING_DEFAULT_INTERVAL_MS;
    options->count = 0;
    options->ttl = 0;
    options->verbose = 0;
    options->numeric = 0;
    options->pattern_length = 0;
    options->timeout_ms = 0;
    options->linger_ms = PING_DEFAULT_LINGER_MS;
    opterr = 0;

    while ((opt = getopt_long(argc, argv, "v?nc:i:s:w:W:p:",
                            long_options, NULL)) != -1)
    {
        if (opt == 'v')
            options->verbose = 1;
        else if (opt == 'n')
            options->numeric = 1;
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
        else if (opt == 'c')
        {
            if (parse_uint64(optarg, &number) != 0 || number == 0)
            {
                fprintf(stderr, "ft_ping: invalid packet count\n");
                return PARSE_ERROR;
            }

            options->count = number;
        }
        else if (opt == 'i')
        {
            if (parse_interval(optarg, &options->interval_ms) != 0)
            {
                fprintf(stderr, "ft_ping: invalid interval\n");
                return PARSE_ERROR;
            }
        }
        else if (opt == 'p')
        {
            if (parse_pattern(optarg, options) != 0)
            {
                fprintf(stderr, "ft_ping: invalid pattern (1..16 hex bytes)\n");
                return PARSE_ERROR;
            }
        }
        else if (opt == 's')
        {
            if (parse_uint64(optarg, &number) != 0 ||
                number > PING_MAX_PAYLOAD_SIZE)
            {
                fprintf(stderr, "ft_ping: invalid payload size\n");
                return PARSE_ERROR;
            }

            options->payload_size = (size_t)number;
        }
        else if (opt == 'w')
        {
            if (parse_seconds(optarg, &options->timeout_ms, 0) != 0)
            {
                fprintf(stderr, "ft_ping: invalid timeout\n");
                return PARSE_ERROR;
            }
        }
        else if (opt == 'W')
        {
            if (parse_seconds(optarg, &options->linger_ms, 1) != 0)
            {
                fprintf(stderr, "ft_ping: invalid linger time\n");
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

