#include "ft_ping.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

/**
 * @brief Handles a pending socket error.
 *
 * Reads and clears SO_ERROR. Reports the error
 * when verbose output is enabled.
 *
 * @param ctx Application context containing the socket.
 * @return 0 on success, -1 on getsockopt failure.
 */
static int handle_socket_error(const t_ping_context *ctx)
{
    int pending_error = 0;
    socklen_t length = sizeof(pending_error);

    if (getsockopt(ctx->socket_fd, SOL_SOCKET, SO_ERROR,
                   &pending_error, &length) == -1)
    {
        perror("ft_ping: getsockopt");
        return -1;
    }

    if (pending_error != 0 && ctx->options.verbose)
    {
        fprintf(stderr, "ft_ping: socket error: %s\n",
                strerror(pending_error));
    }

    return 0;
}

/**
 * @brief Sends ICMP requests and processes replies using poll().
 *
 * -w is an absolute deadline: when it coincides with a scheduled
 * transmission, that last request is sent before exiting, as
 * observed with GNU inetutils 2.0.
 *
 * After -c, the receive-only linger period starts at the next
 * scheduled send slot, not immediately after sendto().
 *
 * @param ctx Initialized application context.
 * @return 0 after normal termination, -1 on fatal error.
 */
int run_ping_loop(t_ping_context *ctx)
{
    struct pollfd pfd = {
        .fd = ctx->socket_fd,
        .events = POLLIN
    };

    int64_t next_send;
    int64_t timeout_deadline = INT64_MAX;
    int64_t final_deadline = 0;
    int64_t now;
    int64_t wake_at;
    uint64_t attempts = 0;
    int sending_done = 0;
    int ready;
    int send_status;

    next_send = monotonic_milliseconds();
    if (next_send < 0)
        return -1;

    if (ctx->options.timeout_ms > 0)
        timeout_deadline = next_send + ctx->options.timeout_ms;

    while (!stop_requested())
    {
        now = monotonic_milliseconds();
        if (now < 0)
            return -1;

        /*
         * The -w deadline is a hard stop; it does not start a
         * new -W wait. Include a transmission scheduled exactly
         * at the deadline before printing final statistics.
         */
        if (now >= timeout_deadline)
        {
            if (!sending_done && now >= next_send &&
                next_send <= timeout_deadline)
            {
                if (send_icmp_request(ctx) < 0)
                    return -1;
            }
            break;
        }

        if (sending_done)
        {
            if (ctx->stats.received == ctx->stats.transmitted ||
                now >= final_deadline)
                break;

            wake_at = final_deadline;
        }
        else if (now >= next_send)
        {
            send_status = send_icmp_request(ctx);
            if (send_status < 0)
                return -1;

            attempts++;

            now = monotonic_milliseconds();
            if (now < 0)
                return -1;

            next_send += ctx->options.interval_ms;
            if (next_send <= now)
                next_send = now + ctx->options.interval_ms;

            if (ctx->options.count > 0 &&
                attempts >= ctx->options.count)
            {
                sending_done = 1;
                final_deadline = next_send + ctx->options.linger_ms;
            }
            continue;
        }
        else
        {
            wake_at = next_send;
        }

        if (timeout_deadline < wake_at)
            wake_at = timeout_deadline;

        pfd.revents = 0;
        ready = poll(&pfd, 1, (int)(wake_at - now));

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
            if (handle_socket_error(ctx) != 0)
                return -1;
        }

        if (pfd.revents & POLLHUP)
        {
            fprintf(stderr, "ft_ping: socket hangup\n");
            return -1;
        }
    }

    return 0;
}
