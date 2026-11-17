#include "ft_ping.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>

#define PING_INTERVAL_MS 1000

/**
 * @brief Runs the continuous ICMP send/receive loop.
 *
 * Sends Echo Requests at one-second intervals.
 * Uses poll() to receive replies without delaying
 * the next scheduled transmission.
 *
 * Stops gracefully after SIGINT or SIGTERM.
 *
 * @param ctx Initialized ping application context.
 * @return 0 after graceful shutdown, -1 on error.
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
            if (send_icmp_request(ctx) != 0)
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

        if (pfd.revents & POLLIN)
        {
            if (receive_icmp_reply(ctx) != 0)
                return -1;
        }

        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            fprintf(stderr, "ft_ping: socket poll error\n");
            return -1;
        }
    }

    return 0;
}