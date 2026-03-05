/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ethernet.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 16:02:13 by vzurera-          #+#    #+#             */
/*   Updated: 2026/02/24 00:36:50 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <arpa/inet.h>

#pragma endregion

#pragma region "Structures"

	typedef struct __attribute__((packed)) {
		uint8_t		dst_mac[6];						// Destination MAC address
		uint8_t		src_mac[6];						// Source MAC address
		uint16_t	ethertype;						// Type of protocol (ETH_P_IP, ETH_P_ARP)
	}	t_ethernet;

#pragma endregion

#pragma region "Methods"

	int	ethernet_set_dst_mac(t_ethernet *header, const uint8_t *dst_mac);
	int	ethernet_set_src_mac(t_ethernet *header, const uint8_t *src_mac);
	int	ethernet_set_ethertype(t_ethernet *header, uint16_t ethertype);

	int	ethernet_create(t_ethernet *header, const uint8_t *dst_mac, const uint8_t *src_mac, uint16_t ethertype);

#pragma endregion
