/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arp.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/01 19:22:05 by vzurera-          #+#    #+#             */
/*   Updated: 2026/03/11 22:10:46 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*                                    ARP Header (28 bytes)                                  
    ┌─────────────────────┬─────────────────────┬─────────────────────┬─────────────────────┐
    │          0          │          1          │          2          │          3          │
    ├─────────────────────┴─────────────────────┼─────────────────────┴─────────────────────┤
    │             Hardware Type (1)             │          Protocol Type (0x0800)           │
    ├─────────────────────┬─────────────────────┼───────────────────────────────────────────┤
    │     Hwd Len (6)     │    Proto Len (4)    │                 Operation                 │
    ├─────────────────────┴─────────────────────┴───────────────────────────────────────────┤
    │                                Sender Hardware Address                                │
    │                                           ┌───────────────────────────────────────────┤
    │                                           │          Sender Protocol Address          │
    ├───────────────────────────────────────────┼───────────────────────────────────────────┤
    │      Sender Protocol Address (cont.)      │          Target Hardware Address          │
    ├───────────────────────────────────────────┘                                           │
    │                                                                                       │
    ├───────────────────────────────────────────────────────────────────────────────────────┤
    │                                Target Protocol Address                                │
    └───────────────────────────────────────────────────────────────────────────────────────┘
*/

#pragma region "Includes"

	#include "frame/arp.h"
	#include <string.h>

#pragma endregion

#pragma region "Set"

	#pragma region "Hardware Type"

		int arp_set_htype(t_arp *header, uint16_t htype) {
			if (!header) return (1);

			header->htype = htons(htype);

			return (0);
		}

	#pragma endregion

	#pragma region "Protocol Type"

		int arp_set_ptype(t_arp *header, uint16_t ptype) {
			if (!header) return (1);

			header->ptype = htons(ptype);

			return (0);
		}

	#pragma endregion

	#pragma region "Hardware Length"

		int arp_set_hlen(t_arp *header, uint8_t hlen) {
			if (!header) return (1);

			header->hlen = hlen;

			return (0);
		}

	#pragma endregion

	#pragma region "Protocol Length"

		int arp_set_plen(t_arp *header, uint8_t plen) {
			if (!header) return (1);

			header->plen = plen;

			return (0);
		}

	#pragma endregion

	#pragma region "Operation"

		int arp_set_oper(t_arp *header, uint16_t oper) {
			if (!header) return (1);

			header->oper = htons(oper);

			return (0);
		}

	#pragma endregion

	#pragma region "Source Hardware Address (MAC)"

		int arp_set_sha(t_arp *header, const uint8_t *sha) {
			if (!header || !sha) return (1);

			memcpy(header->sha, sha, 6);

			return (0);
		}

	#pragma endregion

	#pragma region "Sender Protocol Address (IP)"

		int arp_set_spa(t_arp *header, uint32_t spa) {
			if (!header) return (1);

			header->spa = spa;

			return (0);
		}

	#pragma endregion

	#pragma region "Target Hardware Address (MAC)"

		int arp_set_tha(t_arp *header, const uint8_t *tha) {
			if (!header || !tha) return (1);

			memcpy(header->tha, tha, 6);

			return (0);
		}

	#pragma endregion

	#pragma region "Target Protocol Address (IP)"

		int arp_set_tpa(t_arp *header, uint32_t tpa) {
			if (!header) return (1);

			header->tpa = tpa;

			return (0);
		}

	#pragma endregion

#pragma endregion

#pragma region "Create"

	static int arp_create(t_arp *header, uint16_t oper, const uint8_t *sha, uint32_t spa, const uint8_t *tha, uint32_t tpa) {
		if (!header || !sha || !tha) return (1);

		header->htype = htons(1);           // Ethernet = 1
		header->ptype = htons(ETH_P_IP);    // IPv4
		header->hlen  = 6;                  // MAC length = 6
		header->plen  = 4;                  // IP length = 4
		header->oper  = htons(oper);        // Operation (Request = 1, Reply = 2)

		memcpy(header->sha, sha, 6);    	 // Sender MAC
		header->spa = spa;                  // Sender IP

		memcpy(header->tha, tha, 6);     	// Target MAC
		header->tpa = tpa;                  // Target IP

		return (0);
	}

	int arp_create_reply(t_arp *header, const uint8_t *sha, uint32_t spa, const uint8_t *tha, uint32_t tpa) {
		return (arp_create(header, ARPOP_REPLY, sha, spa, tha, tpa));
	}

	int arp_create_request(t_arp *header, const uint8_t *sha, uint32_t spa, uint32_t tpa) {
		uint8_t target[6] = {0, 0, 0, 0, 0, 0};

		return (arp_create(header, ARPOP_REQUEST, sha, spa, target, tpa));
	}

#pragma endregion
