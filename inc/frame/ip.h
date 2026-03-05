/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ip.h                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:30 by vzurera-          #+#    #+#             */
/*   Updated: 2026/02/25 12:34:07 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <netinet/in.h>

#pragma endregion

#pragma region "Defines"

	#define VERSION					4				// Version = 4
	#define IHL						5				// IHL = 5

	#define DSCP_MASK				0x3F			// 6 bits (0-63)
	#define ECN_MASK				0x03			// 2 bits (0-3)
	#define DSCP_FIELD_MASK			0xFC			// Bits 7-2 (preserve DSCP  clean ECN)
	#define ECN_FIELD_MASK			0x03			// Bits 1-0 (preserve ECN   clean DSCP)

	#define DF_FLAG					0x4000			// Don't Fragment  (bit 14)
	#define MF_FLAG					0x2000			// More Fragments  (bit 13)
	#define FRAG_OFFSET_MASK		0x1FFF			// Fragment offset (bits 12-0)

	// Options
	#define EOOL					0x00			// End of Option List
	#define NOP						0x01			// No Operation

	#define TS						0x44			// Time Stamp
	#define MAX_TIMESTAMPS			9				// Time Stamp maximum

	#define RR						0x07			// Record Route
	#define MAX_RR_ADDRESSES		9				// Record Route maximum

	#define RTRALT					0x94			// Router Alert
	#define RTRALT_MLD				0				// MLD (Multicast Listener Discovery)
	#define RTRALT_RESERVED			1				// Reserved for future use
	#define RTRALT_RSVP				136				// RSVP (Resource Reservation Protocol)
	#define RTRALT_IGMP				512				// IGMP (Internet Group Management Protocol)
	#define RTRALT_VRRP				1024			// VRRP (Virtual Router Redundancy Protocol)


#pragma endregion

#pragma region "Structures"

	typedef struct __attribute__((__packed__)) {
		uint8_t		ver_ihl;						// Version (4 bits) + IHL in 32-bit words (4 bits, min=5, max=15)
		uint8_t		dscp_ecn;						// DSCP (6 bits, differentiated services) + ECN (2 bits, congestion notification)
		uint16_t	length;							// Total length in bytes, header + payload (min=20, max=65535)
		uint16_t	id;								// Identification, used for fragment reassembly
		uint16_t	frag;							// Flags (3 bits: reserved, DF, MF) + Fragment offset (13 bits)
		uint8_t		ttl;							// Time to live, decremented by each router
		uint8_t		protocol;						// Upper layer protocol (TCP=6, UDP=17, ICMP=1...)
		uint16_t	checksum;						// Header checksum (excludes payload)
		uint32_t	src_addr;						// Source IP address (network byte order)
		uint32_t	dst_addr;						// Destination IP address (network byte order)
	}	t_ip;

	typedef struct __attribute__((__packed__)) {
		uint8_t		options[40];					// Options data, max 40 bytes (IHL max=15, 15-5=10 words = 40 bytes)
		uint8_t		length;							// Used bytes in options, must be multiple of 4 and end with EOOL
	}	t_ip_option;

#pragma endregion

#pragma region "Methods"

	int	option_set_nop(t_ip_option *opt);
	int	option_set_eool(t_ip_option *opt);
	int	option_timestamp_create(t_ip_option *opt, uint8_t num_timestamps, uint8_t flags);
	int	option_set_record_route(t_ip_option *opt, uint8_t num_addresses);
	int	option_set_router_alert(t_ip_option *opt, uint16_t alert_value);

	int ip_set_ihl(t_ip *header, uint8_t ihl);
	int	ip_set_tos(t_ip *header, uint8_t tos);
	int	ip_set_dscp(t_ip *header, uint8_t dscp);
	int	ip_set_ecn(t_ip *header, uint8_t ecn);
	int	ip_set_length(t_ip *header, uint16_t data_len);
	int	ip_set_id(t_ip *header, uint16_t id);
	int	ip_set_df(t_ip *header, uint8_t frag_df);
	int	ip_set_mf(t_ip *header, uint8_t frag_mf);
	int	ip_set_frag_offset(t_ip *header, uint16_t frag_offset);
	int	ip_set_ttl(t_ip *header, uint8_t ttl);
	int	ip_set_protocol(t_ip *header, uint8_t protocol);
	int	ip_set_checksum(t_ip *header);
	int	ip_set_src_addr(t_ip *header, uint32_t src_addr);
	int	ip_set_dst_addr(t_ip *header, uint32_t dst_addr);

	int ip_create(t_ip *header, uint8_t dscp, uint8_t ecn, uint16_t data_len, uint16_t id, uint8_t frag_df, uint8_t frag_mf, uint16_t frag_offset, uint8_t ttl, uint8_t protocol, uint32_t src_addr, uint32_t dst_addr);

#pragma endregion
