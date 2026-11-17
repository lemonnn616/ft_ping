#include "ft_ping.h"

/**
 * @brief Computes the Internet checksum for an ICMP message.
 *
 * Treats input as a sequence of big-endian 16-bit words.
 * The checksum field must be zero before calling.
 *
 * The returned value can be assigned directly to the
 * checksum field of struct icmphdr.
 *
 * @param data Pointer to ICMP message bytes.
 * @param length Length of the message in bytes.
 * @return 16-bit one's-complement checksum in network byte order.
 */
uint16_t icmp_checksum(const void *data, size_t length)
{
    const uint8_t *bytes = data;
    uint32_t sum = 0;
    size_t i = 0;

    while (i + 1 < length)
    {
        sum += ((uint16_t)bytes[i] << 8) | bytes[i + 1];
        i += 2;
    }

    if (i < length)
        sum += (uint16_t)bytes[i] << 8;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return htons((uint16_t)~sum);
}