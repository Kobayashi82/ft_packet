/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   packet.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/04 11:22:45 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/11 22:11:37 by vzurera-         ###   ########.fr       */
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
	#include <stddef.h>
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

	static int checksum_match(const void *data, uint16_t len, uint16_t checksum_offset) {
		uint8_t		copy[60];

		if (!data || len > sizeof(copy) || checksum_offset + sizeof(uint16_t) > len)
			return (0);
		memcpy(copy, data, len);
		*(uint16_t *)(copy + checksum_offset) = 0;
		return (*(const uint16_t *)((const uint8_t *)data + checksum_offset) == checksum(copy, len));
	}

	static int protocol_is_supported(uint8_t protocol) {
		return (protocol == IPPROTO_ICMP || protocol == IPPROTO_UDP || protocol == IPPROTO_TCP);
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

		static int trailing_padding_is_valid(const uint8_t *options, uint8_t start, uint8_t len) {
			while (start < len) {
				if (options[start] != EOOL && options[start] != NOP)
					return (0);
				start++;
			}
			return (1);
		}

		#pragma region "IP"

			static int ip_option_validate(t_packet *packet) {
				uint8_t		*options;
				uint8_t		len;
				uint8_t		index;
				uint8_t		opt_len;

				if (!packet || !packet->ip_option) return (0);

				if (!packet->ip) return (1);
				if (!packet->ip_option_len || packet->ip_option_len > sizeof(((t_ip_option *)0)->options)) return (1);
				if (packet->ip_option_len % 4) return (1);

				options = (uint8_t *)packet->ip_option;
				len = packet->ip_option_len;
				index = 0;
				while (index < len) {
					if (options[index] == EOOL)
						return (trailing_padding_is_valid(options, index + 1, len) ? 0 : 1);
					if (options[index] == NOP) {
						index++;
						continue;
					}
					if (index + 1 >= len) return (1);
					opt_len = options[index + 1];
					if (opt_len < 2 || index + opt_len > len) return (1);
					if (options[index] == TS) {
						if (opt_len < 8) return (1);
						if (((options[index + 3] & 0x0F) != 0) && ((options[index + 3] & 0x0F) != 1)) return (1);
						if (options[index + 2] < 5 || options[index + 2] > opt_len + 1) return (1);
					}
					else if (options[index] == RR) {
						if (opt_len < 7 || (opt_len - 3) % 4) return (1);
						if (options[index + 2] < 4 || options[index + 2] > opt_len + 1) return (1);
					}
					else if (options[index] == RTRALT) {
						if (opt_len != 4) return (1);
					}
					else return (1);
					index += opt_len;
				}

				return (0);
			}

			static int ip_validate(t_packet *packet) {
				uint8_t		ihl;
				uint16_t	header_len;
				uint16_t	total_len;
				uint16_t	expected_len;

				if (!packet || !packet->ip) return (0);

				if (packet->ethernet && ntohs(packet->ethernet->ethertype) != ETH_P_IP) return (1);
				if (packet->arp) return (1);
				if ((packet->ip->ver_ihl >> 4) != VERSION) return (1);
				ihl = packet->ip->ver_ihl & 0x0F;
				if (ihl < 5 || ihl > 15) return (1);
				header_len = ihl * 4;
				if (packet->ip_option && ihl != 5 + (packet->ip_option_len / 4)) return (1);
				if (!packet->ip_option && ihl != 5) return (1);
				total_len = ntohs(packet->ip->length);
				if (total_len < header_len) return (1);
				expected_len = packet->packet_len;
				if (packet->ethernet) expected_len -= sizeof(t_ethernet);
				if (expected_len != total_len) return (1);
				if (packet->icmp && packet->ip->protocol != IPPROTO_ICMP) return (1);
				if (packet->udp && packet->ip->protocol != IPPROTO_UDP) return (1);
				if (packet->tcp && packet->ip->protocol != IPPROTO_TCP) return (1);
				if (!packet->icmp && !packet->udp && !packet->tcp && !protocol_is_supported(packet->ip->protocol)) return (1);
				if (!checksum_match(packet->ip, header_len, offsetof(t_ip, checksum))) return (1);

				if (ip_option_validate(packet)) return (1);

				return (0);
			}

		#pragma endregion

		#pragma region "ICMP"

			static int icmp_validate(t_packet *packet) {
				uint16_t	icmp_len;

				if (!packet || !packet->icmp) return (1);
				if (ip_validate(packet)) return (1);
				if (!packet->ip) return (1);
				if (packet->ip->protocol != IPPROTO_ICMP) return (1);
				icmp_len = sizeof(t_icmp) + packet->payload_len;
				if (ntohs(packet->ip->length) != ((packet->ip->ver_ihl & 0x0F) * 4) + icmp_len) return (1);
				if (packet->icmp->type == ICMP_ECHO || packet->icmp->type == ICMP_ECHOREPLY || packet->icmp->type == ICMP_TIMESTAMP || packet->icmp->type == ICMP_TIMESTAMPREPLY) {
					if (packet->icmp->code != 0) return (1);
				}
				else if (packet->icmp->type == ICMP_DEST_UNREACH) {
					if (packet->icmp->code > NR_ICMP_UNREACH) return (1);
				}
				else if (packet->icmp->type == ICMP_TIME_EXCEEDED) {
					if (packet->icmp->code > 1) return (1);
				}
				else if (packet->icmp->type == ICMP_REDIRECT) {
					if (packet->icmp->code > 3) return (1);
				}
				else return (1);
				if (!checksum_match(packet->icmp, icmp_len, offsetof(t_icmp, checksum))) return (1);

				return (0);
			}

		#pragma endregion

		#pragma region "UDP"

			static int udp_validate(t_packet *packet) {
				t_udp		copy;
				uint16_t	header_len;
				uint16_t	udp_len;

				if (!packet || !packet->udp) return (1);
				if (ip_validate(packet)) return (1);
				if (!packet->ip) return (1);
				if (packet->ip->protocol != IPPROTO_UDP) return (1);
				header_len = (packet->ip->ver_ihl & 0x0F) * 4;
				udp_len = ntohs(packet->udp->length);
				if (udp_len != sizeof(t_udp) + packet->payload_len) return (1);
				if (ntohs(packet->ip->length) != header_len + udp_len) return (1);
				copy = *packet->udp;
				udp_set_checksum(&copy, packet->ip->src_addr, packet->ip->dst_addr, packet->payload_len, packet->payload);
				if (copy.checksum != packet->udp->checksum) return (1);

				return (0);
			}

		#pragma endregion

		#pragma region "TCP"

			static int tcp_option_validate(t_packet *packet) {
				uint8_t		*options;
				uint8_t		index;
				uint8_t		kind;
				uint8_t		opt_len;

				if (!packet) return (1);
				if (!packet->tcp_option) return (0);
				if (!packet->tcp_option_len || packet->tcp_option_len > sizeof(((t_tcp_option *)0)->options)) return (1);
				if (packet->tcp_option_len % 4) return (1);
				options = (uint8_t *)packet->tcp_option;
				index = 0;
				while (index < packet->tcp_option_len) {
					kind = options[index];
					if (kind == EOL)
						return (trailing_padding_is_valid(options, index + 1, packet->tcp_option_len) ? 0 : 1);
					if (kind == NOP) {
						index++;
						continue;
					}
					if (index + 1 >= packet->tcp_option_len) return (1);
					opt_len = options[index + 1];
					if (opt_len < 2 || index + opt_len > packet->tcp_option_len) return (1);
					if (kind == MSS) {
						if (opt_len != 4) return (1);
					}
					else if (kind == WSOPT) {
						if (opt_len != 3 || options[index + 2] > 14) return (1);
					}
					else if (kind == SACK_PERMITTED) {
						if (opt_len != 2) return (1);
					}
					else if (kind == SACK) {
						if (opt_len < 10 || opt_len > 34 || (opt_len - 2) % 8) return (1);
					}
					else if (kind == TIMESTAMPS) {
						if (opt_len != 10) return (1);
					}
					else return (1);
					index += opt_len;
				}

				return (0);
			}

			static int tcp_validate(t_packet *packet) {
				struct {
					t_tcp	tcp;
					uint8_t	options[40];
				} copy;
				uint8_t		data_offset;
				uint16_t	header_len;
				uint16_t	ip_header_len;
				uint16_t	tcp_len;

				if (!packet || !packet->tcp) return (1);
				if (ip_validate(packet)) return (1);
				if (!packet->ip) return (1);
				if (packet->ip->protocol != IPPROTO_TCP) return (1);
				if (packet->tcp->data_offset & 0x0F) return (1);
				data_offset = packet->tcp->data_offset >> 4;
				if (data_offset < 5 || data_offset > 15) return (1);
				header_len = data_offset * 4;
				if (header_len != sizeof(t_tcp) + packet->tcp_option_len) return (1);
				if (ntohs(packet->tcp->dst_port) == 0) return (1);
				if (!(packet->tcp->flags & URG) && packet->tcp->urg_ptr != 0) return (1);
				ip_header_len = (packet->ip->ver_ihl & 0x0F) * 4;
				tcp_len = ntohs(packet->ip->length) - ip_header_len;
				if (tcp_len != header_len + packet->payload_len) return (1);
				if (tcp_option_validate(packet)) return (1);
				memset(&copy, 0, sizeof(copy));
				copy.tcp = *packet->tcp;
				if (packet->tcp_option_len)
					memcpy(copy.options, packet->tcp_option, packet->tcp_option_len);
				tcp_set_checksum(&copy.tcp, packet->ip->src_addr, packet->ip->dst_addr, packet->tcp_option_len, packet->payload_len, packet->payload);
				if (copy.tcp.checksum != packet->tcp->checksum) return (1);


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

				uint16_t options_len = 0;
				if (packet->tcp_option)
					options_len = packet->tcp_option_len;

				uint8_t data_offset = (sizeof(t_tcp) + options_len) / 4;
				if (data_offset < 5 || data_offset > 15) return (1);
				tcp_set_data_offset(packet->tcp, data_offset);

				ip_complete(packet);

				tcp_set_checksum(packet->tcp, src_addr, dst_addr, options_len, packet->payload_len, packet->payload);

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
