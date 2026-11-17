#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <netinet/in.h>

/**
 * @brief Stores command-line configuration for ft_ping.
 *
 * Contains user-provided options that control ping behavior.
 * Additional fields will be added for bonus features.
 */
typedef struct s_ping_options
{
    const char *target;
    int verbose;
} t_ping_options;

/**
 * @brief Stores the resolved IPv4 destination.
 *
 * Contains the binary socket address used for network
 * operations and its human-readable IPv4 representation.
 */
typedef struct s_ping_target
{
    struct sockaddr_in address;
    char ip_string[INET_ADDRSTRLEN];
} t_ping_target;

/**
 * @brief Represents possible command-line parsing results.
 */
typedef enum e_parse_result
{
    PARSE_ERROR = -1,
    PARSE_OK = 0,
    PARSE_HELP = 1
} t_parse_result;

/**
 * @brief Parses and validates command-line arguments.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 * @param options Pointer to the configuration structure.
 *
 * @return PARSE_OK if arguments are valid.
 * @return PARSE_HELP if help was requested.
 * @return PARSE_ERROR if arguments are invalid.
 */
t_parse_result parse_options(
    int argc,
    char **argv,
    t_ping_options *options
);

/**
 * @brief Prints the available command-line options.
 *
 * @return Nothing (void).
 */
void print_help(void);

/**
 * @brief Resolves a hostname or IPv4 string to an IPv4 address.
 *
 * Uses getaddrinfo() to obtain a binary IPv4 socket address
 * and inet_ntop() to generate its printable representation.
 *
 * @param hostname Hostname or IPv4 address to resolve.
 * @param target Output structure for the resolved address.
 *
 * @return 0 on success, -1 on failure.
 */
int resolve_target(const char *hostname, t_ping_target *target);

#endif