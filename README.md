<div align="center">

![WIP](https://img.shields.io/badge/work%20in%20progress-yellow?style=for-the-badge)
![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Network Communication](https://img.shields.io/badge/Network-Communication-blue?style=for-the-badge)
![ICMP Protocol](https://img.shields.io/badge/Protocol-UDP--TCP--ICMP-green?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*A reimplementation of the traceroute command*

</div>

<div align="center">
  <img src="/images/ft_packet.jpg">
</div>

# ft_traceroute

[README en Español](README_es.md)

`ft_traceroute` is a from-scratch implementation of the `traceroute` command, an essential tool for diagnosing and analyzing network paths. This project explores IP routing behavior and network topology using advanced `TTL` (Time To Live) manipulation techniques.

### What is Traceroute?

Traceroute is a network utility that:

- `Maps the path` packets take from source to destination
- `Identifies intermediate routers` (hops) along the way
- `Measures latency` at each hop
- `Diagnoses routing issues` and failure points
- `Uses decreasing TTL` to reveal network topology

### Technical Flow

```
TTL=1  [Cliente] -----> [Router1] (TTL expired)
         ^                  |
         |<--- ICMP Time Exceeded ---
         
TTL=2  [Cliente] -----> [Router1] -----> [Router2] (TTL expired)
         ^                                   |
         |<--------- ICMP Time Exceeded ----
         
TTL=3  [Cliente] -----> [Router1] -----> [Router2] -----> [Destino]
         ^                                                     |
         |<-------------- ICMP Echo Reply -------------------
```

#### Basic algorithm

1. `Initialization`: Start with TTL = 1
2. `Probe sending`: Send multiple packets (typically 3) with the same TTL
3. `Receive replies`: 
  - ICMP Time Exceeded → Intermediate router identified
  - ICMP Echo Reply → Destination reached
4. `TTL increment`: TTL++ for the next hop
5. `Repeat`: Continue until destination or max TTL

### Probe types

ft_traceroute can use different protocols for probes:

| Protocol | Port     | Detection      |
|----------|----------|----------------|
| `UDP`    | 33434+   | Port closed    |
| `ICMP`   | N/A      | Echo Reply     |
| `TCP`    | Variable | SYN/ACK or RST |

## 🔧 Installation

```bash
git clone https://github.com/Kobayashi82/ft_traceroute.git
cd ft_traceroute
make
```

## 🖥️ Usage

### Permissions

```bash
# ft_traceroute requires root privileges for raw sockets
sudo ./ft_traceroute destino.com

# Alternative: configure capabilities
sudo setcap cap_net_raw+ep ./ft_traceroute
./ft_traceroute destino.com
```

### Execution

```bash
sudo ./ft_traceroute [opciones] <destino> [packetlen]
```

| Argument      | Type          | Description                                   | Example                 |
|---------------|---------------|------------------------------------------------------|------------------|
| `destination` | IPv4/Hostname | IP address or hostname                        | `8.8.8.8`, `google.com` |
| `packetlen`   | Number        | Packet length (default: IP header + 40 bytes) | `60`, `1500`            |

#### Basic
| Option     | Long form   | Description            |
|------------|-------------|------------------------|
| `-h`, `-?` | `--help`    | Shows help information |
| `-V`       | `--version` | Program version        |
|            | `--usage`   | Short usage message    |

#### Route control
| Option | Long form           | Parameter | Description                                                                                           |
|--------|---------------------|-----------|-------------------------------------------------------------------------------------------------------|
| `-m`   | `--max-hops=NUM`    | Number    | Maximum number of hops (default: 30)                                                                  |
| `-f`   | `--first-hop=NUM`   | Number    | Initial TTL (default: 1)                                                                              |
| `-q`   | `--queries=NUM`     | Number    | Number of probes per hop (default: 3)                                                                 |
| `-w`   | `--wait=NUM`        | Seconds   | Wait time per reply (default: 5)                                                                      |
| `-N`   | `--sim-queries=NUM` | Number    | Number of simultaneous probes (default: 16)                                                           |
| `-z`   | `--sendwait=NUM`    | Seconds   | Minimum interval between probes (default: 0). If NUM is greater than 10, it's treated as milliseconds |

#### Probe configuration
| Option | Long form         | Parameter | Description                               |
|--------|-------------------|-----------|-------------------------------------------|
| `-p`   | `--port=NUM`      | Port      | Base port for UDP probes (default: 33434) |
| `-s`   | `--source=ADDR`   | IP        | Source IP address                         |
| `-t`   | `--tos=NUM`       | Number    | Type of Service (TOS)                     |
| `-F`   | `--dont-fragment` | -         | Enables Don't Fragment flag               |
|

#### Probe methods
| Option | Long form | Description                           |
|--------|-----------|---------------------------------------|
| `-I`   | `--icmp`  | Uses ICMP Echo Request instead of UDP |
| `-T`   | `--tcp`   | Uses TCP SYN probes                   |
| `-U`   | `--udp`   | Uses UDP (default behavior)           |

#### Network options
| Option | Long form            | Parameter  | Description                          |
|--------|----------------------|------------|--------------------------------------|
| `-n`   | `--numeric`          | -          | Do not resolve IP addresses to names |
| `-d`   | `--debug`            | -          | Enables socket-level debugging       |
| `-i`   | `--interface=DEVICE` | Device     | Specifies network interface to use   |
| `-r`   | -                    | -          | Bypass normal routing, send directly |

#### TOS values (Type of Service)

The `-t` option lets you configure the TOS field of the IP header:

| Value | Type                 | Description          |
|-------|----------------------|----------------------|
| `16`  | Low Delay            | Low latency          |
| `4`   | High Reliability     | High reliability     |
| `8`   | High Throughput      | High throughput      |
| `136` | High Priority        | High priority        |
| `184` | Expedited Forwarding | Expedited forwarding |

## 📡 Internal behavior

### TTL manipulation

The TTL (Time To Live) field in the IP header is fundamental:

```c
struct ip_header {
    uint8_t  version_ihl;     // Version (4 bits) + IHL (4 bits)
    uint8_t  tos;             // Type of Service
    uint16_t total_length;    // Total packet length
    uint16_t identification;  // Fragmentation ID
    uint16_t flags_fragment;  // Flags (3 bits) + Fragment offset (13 bits)
    uint8_t  ttl;             // Time To Live ← Key field
    uint8_t  protocol;        // Protocol (UDP=17, ICMP=1, TCP=6)
    uint16_t checksum;        // Header checksum
    uint32_t source_addr;     // Source IP address
    uint32_t dest_addr;       // Destination IP address
};
```

### Response processing

#### Time Exceeded (Type 11)
```c
struct icmp_time_exceeded {
    uint8_t  type;            // 11 (Time Exceeded)
    uint8_t  code;            // 0 (TTL exceeded in transit)
    uint16_t checksum;        // ICMP checksum
    uint32_t unused;          // Reserved
    // Original IP header + 8 bytes of original data
    struct ip_header original_ip;
    uint8_t original_data[8];
};
```

#### Destination Unreachable (Type 3)
| Code | Description            | Meaning                           |
|------|------------------------|-----------------------------------|
| `0`  | Network Unreachable    | Network unreachable               |
| `1`  | Host Unreachable       | Host unreachable                  |
| `2`  | Protocol Unreachable   | Protocol not supported            |
| `3`  | Port Unreachable       | Port closed (UDP traceroute)      |
| `4`  | Fragmentation Required | Fragmentation required but DF set |

### Completion detection

The trace ends when:

1. `Echo Reply received` (for ICMP traceroute)
2. `Port Unreachable` (for UDP traceroute)
3. `TCP SYN/ACK` or `RST` (for TCP traceroute)
4. `Max TTL reached` (timeout or limit)

## 🗺️ Interpreting Results

### Standard output format

```
traceroute to google.com (142.250.185.14), 30 hops max, 60 byte packets
 1  gateway (192.168.1.1)  1.234 ms  1.567 ms  1.890 ms
 2  10.0.0.1 (10.0.0.1)  15.234 ms  14.567 ms  16.890 ms
 3  * * *
 4  72.14.194.226 (72.14.194.226)  45.123 ms  44.567 ms  43.890 ms
```

### Symbol interpretation

| Symbol | Meaning                  | Likely cause                     |
|--------|--------------------------|----------------------------------|
| `*`    | No reply                 | Firewall, silent router, timeout |
| `!H`   | Host Unreachable         | Destination unreachable          |
| `!N`   | Network Unreachable      | Network missing or not routed    |
| `!P`   | Protocol Unreachable     | Protocol blocked                 |
| `!X`   | Communication Prohibited | Administrative filtering         |

### Latency analysis

```bash
# Normal latencies
 5  router.normal.com  25.123 ms  24.567 ms  26.890 ms

# Inconsistent latencies
 5  router.congestionado.com  125.123 ms  45.567 ms  186.890 ms

# Packet loss
 5  router.perdidas.com  35.123 ms  *  37.890 ms
```
## ⚠️ Limitations and Considerations

### Router behavior

- `Load balancing`: Routes can change between packets
- `ICMP rate limiting`: Some routers limit ICMP replies
- `Selective filtering`: Firewalls may block certain TTLs
- `Asymmetric replies`: Router A may respond for Router B

### Measurement accuracy

- `Network variability`: Latencies can fluctuate significantly
- `ICMP processing`: Low priority on many routers
- `ARP cache`: Initial measurements may be inaccurate
- `QoS`: Type of Service can affect packet handling

### Security considerations

⚠️ **Responsible use:**
- `Respect organizational` network policies
- `Avoid unauthorized` reconnaissance
- `Consider rate limiting` to avoid being detected as an attack

### Detection and countermeasures

Some systems can detect:
- `Port scan patterns` (with TCP traceroute)
- `Repeated probing` (multiple consecutive traces)
- `Anomalous TTL patterns` (non-sequential hops)
- `High-frequency probing` (very short intervals)

## 📄 License

This project is licensed under the WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🧭 Developed as part of the 42 School curriculum 🧭**

*"From source to destination, every step uncovered"*

</div>
