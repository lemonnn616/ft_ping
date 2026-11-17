#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define PING_DEFAULT_PAYLOAD_SIZE 56
#define PING_MAX_PAYLOAD_SIZE 65507
#define PING_SEQUENCE_COUNT 65536U

/**
 * @brief Stores the command-line configuration for ft_ping.
 *
 * Contains the destination hostname and options controlling
 * ICMP packet construction and program behavior.
 */
typedef struct s_ping_options
{
    const char *target;
    int verbose;
    size_t payload_size;
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
 * @brief Tracks one ICMP Echo Request.
 *
 * Stores its transmission timestamp and whether
 * a matching Echo Reply has already been received.
 */
typedef struct s_ping_request
{
    struct timespec sent_at;
    int sent;
    int received;
} t_ping_request;

/**
 * @brief Stores the complete runtime state of ft_ping.
 *
 * Contains configuration, socket information, ICMP
 * identifiers, request tracking, and packet counters.
 */
typedef struct s_ping_context
{
    t_ping_options options;
    t_ping_target target;
    int socket_fd;
    uint16_t identifier;
    uint16_t sequence;
    t_ping_request *requests;
    uint64_t transmitted;
    uint64_t received;
} t_ping_context;

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


void print_help(void);
int resolve_target(const char *hostname, t_ping_target *target);
int create_icmp_socket(void);
uint16_t icmp_checksum(const void *data, size_t length);
int send_icmp_request(t_ping_context *ctx);
int receive_icmp_reply(t_ping_context *ctx);
int get_monotonic_time(struct timespec *result);
int64_t monotonic_milliseconds(void);
double elapsed_milliseconds(const struct timespec *start,
                            const struct timespec *end);
int install_signal_handlers(void);
int stop_requested(void);
int run_ping_loop(t_ping_context *ctx);

#endif