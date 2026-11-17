#include "ft_ping.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define PING_INTERVAL_MS 1000

/**
 * @brief Runs the continuous ICMP send/receive loop.
 *
 * Sends Echo Requests every second and processes incoming
 * ICMP packets independently of the send schedule.
 *
 * Handles recoverable socket errors without stopping
 * the application.
 *
 * @param ctx Initialized ping application context.
 * @return 0 after graceful shutdown, -1 on fatal error.
 */
int run_ping_loop(t_ping_context *ctx)
{
    struct pollfd pfd = {
        .fd = ctx->socket_fd,
        .events = POLLIN
    };

    int64_t next_send;
    int64_t now;
    int ready;
    int send_status;
    int pending_error;
    socklen_t error_length;

    next_send = monotonic_milliseconds();
    if (next_send < 0)
        return -1;

    while (!stop_requested())
    {
        now = monotonic_milliseconds();
        if (now < 0)
            return -1;

        if (now >= next_send)
        {
            send_status = send_icmp_request(ctx);
            if (send_status < 0)
                return -1;

            next_send += PING_INTERVAL_MS;

            if (next_send <= now)
                next_send = now + PING_INTERVAL_MS;

            continue;
        }

        pfd.revents = 0;

        ready = poll(&pfd, 1, (int)(next_send - now));

        if (ready < 0)
        {
            if (errno == EINTR)
                continue;

            perror("ft_ping: poll");
            return -1;
        }

        if (ready == 0)
            continue;

        if (pfd.revents & POLLNVAL)
        {
            fprintf(stderr, "ft_ping: invalid socket descriptor\n");
            return -1;
        }

        if ((pfd.revents & POLLIN) &&
            receive_icmp_reply(ctx) != 0)
            return -1;

        if (pfd.revents & POLLERR)
        {
            pending_error = 0;
            error_length = sizeof(pending_error);

            if (getsockopt(ctx->socket_fd,
                           SOL_SOCKET,
                           SO_ERROR,
                           &pending_error,
                           &error_length) == -1)
            {
                perror("ft_ping: getsockopt");
                return -1;
            }

            if (pending_error != 0 && ctx->options.verbose)
            {
                fprintf(stderr, "ft_ping: socket error: %s\n",
                        strerror(pending_error));
            }
        }

        if (pfd.revents & POLLHUP)
        {
            fprintf(stderr, "ft_ping: socket hangup\n");
            return -1;
        }
    }

    return 0;
}