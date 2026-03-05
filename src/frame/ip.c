/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ip.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:05 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/05 21:32:47 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*                                   IP Header (20/60 bytes)                                 
    ┌─────────────────────┬─────────────────────┬─────────────────────┬─────────────────────┐
    │          0          │          1          │          2          │          3          │
    ├──────────┬──────────┼───────────────┬─────┼─────────────────────┴─────────────────────┤
    │ Version  │   IHL    │   DSCP        │ ECN │               Total Length                │
    ├──────────┴──────────┴───────────────┴─────┼───────┬───────────────────────────────────┤
    │              Identification               │ Flags │          Fragment Offset          │
    ├─────────────────────┬─────────────────────┼───────┴───────────────────────────────────┤
    │    Time to Live     │      Protocol       │              Header Checksum              │
    ├─────────────────────┴─────────────────────┴───────────────────────────────────────────┤
    │                                    Source Address                                     │
    ├───────────────────────────────────────────────────────────────────────────────────────┤
    │                                  Destination Address                                  │
    ├───────────────────────────────────────────────────────────────────────────────────────┤
    │                                                                                       │
    │                                        Options                                        │
    │                                                                                       │
    └───────────────────────────────────────────────────────────────────────────────────────┘
    * Fragmentation not supported...
*/

#pragma region "Includes"

	#include "frame/ip.h"
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

#pragma region "Options"

	#pragma region "Set"

		static int option_set(t_ip_option *opt, uint8_t type, uint8_t len, const uint8_t *data) {
			if (!opt) return (1);

			if (type == NOP) {
				if (opt->length + 1 > sizeof(opt->options)) return (1);
				opt->options[opt->length++] = type;
				return (0);
			}

			if (type == EOOL) {
				if (opt->length >= sizeof(opt->options)) return (1);
				opt->options[opt->length++] = EOOL;
				while (opt->length % 4 && opt->length < sizeof(opt->options))
					opt->options[opt->length++] = NOP;
				return (0);
			}

			uint8_t total_len = 2 + len;
			if ((len && !data) || opt->length + total_len > sizeof(opt->options)) return (1);
			opt->options[opt->length] = type;
			opt->options[opt->length + 1] = total_len;
			if (data && len > 0) memcpy(&opt->options[opt->length + 2], data, len);

			opt->length += total_len;

			return (0);
		}

	#pragma endregion

	#pragma region "No Operation (NOP)"

		int option_set_nop(t_ip_option *opt) {
			if (!opt) return (1);

			return (option_set(opt, NOP, 0, NULL));
		}

	#pragma endregion

	#pragma region "End of Option List (EOOL)"

		int option_set_eool(t_ip_option *opt) {
			if (!opt) return (1);

			return (option_set(opt, EOOL, 0, NULL));
		}

	#pragma endregion

	#pragma region "TimeStamp (TS)"

		int option_timestamp_create(t_ip_option *opt, uint8_t num_timestamps, uint8_t flags) {
			if (!opt || !num_timestamps || num_timestamps > MAX_TIMESTAMPS) return (1);

			uint8_t entry_size = (flags == 1) ? 8 : 4;			// IP + timestamp o solo timestamp
			uint8_t len = 4 + (num_timestamps * entry_size);	// total len: header(4) + timestamps

			if (len > sizeof(opt->options)) return (1);

			uint8_t data[38];
			memset(data, 0, sizeof(data));

			data[0] = 5;										// Pointer (primer timestamp empieza en byte 5)
			data[1] = flags & 0xF;								// Flags (4 bits bajo), overflow (4 bits alto) 0

			return (option_set(opt, TS, len - 2, data));
		}


	#pragma endregion

	#pragma region "Record Route (RR)"

		int option_set_record_route(t_ip_option *opt, uint8_t num_addresses) {
			if (!opt || !num_addresses || num_addresses > MAX_RR_ADDRESSES) return (1);

			uint8_t len = 3 + (num_addresses * 4);

			if (len > sizeof(opt->options)) return (1);

			uint8_t data[38];
			memset(data, 0, sizeof(data));
			data[0] = 4;       					// pointer start at byte 4 (first address)

			return (option_set(opt, RR, len - 2, data));
		}

	#pragma endregion

	#pragma region "Router Alert (RTRALT)"

		int option_set_router_alert(t_ip_option *opt, uint16_t alert_value) {
			if (!opt) return (1);

			uint16_t be_value = htons(alert_value);
			return option_set(opt, RTRALT, sizeof(be_value), (uint8_t *)&be_value);
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	#pragma region "Ver/IHL"

		int ip_set_ihl(t_ip *header, uint8_t ihl) {
			if (!header || ihl < 5 || ihl > 15) return (1);

			header->ver_ihl = (VERSION << 4) | (ihl & 0x0F);

			return (0);
		}

	#pragma endregion

	#pragma region "DSCP/ECN"

		#pragma region "Set ToS"

			int ip_set_tos(t_ip *header, uint8_t tos) {
				if (!header || tos > 252) return (1);

				uint8_t dscp = (tos >> 2) & DSCP_MASK;
				header->dscp_ecn = (header->dscp_ecn & ECN_FIELD_MASK) | ((dscp & DSCP_MASK) << 2);

				return (0);
			}

		#pragma endregion

		#pragma region "Set DSCP"

			int ip_set_dscp(t_ip *header, uint8_t dscp) {
				if (!header || dscp > 63) return (1);

				header->dscp_ecn = (header->dscp_ecn & ECN_FIELD_MASK) | ((dscp & DSCP_MASK) << 2);

				return (0);
			}

		#pragma endregion

		#pragma region "Set ECN"

			int ip_set_ecn(t_ip *header, uint8_t ecn) {
				if (!header || ecn > 3) return (1);

				header->dscp_ecn = (header->dscp_ecn & DSCP_FIELD_MASK) | (ecn & ECN_MASK);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "Length"

		int ip_set_length(t_ip *header, uint16_t data_len) {
			if (!header) return (1);

			header->length = htons(sizeof(t_ip) + data_len);

			return (0);
		}

	#pragma endregion

	#pragma region "ID"

		int ip_set_id(t_ip *header, uint16_t id) {
			if (!header) return (1);

			header->id = htons(id);

			return (0);
		}

	#pragma endregion

	#pragma region "Fragment"

		#pragma region "Set Don't Fragment"

			int ip_set_df(t_ip *header, uint8_t frag_df) {
				if (!header) return (1);

				uint16_t frag = ntohs(header->frag);
				if (frag_df) frag |= DF_FLAG; else frag &= ~DF_FLAG;

				header->frag = htons(frag);

				return (0);
			}

		#pragma endregion

		#pragma region "Set More Fragments"

			int ip_set_mf(t_ip *header, uint8_t frag_mf) {
				if (!header) return (1);

				uint16_t frag = ntohs(header->frag);
				if (frag_mf) frag |= MF_FLAG; else frag &= ~MF_FLAG;

				header->frag = htons(frag);

				return (0);
			}

		#pragma endregion

		#pragma region "Set Fragment Offset"

			int ip_set_frag_offset(t_ip *header, uint16_t frag_offset) {
				if (!header || frag_offset > 8191) return (1);

				uint16_t frag = ntohs(header->frag);
				frag = (frag & ~FRAG_OFFSET_MASK) | (frag_offset & FRAG_OFFSET_MASK);
				header->frag = htons(frag);

				return (0);
			}

		#pragma endregion

	#pragma endregion

	#pragma region "TTL"

		int ip_set_ttl(t_ip *header, uint8_t ttl) {
			if (!header) return (1);

			header->ttl = ttl;

			return (0);
		}

	#pragma endregion

	#pragma region "Protocol"

		int ip_set_protocol(t_ip *header, uint8_t protocol) {
			if (!header) return (1);

			header->protocol = protocol;

			return (0);
		}

	#pragma endregion

	#pragma region "Checksum"

		int ip_set_checksum(t_ip *header) {
			if (!header) return (1);

			header->checksum = 0;
			header->checksum = htons(checksum(header, sizeof(t_ip)));

			return (0);
		}

	#pragma endregion

	#pragma region "SRC Address"

		int ip_set_src_addr(t_ip *header, uint32_t src_addr) {
			if (!header) return (1);

			header->src_addr = src_addr;

			return (0);
		}

	#pragma endregion

	#pragma region "DST Address"

		int ip_set_dst_addr(t_ip *header, uint32_t dst_addr) {
			if (!header) return (1);

			header->dst_addr = dst_addr;

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Create"

	// Simplified: dscp=0, ecn=0, frag_df=1, frag_mf=0, frag_offset=0, ttl=64
	int ip_create(t_ip *header, uint8_t dscp, uint8_t ecn, uint16_t data_len, uint16_t id, uint8_t frag_df, uint8_t frag_mf, uint16_t frag_offset, uint8_t ttl, uint8_t protocol, uint32_t src_addr, uint32_t dst_addr) {
		if (!header || dscp > 63 || ecn > 3) return (1);

		ip_set_ihl(header, IHL);
		ip_set_dscp(header, dscp);
		ip_set_ecn(header, ecn);
		ip_set_length(header, data_len);
		ip_set_id(header, id);
		ip_set_df(header, frag_df);
		ip_set_mf(header, frag_mf);
		ip_set_frag_offset(header, frag_offset);
		ip_set_ttl(header, ttl);
		ip_set_protocol(header, protocol);
		ip_set_src_addr(header, src_addr);
		ip_set_dst_addr(header, dst_addr);
		ip_set_checksum(header);

		return (0);
	}

#pragma endregion
