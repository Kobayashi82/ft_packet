/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 11:22:45 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/05 21:33:16 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
   |  Header  |      Length      |             Checksum             |
   |----------|------------------|----------------------------------|
   | Ethernet | No               | FCS (managed by NIC)             |
   | ARP      | No               | No                               |
   | IP       | Header + Payload | Header                           |
   | ICMP     | No               | Header + Payload                 |
   | UDP      | Header + Payload | Pseudo-Header + Header + Payload |
   | TCP      | Header + Payload | Pseudo-Header + Header + Payload |
   | PAYLOAD  | No               | No                               |
*/

#pragma region "Includes"

	#include "packet.h"
	#include <string.h>

#pragma endregion

#pragma region "Checksum"

	static unsigned short checksum(void *data, int len) {
		unsigned long	sum = 0;
		unsigned short	*buf = data;

		while (len > 1) { sum += *buf++; len -= 2; }
		if (len == 1) sum += *(unsigned char *)buf;
		sum = (sum >> 16) + (sum & 0xFFFF);
		sum += (sum >> 16);

		return (unsigned short)(~sum);
	}

#pragma endregion

#pragma region "Headers"

	#pragma region "Add"

		#pragma region "Ethernet"

			static int ethernet_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (packet->ethernet || packet->ip || packet->ip_option || packet->icmp || packet->udp || packet->tcp || packet->tcp_option || packet->arp || packet->payload) return (1);

				memcpy((uint8_t *)packet, data, sizeof(t_ethernet));
				packet->ethernet = (t_ethernet *)packet;
				packet->packet_len = sizeof(t_ethernet);

				return (0);
			}

		#pragma endregion

		#pragma region "ARP"

			static int arp_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (!packet->ethernet) return (1);
				if (packet->arp || packet->ip || packet->ip_option || packet->icmp || packet->tcp || packet->tcp_option || packet->payload) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, data, sizeof(t_arp));
				packet->arp = (t_arp *)((uint8_t *)packet + packet->packet_len);
				packet->packet_len += sizeof(t_arp);

				return (0);
			}

		#pragma endregion

		#pragma region "IP"

			static int ip_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (packet->ip || packet->ip_option || packet->icmp || packet->udp || packet->tcp || packet->tcp_option || packet->arp || packet->payload) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, data, sizeof(t_ip));
				packet->ip = (t_ip *)((uint8_t *)packet + packet->packet_len);
				packet->packet_len += sizeof(t_ip);

				return (0);
			}

			static int ip_option_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (!packet->ip) return (1);
				if (packet->ip_option || packet->icmp || packet->udp || packet->tcp || packet->tcp_option || packet->arp || packet->payload) return (1);

				t_ip_option *opt = (t_ip_option *)data;

				if (!opt->length) return (1);
				if (opt->length % 4) return (1);

				uint8_t total_oct = ((opt->length + 3) / 4);

				memcpy((uint8_t *)packet + packet->packet_len, data, total_oct * 4);
				packet->ip_option = (t_ip_option *)((uint8_t *)packet + packet->packet_len);
				packet->ip_option_len = total_oct * 4;
				packet->packet_len += total_oct * 4;

				return (0);
			}

		#pragma endregion

		#pragma region "ICMP"

			static int icmp_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (packet->ethernet && !packet->ip) return (1);
				if (packet->ip_option && !packet->ip) return (1);
				if (packet->icmp || packet->udp || packet->tcp || packet->tcp_option || packet->arp || packet->payload) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, data, sizeof(t_icmp));
				packet->icmp = (t_icmp *)((uint8_t *)packet + packet->packet_len);
				packet->packet_len += sizeof(t_icmp);

				return (0);
			}

		#pragma endregion

		#pragma region "UDP"

			static int udp_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (packet->ethernet && !packet->ip) return (1);
				if (packet->ip_option && !packet->ip) return (1);
				if (packet->udp || packet->icmp || packet->tcp || packet->tcp_option || packet->arp || packet->payload) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, data, sizeof(t_udp));
				packet->udp = (t_udp *)((uint8_t *)packet + packet->packet_len);
				packet->packet_len += sizeof(t_udp);

				return (0);
			}

		#pragma endregion

		#pragma region "TCP"

			static int tcp_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (packet->ethernet && !packet->ip) return (1);
				if (packet->ip_option && !packet->ip) return (1);
				if (packet->tcp || packet->icmp || packet->udp || packet->tcp_option || packet->arp || packet->payload) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, data, sizeof(t_tcp));
				packet->tcp = (t_tcp *)((uint8_t *)packet + packet->packet_len);
				packet->packet_len += sizeof(t_tcp);

				return (0);
			}

			static int tcp_option_add(t_packet *packet, void *data) {
				if (!packet || !data) return (1);

				if (!packet->tcp) return (1);
				if (packet->tcp_option || packet->icmp || packet->udp || packet->arp || packet->payload) return (1);

				t_tcp_option *opt = (t_tcp_option *)data;

				if (!opt->length || opt->length % 4) return (1);
				if (opt->length > sizeof(opt->options)) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, opt->options, opt->length);
				packet->tcp_option = (t_tcp_option *)((uint8_t *)packet + packet->packet_len);
				packet->tcp_option_len = opt->length;
				packet->packet_len += opt->length;

				return (0);
			}

		#pragma endregion

		#pragma region "Payload"

			static int payload_add(t_packet *packet, void *data, uint16_t data_len) {
				if (!packet || !data || !data_len) return (1);

				if (packet->packet_len + data_len >= MAX_PACKET_LEN) return (1);
				if (packet->payload || (!packet->icmp && !packet->udp && !packet->tcp)) return (1);

				memcpy((uint8_t *)packet + packet->packet_len, data, data_len);
				packet->payload = (uint8_t *)packet + packet->packet_len;
				packet->payload_len = data_len;
				packet->packet_len += data_len;

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Validate"

		#pragma region "ARP"

			int arp_validate(t_packet *packet) {
				if (!packet || !packet->arp) return (0);

				if (!packet->ethernet) return (1);
				if (packet->ethernet->ethertype != ETH_P_ARP) return (1);
				if (memcmp(packet->arp->sha, packet->ethernet->src_mac, sizeof(packet->arp->sha))) return (1);
				if (packet->arp->oper == ARPOP_REPLY && packet->arp->tha != packet->ethernet->dst_mac) return (1);

				return (0);
			}

		#pragma endregion

		#pragma region "IP"

			static int ip_option_validate(t_packet *packet) {
				if (!packet || !packet->ip_option) return (0);

				if (!packet->ip) return (1);
				// invalid options

				return (0);
			}

			int ip_validate(t_packet *packet) {
				if (!packet || !packet->ip) return (0);

				// invalid ver and ihl
				// invalid dscp/ecn
				// total length < ip header
				// invalid protocol
				// invalid src addr
				// invalid dst addr

				ip_option_validate(packet);

				return (0);
			}

		#pragma endregion

		#pragma region "ICMP"

			int icmp_validate(t_packet *packet) {
				if (!packet) return (1);

				// dst port
				// 
				return (0);
			}

		#pragma endregion

		#pragma region "UDP"

			int udp_validate(t_packet *packet) {
				if (!packet) return (1);

				return (0);
			}

		#pragma endregion

		#pragma region "TCP"

			static int tcp_option_validate(t_packet *packet) {
				if (!packet) return (1);

				return (0);
			}

			int tcp_validate(t_packet *packet) {
				if (!packet) return (1);

				tcp_option_validate(packet);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Complete"

		#pragma region "IP"

			static int ip_complete(t_packet *packet) {
				if (!packet || !packet->ip) return (0);

				uint16_t total_len = packet->packet_len;

				if (packet->ethernet) total_len -= sizeof(t_ethernet);
				packet->ip->length = htons(total_len);
				packet->ip->checksum = 0;

				uint8_t	total_oct = 0;
				if (packet->ip_option) total_oct = packet->ip_option_len / 4;
				ip_set_ihl(packet->ip, 5 + total_oct);

				packet->ip->checksum = checksum(packet->ip, (5 + total_oct) * 4);

				return (0);
			}

		#pragma endregion

		#pragma region "ICMP"

			int icmp_complete(t_packet *packet) {
				if (!packet || !packet->icmp) return (1);

				ip_complete(packet);
				packet->icmp->checksum = 0;
				packet->icmp->checksum = checksum(packet->icmp, sizeof(t_icmp) + packet->payload_len);

				return (icmp_validate(packet));
			}

		#pragma endregion

		#pragma region "UDP"

			int udp_complete(t_packet *packet, uint32_t src_addr, uint32_t dst_addr) {
				if (!packet || !packet->udp) return (1);

				ip_complete(packet);

				packet->udp->length = htons(sizeof(t_udp) + packet->payload_len);
				packet->udp->checksum = 0;
				udp_set_checksum(packet->udp, src_addr, dst_addr, packet->payload_len, packet->payload);

				return (udp_validate(packet));
			}

		#pragma endregion

		#pragma region "TCP"

			int tcp_complete(t_packet *packet, uint32_t src_addr, uint32_t dst_addr) {
				if (!packet || !packet->tcp) return (1);

				uint16_t total_oct = 0;
				if (packet->tcp_option)
					total_oct = packet->tcp_option_len / 4;

				uint8_t data_offset = (sizeof(t_tcp) + total_oct) / 4;
				if (data_offset < 5 || data_offset > 15) return (1);
				tcp_set_data_offset(packet->tcp, data_offset);

				ip_complete(packet);

				tcp_set_checksum(packet->tcp, src_addr, dst_addr, total_oct, packet->payload_len, packet->payload);

				return (tcp_validate(packet));
			}

		#pragma endregion

	#pragma endregion

#pragma endregion

#pragma region "Packet"

	#pragma region "Add"

		int packet_add(t_packet *packet, void *data, uint32_t data_len, t_header_type header_type) {
			if (!packet || !data || !data_len) return (1);

			switch (header_type) {
				case ETHERNET:		return (ethernet_add(packet, data));			// 14 bytes
				case ARP:			return (arp_add(packet, data));					// 28 bytes
				case IP:			return (ip_add(packet, data));					// 20 bytes
				case IP_OPTION:		return (ip_option_add(packet, data));			// 0/40 bytes
				case ICMP:			return (icmp_add(packet, data));				// 8 bytes
				case UDP:			return (udp_add(packet, data));					// 8 bytes
				case TCP:			return (tcp_add(packet, data));					// 20 bytes
				case TCP_OPTION:	return (tcp_option_add(packet, data));			// 0/40 bytes
				case PAYLOAD:		return (payload_add(packet, data, data_len));
			}

			return (1);
		}

	#pragma endregion

	#pragma region "Clear"

		int packet_clear(t_packet *packet) {
			if (!packet) return (1);

			memset(packet, 0, sizeof(t_packet));
			packet->ethernet = NULL;
			packet->arp = NULL;
			packet->ip = NULL;
			packet->ip_option = NULL;
			packet->icmp = NULL;
			packet->udp = NULL;
			packet->tcp = NULL;
			packet->tcp_option = NULL;
			packet->payload = NULL;
			packet->payload_len = 0;
			packet->ip_option_len = 0;
			packet->tcp_option_len = 0;
			packet->packet_len = 0;

			return (0);
		}

	#pragma endregion

#pragma endregion
