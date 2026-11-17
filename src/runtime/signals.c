#include "ft_ping.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>

static volatile sig_atomic_t g_stop = 0;

/**
 * @brief Marks the application for graceful shutdown.
 *
 * Called when SIGINT or SIGTERM is received.
 * Only modifies a signal-safe flag.
 *
 * @param signal_number Received signal number (unused).
 * @return Nothing.
 */
static void handle_stop(int signal_number)
{
    (void)signal_number;
    g_stop = 1;
}

/**
 * @brief Installs handlers for SIGINT and SIGTERM.
 *
 * Configures signal handlers using sigaction().
 *
 * @return 0 on success, -1 on failure.
 */
int install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));

    action.sa_handler = handle_stop;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1)
    {
        perror("ft_ping: sigaction");
        return -1;
    }

    return 0;
}

/**
 * @brief Checks whether application termination was requested.
 *
 * @return 1 if a termination signal was received.
 * @return 0 otherwise.
 */
int stop_requested(void)
{
    return g_stop != 0;
}