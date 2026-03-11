<div align="center">

![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Network Packets](https://img.shields.io/badge/Network-Packets-blue?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*A static C library for building network packets*

</div>

<div align="center">
  <img src="/images/ft_packet.jpg">
</div>

# ft_packet

[README en Español](README_es.md)

`ft_packet` is a low-level C library for constructing network packets from scratch. It provides public headers and helpers to assemble `Ethernet`, `ARP`, `IPv4`, `ICMP`, `UDP`, and `TCP` frames, including optional IP and TCP options, payload handling, and checksum completion.

### Supported stack

`ft_packet` can assemble the following layers:

| Layer      | Support                                       |
|------------|-----------------------------------------------|
| `Ethernet` | Frame header creation                         |
| `ARP`      | Request and reply creation                    |
| `IPv4`     | Header fields and options                     |
| `ICMP`     | Echo, reply, redirect, unreachable, timestamp |
| `UDP`      | Header creation and checksum                  |
| `TCP`      | Flags, checksum, and options                  |
| `Payload`  | Raw byte payload attachment                   |

## 🔧 Installation

```bash
git clone https://github.com/Kobayashi82/ft_packet.git
cd ft_packet
make
```

The build generates the static library `lib/libft_packet.a`

## 🖥️ Usage

### Integration

Link against the generated library:

```bash
cc example.c -Iinc -Llib -lft_packet
```

### Basic workflow

The intended flow is:

1. `Create headers` with `ethernet_create`, `ip_create`, `icmp_create_echo`, `udp_create`, or `tcp_create`
2. `Append each part` to a `t_packet` with `packet_add`
3. `Append payload` if the transport protocol allows it
4. `Finalize the packet` with `icmp_complete`, `udp_complete`, or `tcp_complete`
5. `Reuse or reset` the packet with `packet_clear`

### Example

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
Ethernet + IP + UDP  + Payload
Ethernet + IP + TCP  + TCP options + Payload
IP + ICMP/UDP/TCP + Payload
```

### Completion logic

The completion functions update protocol-dependent fields before the packet is sent or analyzed:

| Function        | Finalizes                                         |
|-----------------|---------------------------------------------------|
| `icmp_complete` | ICMP checksum, IP length/checksum when applicable |
| `udp_complete`  | UDP length, UDP checksum, IP length/checksum      |
| `tcp_complete`  | TCP data offset, TCP checksum, IP length/checksum |

### Checksum coverage

The library follows the standard checksum model for each protocol:

| Header     | Length field     | Checksum scope                   |
|----------  |------------------|----------------------------------|
| `Ethernet` | No               | FCS handled by NIC               |
| `ARP`      | No               | No checksum                      |
| `IP`       | Header + payload | IP header only                   |
| `ICMP`     | No               | Header + payload                 |
| `UDP`      | Header + payload | Pseudo-header + header + payload |
| `TCP`      | Header + payload | Pseudo-header + header + payload |

## 📄 License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🧭 Developed as part of the 42 School curriculum 🧭**

*"Building packets the hard way"*

</div>
