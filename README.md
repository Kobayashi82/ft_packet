<div align="center">

![WIP](https://img.shields.io/badge/work%20in%20progress-yellow?style=for-the-badge)
![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Network Packets](https://img.shields.io/badge/Network-Packets-blue?style=for-the-badge)
![Protocols](https://img.shields.io/badge/Protocol-Ethernet--ARP--IPv4--ICMP--UDP--TCP-green?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*A static C library for building network packets*

</div>

<div align="center">
  <img src="/images/ft_packet.jpg">
</div>

# ft_packet

[README en Español](README_es.md)

`ft_packet` is a low-level C library for constructing network packets from scratch. It provides public headers and helpers to assemble `Ethernet`, `ARP`, `IPv4`, `ICMP`, `UDP`, and `TCP` frames, including optional IP and TCP options, payload handling, and checksum completion.

### What is ft_packet?

This project is focused on packet composition rather than packet capture or transmission. Its main goals are:

- `Build raw frames` in a predictable order
- `Set protocol fields` through dedicated helpers
- `Attach payloads and options` while keeping header layout coherent
- `Complete lengths and checksums` before transmission or inspection
- `Expose a small public API` suitable for networking experiments and tools

### Supported stack

`ft_packet` can assemble the following layers:

| Layer      | Support |
|------------|---------|
| `Ethernet` | Frame header creation |
| `ARP`      | Request and reply creation |
| `IPv4`     | Header fields and options |
| `ICMP`     | Echo, reply, redirect, unreachable, timestamp |
| `UDP`      | Header creation and checksum |
| `TCP`      | Flags, checksum, and options |
| `Payload`  | Raw byte payload attachment |

## 🔧 Installation

```bash
git clone https://github.com/Kobayashi82/ft_packet.git
cd ft_packet
make
```

The build generates a static library:

```bash
lib/libft_packet.a
```

## 🖥️ Usage

### Build integration

Include the public headers and link against the generated archive:

```bash
cc example.c -Iinc -Llib -lft_packet
```

### Basic workflow

The intended flow is:

1. `Create headers` with the module helpers such as `ethernet_create`, `ip_create`, `icmp_create_echo`, `udp_create`, or `tcp_create`
2. `Append each part` to a `t_packet` with `packet_add`
3. `Append payload` if the transport protocol allows it
4. `Finalize the packet` with `icmp_complete`, `udp_complete`, or `tcp_complete`
5. `Reuse or reset` the packet with `packet_clear`

### Public API overview

| Module | Main capabilities |
|--------|-------------------|
| `packet.h` | `packet_add`, `packet_clear`, transport completion helpers |
| `frame/ethernet.h` | Ethernet header setters and `ethernet_create` |
| `frame/arp.h` | ARP request and reply helpers |
| `frame/ip.h` | IPv4 setters, checksum, and IP option builders |
| `frame/icmp.h` | ICMP message creation and checksum helpers |
| `frame/udp.h` | UDP header creation and pseudo-header checksum |
| `frame/tcp.h` | TCP flags, checksum, and TCP option helpers |

### Minimal example

```c
#include "packet.h"

int main(void)
{
    t_packet packet = {0};
    t_ip ip = {0};
    t_icmp icmp = {0};
    char payload[] = "hello";

    ip_create(&ip, 0, 0, 0, 0x1234, 0, 0, 0, 64, IPPROTO_ICMP, 0, 0);
    icmp_create_echo(&icmp, 0x42, 1);
    packet_add(&packet, &ip, sizeof(ip), IP);
    packet_add(&packet, &icmp, sizeof(icmp), ICMP);
    packet_add(&packet, payload, sizeof(payload) - 1, PAYLOAD);
    icmp_complete(&packet);
    packet_clear(&packet);
    return (0);
}
```

## 📡 Internal behavior

### Assembly rules

`ft_packet` stores the final bytes inside a fixed packet buffer (`MAX_PACKET_LEN = 2048`) and keeps typed pointers to the headers already inserted. This allows the library to validate ordering constraints while the packet is being built.

Typical valid compositions are:

```text
Ethernet + ARP
Ethernet + IP + ICMP + Payload
Ethernet + IP + UDP + Payload
Ethernet + IP + TCP + TCP options + Payload
IP + ICMP/UDP/TCP + Payload
```

### Completion logic

The completion helpers update protocol-dependent fields before the packet is sent or analyzed:

| Helper | Finalizes |
|--------|-----------|
| `icmp_complete` | ICMP checksum, IP length/checksum when applicable |
| `udp_complete`  | UDP length, UDP checksum, IP length/checksum |
| `tcp_complete`  | TCP data offset, TCP checksum, IP length/checksum |

### Checksum coverage

The library follows the standard checksum model for each protocol:

| Header   | Length field | Checksum scope |
|----------|--------------|----------------|
| `Ethernet` | No | FCS handled by NIC |
| `ARP`      | No | No checksum |
| `IP`       | Header + payload | IP header only |
| `ICMP`     | No | Header + payload |
| `UDP`      | Header + payload | Pseudo-header + header + payload |
| `TCP`      | Header + payload | Pseudo-header + header + payload |

## 🗺️ Project documentation

The repository includes protocol-specific notes and a manual test plan:

| File | Content |
|------|---------|
| `doc/Ethernet.md` | Ethernet frame notes |
| `doc/ARP.md`      | ARP reference |
| `doc/IPv4.md`     | IPv4 header and options |
| `doc/ICMP.md`     | ICMP message types |
| `doc/UDP.md`      | UDP behavior and checksum |
| `doc/TCP.md`      | TCP flags and options |
| `doc/test_plan.md`| Suggested validation scenarios |

### Suggested validation

The current test plan is oriented around packet inspection with tools such as:

- `Wireshark` or `tcpdump` to validate header layout and checksums
- `netcat` for UDP payload tests
- A second host or service for TCP handshake validation
- `root` privileges when raw socket tests are involved

## ⚠️ Limitations and Considerations

### Current scope

- `Static library only`: the default build does not generate an executable
- `Packet construction focus`: transmission and socket orchestration are outside this repository
- `IPv4 only`: the public API currently targets Ethernet, ARP, and IPv4-based transports
- `Fixed buffer size`: packets are limited by `MAX_PACKET_LEN`

### Practical considerations

- `Byte order matters`: callers must stay consistent with host/network order expectations
- `Raw networking tests`: some verification scenarios require elevated privileges
- `Protocol ordering`: invalid layer combinations are rejected by `packet_add`
- `Option alignment`: IP and TCP options must remain aligned to 4-byte boundaries

### Intended use

This library is suitable for:

- `Learning` how network headers are laid out in memory
- `Crafting packets` for raw socket experiments
- `Testing` checksum and option handling
- `Building tooling` on top of a small packet-construction API

## 📄 License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🧭 Developed as part of the 42 School curriculum 🧭**

*"Build the packet, byte by byte"*

</div>
