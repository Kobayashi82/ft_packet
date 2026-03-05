/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 11:25:52 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/05 21:32:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include "frame/ethernet.h"
	#include "frame/ip.h"
	#include "frame/icmp.h"
	#include "frame/udp.h"
	#include "frame/tcp.h"
	#include "frame/arp.h"

#pragma endregion

#pragma region "Defines"

	#define MAX_PACKET_LEN	2048

#pragma endregion

#pragma region "Enumerators"

	typedef enum e_header_type { ETHERNET, IP, ICMP, UDP, TCP, ARP, PAYLOAD, IP_OPTION, TCP_OPTION } t_header_type;

#pragma endregion

#pragma region "Structures"

	typedef struct __attribute__((packed)) {
		uint8_t			packet[MAX_PACKET_LEN];
		uint32_t		packet_len;
		t_ethernet		*ethernet;
		t_arp			*arp;
		t_ip			*ip;
		t_ip_option		*ip_option;
		t_icmp			*icmp;
		t_udp			*udp;
		t_tcp			*tcp;
		t_tcp_option	*tcp_option;
		void			*payload;
		uint16_t		ip_option_len;
		uint16_t		tcp_option_len;
		uint16_t		payload_len;

	}	t_packet;

#pragma endregion

#pragma region "Methods"

	int	icmp_complete(t_packet *packet);
	int	udp_complete(t_packet *packet, uint32_t src_addr, uint32_t dst_addr);
	int	tcp_complete(t_packet *packet, uint32_t src_addr, uint32_t dst_addr);

	int	packet_add(t_packet *packet, void *data, uint32_t data_len, t_header_type header_type);
	int	packet_clear(t_packet *packet);

#pragma endregion
