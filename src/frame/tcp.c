/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tcp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:05 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/05 21:33:17 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*                                  TCP Header (20/60 bytes)                                 
    ┌─────────────────────┬─────────────────────┬─────────────────────┬─────────────────────┐
    │          0          │          1          │          2          │          3          │
    ├─────────────────────┴─────────────────────┼─────────────────────┴─────────────────────┤
    │                Source Port                │             Destination Port              │
    ├───────────────────────────────────────────┴───────────────────────────────────────────┤
    │                                    Sequence Number                                    │
    ├───────────────────────────────────────────────────────────────────────────────────────┤
    │                 Acknowledgement Number (meaningful when ACK bit set)                  │
    ├──────────┬──────────┬──┬─┬─┬─┬─┬─┬─┬─┬─┬──┬───────────────────────────────────────────┤
    │   Data   │          │  │C│E│U│A│R│P│S│F│  │                                           │
    │  Offset  │ Reserved │  │W│C│R│C│S│S│Y│I│  │                  Windows                  │
    │          │          │  │R│E│G│K│T│H│N│N│  │                                           │
    ├──────────┴──────────┴──┴─┴─┴─┴─┴─┴─┴─┴─┴──┼───────────────────────────────────────────┤
    │                 Checksum                  │  Urgent Pointer (only when URG bit set)   │
    ├───────────────────────────────────────────┴───────────────────────────────────────────┤
    │               (Options) If present, Data Offset will be greater than 5                │
    │                  Padded with zeroes to a multiple of 32 bits, since                   │
    │                Data Offset counts words of 4 octets (not implemented)                 │
    ├───────────────────────────────────────────────────────────────────────────────────────┤
    │                                                                                       │
    │                                         Data                                          │
    │                                                                                       │
    └───────────────────────────────────────────────────────────────────────────────────────┘
*/

#pragma region "Includes"

	#include "frame/tcp.h"
	#include <string.h>

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

		return ((unsigned short)(~total_sum));
	}

#pragma endregion

#pragma region "Options"

	#pragma region "Set"

		static int tcp_option_set(t_tcp_option *opt, uint8_t kind, uint8_t len, const uint8_t *data) {
			if (!opt) return (1);

			if (kind == EOL || kind == NOP) {
				if (opt->length + 1 > sizeof(opt->options)) return (1);
				opt->options[opt->length++] = kind;
				return (0);
			}

			// Multi-byte options: kind + length + data
			uint8_t total = 2 + len;
			if ((len && !data) || opt->length + total > sizeof(opt->options)) return (1);

			opt->options[opt->length]     = kind;
			opt->options[opt->length + 1] = total;
			if (data && len > 0) memcpy(&opt->options[opt->length + 2], data, len);

			opt->length += total;

			return (0);
		}

	#pragma endregion

	#pragma region "End of Option List (EOL)"

		// Pads the options buffer to the next multiple of 4 bytes using EOL then NOPs
		int tcp_option_set_eol(t_tcp_option *opt) {
			if (!opt) return (1);

			if (opt->length >= sizeof(opt->options)) return (1);
			opt->options[opt->length++] = EOL;

			// Pad to 4-byte boundary with NOPs
			while (opt->length % 4 && opt->length < (uint8_t)sizeof(opt->options))
				opt->options[opt->length++] = NOP;

			return (0);
		}

	#pragma endregion

	#pragma region "No Operation (NOP)"

		int tcp_option_set_nop(t_tcp_option *opt) {
			if (!opt) return (1);

			return (tcp_option_set(opt, NOP, 0, NULL));
		}

	#pragma endregion

	#pragma region "Maximum Segment Size (MSS)"

		// mss: maximum segment size in bytes (commonly 1460 for Ethernet)
		// Only valid in SYN packets
		int tcp_option_set_mss(t_tcp_option *opt, uint16_t mss) {
			if (!opt) return (1);

			uint16_t be = htons(mss);

			return (tcp_option_set(opt, MSS, sizeof(be), (uint8_t *)&be));
		}

	#pragma endregion

	#pragma region "Window Scale (WSOPT)"

		// shift: scale factor (0-14), advertised window is multiplied by 2^shift
		// Only valid in SYN packets; requires a NOP before it for alignment
		int tcp_option_set_window_scale(t_tcp_option *opt, uint8_t shift) {
			if (!opt || shift > 14) return (1);

			return (tcp_option_set(opt, WSOPT, sizeof(shift), &shift));
		}

	#pragma endregion

	#pragma region "SACK Permitted"

		// Signals that the sender supports Selective Acknowledgement
		// Only valid in SYN packets
		int tcp_option_set_sack_permitted(t_tcp_option *opt) {
			if (!opt) return (1);

			return (tcp_option_set(opt, SACK_PERMITTED, 0, NULL));
		}

	#pragma endregion

	#pragma region "SACK (Selective Acknowledgement)"

		// num_blocks: number of SACK blocks (1-4), each block is a pair of 32-bit sequence numbers
		// blocks: array of [left_edge, right_edge] pairs in host byte order
		int tcp_option_set_sack(t_tcp_option *opt, uint8_t num_blocks, const uint32_t *blocks) {
			if (!opt || !blocks || !num_blocks || num_blocks > 4) return (1);

			uint8_t	data[32];
			uint8_t	len = num_blocks * 8;

			for (uint8_t i = 0; i < num_blocks * 2; i++) {
				uint32_t be = htonl(blocks[i]);
				memcpy(&data[i * 4], &be, 4);
			}

			return (tcp_option_set(opt, SACK, len, data));
		}

	#pragma endregion

	#pragma region "Timestamps (TSOPT)"

		// tsval: sender's timestamp value
		// tsecr: echo reply (set to 0 on SYN; mirrors the received tsval on subsequent packets)
		// Requires two NOPs before it to align to 4 bytes (NOP NOP kind len tsval tsecr = 12 bytes)
		int tcp_option_set_timestamps(t_tcp_option *opt, uint32_t tsval, uint32_t tsecr) {
			if (!opt) return (1);

			uint8_t	data[8];
			uint32_t be_val  = htonl(tsval);
			uint32_t be_ecr  = htonl(tsecr);

			memcpy(data,     &be_val, 4);
			memcpy(data + 4, &be_ecr, 4);

			return (tcp_option_set(opt, TIMESTAMPS, sizeof(data), data));
		}

	#pragma endregion

#pragma endregion

#pragma region "Set"

	#pragma region "Source Port"

		int tcp_set_src_port(t_tcp *header, uint16_t src_port) {
			if (!header) return (1);

			header->src_port = htons(src_port);

			return (0);
		}

	#pragma endregion

	#pragma region "Destination Port"

		int tcp_set_dst_port(t_tcp *header, uint16_t dst_port) {
			if (!header) return (1);

			header->dst_port = htons(dst_port);

			return (0);
		}

	#pragma endregion

	#pragma region "Sequence Number"

		int tcp_set_seq(t_tcp *header, uint32_t seq) {
			if (!header) return (1);

			header->seq = htonl(seq);

			return (0);
		}

	#pragma endregion

	#pragma region "Acknowledgement Number"

		int tcp_set_ack_num(t_tcp *header, uint32_t ack_num) {
			if (!header) return (1);

			header->ack_num = htonl(ack_num);

			return (0);
		}

	#pragma endregion

	#pragma region "Data Offset"

		// data_offset: number of 32-bit words in the TCP header (min 5, max 15)
		// Stored in the high nibble of the byte; low nibble is reserved (must be 0)
		int tcp_set_data_offset(t_tcp *header, uint8_t data_offset) {
			if (!header || data_offset < 5 || data_offset > 15) return (1);

			header->data_offset = (data_offset & 0x0F) << 4;

			return (0);
		}

	#pragma endregion

	#pragma region "Flags"

		int tcp_set_flag_fin(t_tcp *header, uint8_t value) {
			if (!header) return (1);
			
			if (value)	header->flags |= FIN;
			else		header->flags &= ~FIN;

			return (0);
		}

		int tcp_set_flag_syn(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= SYN;
			else		header->flags &= ~SYN;

			return (0);
		}

		int tcp_set_flag_rst(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= RST;
			else		header->flags &= ~RST;

			return (0);
		}

		int tcp_set_flag_psh(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= PSH;
			else		header->flags &= ~PSH;

			return (0);
		}

		int tcp_set_flag_ack(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= ACK;
			else		header->flags &= ~ACK;

			return (0);
		}

		int tcp_set_flag_urg(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= URG;
			else		header->flags &= ~URG;

			return (0);
		}

		int tcp_set_flag_ece(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= ECE;
			else		header->flags &= ~ECE;

			return (0);
		}

		int tcp_set_flag_cwr(t_tcp *header, uint8_t value) {
			if (!header) return (1);

			if (value)	header->flags |= CWR;
			else		header->flags &= ~CWR;

			return (0);
		}

	#pragma endregion

	#pragma region "Window"

		int tcp_set_window(t_tcp *header, uint16_t window) {
			if (!header) return (1);

			header->window = htons(window);

			return (0);
		}

	#pragma endregion

	#pragma region "Urgent Pointer"

		// urg_ptr is only meaningful when the URG flag is set
		int tcp_set_urg_ptr(t_tcp *header, uint16_t urg_ptr) {
			if (!header) return (1);

			header->urg_ptr = htons(urg_ptr);

			return (0);
		}

	#pragma endregion

	#pragma region "Checksum"

		int tcp_set_checksum(t_tcp *header, uint32_t src_addr, uint32_t dst_addr, uint16_t options_len, uint16_t data_len, const void *data) {
			if (!header) return (1);

			unsigned long total_sum = 0;

			struct __attribute__((__packed__)) {
				uint32_t src_addr;
				uint32_t dst_addr;
				uint8_t  zeroes;
				uint8_t  protocol;
				uint16_t length;
			} pseudo;

			uint16_t tcp_total_len = sizeof(t_tcp) + options_len + data_len;

			pseudo.src_addr = src_addr;
			pseudo.dst_addr = dst_addr;
			pseudo.zeroes = 0;
			pseudo.protocol = IPPROTO_TCP;
			pseudo.length = htons(tcp_total_len);
			total_sum += checksum_partial(&pseudo, sizeof(pseudo));

			header->checksum = 0;
			total_sum += checksum_partial(header, sizeof(t_tcp));

			// Sumar opciones si existen
			if (options_len > 0) {
				const void *options_ptr = (const void *)(header + 1);
				total_sum += checksum_partial(options_ptr, options_len);
			}

			// Sumar payload si existe
			if (data && data_len > 0)
				total_sum += checksum_partial(data, data_len);

			header->checksum = htons(checksum(total_sum));

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Create"

	// flags: bitmask using defines (e.g. SYN | ACK).
	int tcp_create(t_tcp *header, uint16_t src_port, uint16_t dst_port, uint32_t seq, uint32_t ack_num, uint8_t flags, uint16_t window, uint16_t urg_ptr) {
		if (!header || !dst_port) return (1);

		tcp_set_src_port(header, src_port);
		tcp_set_dst_port(header, dst_port);
		tcp_set_seq(header, seq);
		tcp_set_ack_num(header, ack_num);
		tcp_set_data_offset(header, 5);
		tcp_set_window(header, window);
		tcp_set_urg_ptr(header, urg_ptr);
		header->flags = flags;
		header->checksum = 0;

		return (0);
	}

#pragma endregion
