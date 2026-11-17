#include "ft_ping.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>

/**
 * @brief Updates RTT statistics with a new measurement.
 *
 * Uses Welford's online algorithm for numerically stable
 * mean and variance calculation without storing samples.
 *
 * @param stats Statistics structure to update.
 * @param rtt_ms RTT measurement in milliseconds.
 * @return Nothing.
 */
void update_rtt_stats(t_ping_stats *stats, double rtt_ms)
{
    double delta;

    if (stats->rtt_samples == 0)
    {
        stats->min_rtt = rtt_ms;
        stats->max_rtt = rtt_ms;
    }
    else
    {
        if (rtt_ms < stats->min_rtt)
            stats->min_rtt = rtt_ms;

        if (rtt_ms > stats->max_rtt)
            stats->max_rtt = rtt_ms;
    }

    stats->rtt_samples++;

    delta = rtt_ms - stats->mean_rtt;

    stats->mean_rtt += delta / (double)stats->rtt_samples;

    stats->m2_rtt += delta * (rtt_ms - stats->mean_rtt);
}

/**
 * @brief Prints the final ping statistics.
 *
 * Calculates packet loss from unique received replies.
 * Prints RTT min/avg/max/stddev when samples exist.
 *
 * @param ctx Application context containing statistics.
 * @return Nothing.
 */
void print_statistics(const t_ping_context *ctx)
{
    const t_ping_stats *stats = &ctx->stats;
    double packet_loss = 0.0;
    double stddev;

    if (stats->transmitted > 0)
    {
        packet_loss = 100.0 *
            (double)(stats->transmitted - stats->received) /
            (double)stats->transmitted;
    }

    printf("--- %s ping statistics ---\n", ctx->options.target);

    printf("%" PRIu64 " packets transmitted, "
           "%" PRIu64 " packets received",
           stats->transmitted, stats->received);

    if (stats->duplicates > 0)
        printf(", +%" PRIu64 " duplicates", stats->duplicates);

    printf(", %.0f%% packet loss\n", packet_loss);

    if (stats->rtt_samples == 0)
        return;

    stddev = sqrt(stats->m2_rtt / (double)stats->rtt_samples);

    printf("round-trip min/avg/max/stddev = "
           "%.3f/%.3f/%.3f/%.3f ms\n",
           stats->min_rtt,
           stats->mean_rtt,
           stats->max_rtt,
           stddev);
}