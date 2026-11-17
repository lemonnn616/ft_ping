#include "ft_ping.h"

#include <stdio.h>

/**
 * @brief Reads the current monotonic clock value.
 *
 * Uses CLOCK_MONOTONIC to avoid discontinuities caused
 * by changes to the system's wall-clock time.
 *
 * @param result Pointer to the output timestamp.
 * @return 0 on success, -1 on failure.
 */
int get_monotonic_time(struct timespec *result)
{
    if (clock_gettime(CLOCK_MONOTONIC, result) == -1)
    {
        perror("ft_ping: clock_gettime");
        return -1;
    }

    return 0;
}

/**
 * @brief Returns the current monotonic time in milliseconds.
 *
 * Converts seconds and nanoseconds to integer milliseconds.
 * Useful for calculating poll() deadlines.
 *
 * @return Monotonic milliseconds, or -1 on failure.
 */
int64_t monotonic_milliseconds(void)
{
    struct timespec now;

    if (get_monotonic_time(&now) != 0)
        return -1;

    return (int64_t)now.tv_sec * 1000
           + now.tv_nsec / 1000000;
}

/**
 * @brief Calculates the elapsed time between two timestamps.
 *
 * Converts the difference between two timespec values
 * into milliseconds with fractional precision.
 *
 * @param start Starting timestamp.
 * @param end Ending timestamp.
 * @return Elapsed time in milliseconds.
 */
double elapsed_milliseconds(const struct timespec *start,
                            const struct timespec *end)
{
    double seconds;
    double nanoseconds;

    seconds = (double)(end->tv_sec - start->tv_sec);
    nanoseconds = (double)(end->tv_nsec - start->tv_nsec);

    return seconds * 1000.0 + nanoseconds / 1000000.0;
}