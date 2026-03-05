/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   udp.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:30 by vzurera-          #+#    #+#             */
/*   Updated: 2026/02/25 12:33:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <netinet/in.h>

#pragma endregion

#pragma region "Structures"

	typedef struct __attribute__((__packed__)) {
		uint16_t	src_port;						// Source port
		uint16_t	dst_port;						// Destination port
		uint16_t	length;							// (header + payload)
		uint16_t	checksum;						// (pseudo header + header + payload)
	}	t_udp;

#pragma endregion

#pragma region "Methods"

	int	udp_set_src_port(t_udp *header, uint16_t src_port);
	int	udp_set_dst_port(t_udp *header, uint16_t dst_port);
	int	udp_set_length(t_udp *header, uint16_t data_len);
	int	udp_set_checksum(t_udp *header, uint32_t src_addr, uint32_t dst_addr, uint16_t data_len, const void *data);

	int	udp_create(t_udp *header, uint16_t src_port, uint16_t dst_port, uint16_t data_len);
	int	udp_create_checksum(t_udp *header, uint16_t src_port, uint16_t dst_port, uint32_t src_addr, uint32_t dst_addr, uint16_t data_len, const void *data);

#pragma endregion
