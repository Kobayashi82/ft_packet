/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   udp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:05 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/05 21:32:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*                                    UDP Header (8 bytes)                                   
    ┌─────────────────────┬─────────────────────┬─────────────────────┬─────────────────────┐
    │          0          │          1          │          2          │          3          │
    ├─────────────────────┴─────────────────────┼─────────────────────┴─────────────────────┤
    │                Source Port                │             Destination Port              │
    ├───────────────────────────────────────────┼───────────────────────────────────────────┤
    │                  Length                   │                 Checksum                  │
    ├───────────────────────────────────────────┴───────────────────────────────────────────┤
    │                                                                                       │
    │                                        Payload                                        │
    │                                                                                       │
    └───────────────────────────────────────────────────────────────────────────────────────┘
*/

#pragma region "Includes"

	#include "frame/udp.h"

#pragma endregion

#pragma region "Checksum"

	static unsigned long checksum_partial(const void *data, int len) {
		unsigned long			sum = 0;
		const unsigned short	*buf = data;

		while (len > 1) { sum += *buf++; len -= 2; }
		if (len == 1) sum += *(unsigned char *)buf;

		return (sum);
	}

	static unsigned short checksum(unsigned long total_sum) {
		total_sum = (total_sum >> 16) + (total_sum & 0xFFFF);
		total_sum += (total_sum >> 16);

		return (unsigned short)(~total_sum);
	}

#pragma endregion

#pragma region "Set"

	#pragma region "Port"

		#pragma region "SRC Port"

			int udp_set_src_port(t_udp *header, uint16_t src_port) {
				if (!header) return (1);

				header->src_port = htons(src_port);

				return (0);
			}

		#pragma endregion

		#pragma region "DST Port"

			int udp_set_dst_port(t_udp *header, uint16_t dst_port) {
				if (!header) return (1);

				header->dst_port = htons(dst_port);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Length"

		int udp_set_length(t_udp *header, uint16_t data_len) {
			if (!header) return (1);

			header->length = htons(sizeof(t_udp) + data_len);

			return (0);
		}

	#pragma endregion

	#pragma region "Checksum"

		int udp_set_checksum(t_udp *header, uint32_t src_addr, uint32_t dst_addr, uint16_t data_len, const void *data) {
			if (!header) return (1);

			unsigned long total_sum = 0;

			struct __attribute__((__packed__)) {
				uint32_t	src_addr;				// Source IP address
				uint32_t	dst_addr;				// Destination IP address
				uint8_t		zeroes;					// Reserved, must be zero
				uint8_t		protocol;				// Protocol (UDP = 17)
				uint16_t	length;					// UDP length (header + data)
			}	pseudo;

			pseudo.src_addr = src_addr;
			pseudo.dst_addr = dst_addr;
			pseudo.zeroes = 0;
			pseudo.protocol = IPPROTO_UDP;
			pseudo.length = htons(sizeof(t_udp) + data_len);
			total_sum += checksum_partial(&pseudo, sizeof(pseudo));

			header->length = htons(sizeof(t_udp) + data_len);
			header->checksum = 0;
			total_sum += checksum_partial(header, sizeof(t_udp));

			if (data && data_len > 0) total_sum += checksum_partial(data, data_len);

			header->checksum = htons(checksum(total_sum));

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Create"

	int udp_create(t_udp *header, uint16_t src_port, uint16_t dst_port, uint16_t data_len) {
		if (!header) return (1);

		header->src_port = htons(src_port);
		header->dst_port = htons(dst_port);
		header->length = htons(sizeof(t_udp) + data_len);
		header->checksum = 0;

		return (0);
	}

	int udp_create_checksum(t_udp *header, uint16_t src_port, uint16_t dst_port, uint32_t src_addr, uint32_t dst_addr, uint16_t data_len, const void *data) {
		if (!data) data_len = 0;
		if (udp_create(header, src_port, dst_port, data_len)) return (1);
		return (udp_set_checksum(header, src_addr, dst_addr, data_len, data));
	}

#pragma endregion
