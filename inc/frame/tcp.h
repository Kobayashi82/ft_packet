/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tcp.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:30 by vzurera-          #+#    #+#             */
/*   Updated: 2026/02/25 12:06:19 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#pragma region "Includes"

	#include <netinet/in.h>

	// Flags
	#define FIN				0x01					// Finish: no more data from sender
	#define SYN				0x02					// Synchronize: initiate connection
	#define RST				0x04					// Reset: abort connection
	#define PSH				0x08					// Push: deliver data immediately
	#define ACK				0x10					// Acknowledgement: ack_num is valid
	#define URG				0x20					// Urgent: urg_ptr is valid
	#define ECE				0x40					// ECN-Echo: congestion notification
	#define CWR				0x80					// Congestion Window Reduced

	// Options
	#define EOL             0x00    				// End of Option List        (RFC 793)
	#define NOP             0x01    				// No Operation              (RFC 793)
	#define MSS             0x02    				// Maximum Segment Size      (RFC 793)
	#define WSOPT           0x03    				// Window Scale              (RFC 7323)
	#define SACK_PERMITTED  0x04    				// SACK Permitted            (RFC 2018)
	#define SACK            0x05    				// Selective ACK             (RFC 2018)
	#define TIMESTAMPS      0x08    				// Timestamps                (RFC 7323)

#pragma endregion

#pragma region "Structures"

	typedef struct __attribute__((__packed__)) {
		uint16_t	src_port;						// Source port
		uint16_t	dst_port;						// Destination port
		uint32_t	seq;							// Sequence number
		uint32_t	ack_num;						// Acknowledgement number (valid when ACK flag is set)
		uint8_t		data_offset;					// High nibble: header length in 32-bit words (min 5, max 15). Low nibble: reserved (must be 0)
		uint8_t		flags;							// Control bits: CWR ECE URG ACK PSH RST SYN FIN
		uint16_t	window;							// Receive window size (flow control)
		uint16_t	checksum;						// Checksum over pseudo-header + header + options + payload
		uint16_t	urg_ptr;						// Urgent pointer: offset to last urgent byte (only when URG set)
	}	t_tcp;

	typedef struct __attribute__((__packed__)) {
		uint8_t		options[40];					// Raw option bytes, padded to a multiple of 4 bytes
		uint8_t		length;							// Actual used bytes (must be a multiple of 4)
	}	t_tcp_option;

#pragma endregion

#pragma region "Methods"

	int	tcp_option_set_eol(t_tcp_option *opt);
	int	tcp_option_set_nop(t_tcp_option *opt);
	int	tcp_option_set_mss(t_tcp_option *opt, uint16_t mss);
	int	tcp_option_set_window_scale(t_tcp_option *opt, uint8_t shift);
	int	tcp_option_set_sack_permitted(t_tcp_option *opt);
	int	tcp_option_set_sack(t_tcp_option *opt, uint8_t num_blocks, const uint32_t *blocks);
	int	tcp_option_set_timestamps(t_tcp_option *opt, uint32_t tsval, uint32_t tsecr);

	int	tcp_set_src_port(t_tcp *header, uint16_t src_port);
	int	tcp_set_dst_port(t_tcp *header, uint16_t dst_port);
	int	tcp_set_seq(t_tcp *header, uint32_t seq);
	int	tcp_set_ack_num(t_tcp *header, uint32_t ack_num);
	int	tcp_set_data_offset(t_tcp *header, uint8_t data_offset);
	int	tcp_set_flag_fin(t_tcp *header, uint8_t value);
	int	tcp_set_flag_syn(t_tcp *header, uint8_t value);
	int	tcp_set_flag_rst(t_tcp *header, uint8_t value);
	int	tcp_set_flag_psh(t_tcp *header, uint8_t value);
	int	tcp_set_flag_ack(t_tcp *header, uint8_t value);
	int	tcp_set_flag_urg(t_tcp *header, uint8_t value);
	int	tcp_set_flag_ece(t_tcp *header, uint8_t value);
	int	tcp_set_flag_cwr(t_tcp *header, uint8_t value);
	int	tcp_set_window(t_tcp *header, uint16_t window);
	int	tcp_set_urg_ptr(t_tcp *header, uint16_t urg_ptr);
	int tcp_set_checksum(t_tcp *header, uint32_t src_addr, uint32_t dst_addr, uint16_t options_len, uint16_t data_len, const void *data);

	int	tcp_create(t_tcp *header, uint16_t src_port, uint16_t dst_port, uint32_t seq, uint32_t ack_num, uint8_t flags, uint16_t window, uint16_t urg_ptr);

#pragma endregion
