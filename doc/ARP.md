# ARP

## Que es ARP

ARP (`Address Resolution Protocol`) resuelve una direccion IPv4 a una direccion MAC dentro de la red local. Se usa antes de poder enviar trafico Ethernet a un host del mismo segmento.

## Estructura de la cabecera

```text
[ htype ][ ptype ][ hlen ][ plen ][ oper ][ sha ][ spa ][ tha ][ tpa ]
	2        2       1       1       2      6      4      6      4
```

Tamano total: 28 bytes.

## Campos

| Campo | Descripcion |
|-------|-------------|
| `htype` | Tipo de hardware. En Ethernet es `1`. |
| `ptype` | Tipo de protocolo. Para IPv4 es `0x0800`. |
| `hlen` | Longitud de direccion hardware. En MAC es `6`. |
| `plen` | Longitud de direccion de protocolo. En IPv4 es `4`. |
| `oper` | Operacion: request (`1`) o reply (`2`). |
| `sha` | Sender Hardware Address. MAC del emisor. |
| `spa` | Sender Protocol Address. IP del emisor. |
| `tha` | Target Hardware Address. MAC del destino. |
| `tpa` | Target Protocol Address. IP del destino. |

## API disponible en `ft_packet`

```c
int arp_set_htype(t_arp *header, uint16_t htype);
int arp_set_ptype(t_arp *header, uint16_t ptype);
int arp_set_hlen(t_arp *header, uint8_t hlen);
int arp_set_plen(t_arp *header, uint8_t plen);
int arp_set_oper(t_arp *header, uint16_t oper);
int arp_set_sha(t_arp *header, const uint8_t *sha);
int arp_set_spa(t_arp *header, uint32_t spa);
int arp_set_tha(t_arp *header, const uint8_t *tha);
int arp_set_tpa(t_arp *header, uint32_t tpa);

int arp_create_request(t_arp *header, const uint8_t *sha,
	uint32_t spa, uint32_t tpa);
int arp_create_reply(t_arp *header, const uint8_t *sha,
	uint32_t spa, const uint8_t *tha, uint32_t tpa);
```

## Request y reply

### ARP Request

- `oper = 1`
- `ethernet.dst_mac = ff:ff:ff:ff:ff:ff`
- `arp.sha = MAC propia`
- `arp.spa = IP propia`
- `arp.tha = 00:00:00:00:00:00`
- `arp.tpa = IP que quieres resolver`

### ARP Reply

- `oper = 2`
- `ethernet.dst_mac = MAC del solicitante`
- `arp.sha = MAC propia`
- `arp.spa = IP propia`
- `arp.tha = MAC del solicitante`
- `arp.tpa = IP del solicitante`

## Ejemplo de request

```c
t_ethernet eth = {0};
t_arp arp = {0};
uint8_t src_mac[6] = {0x02, 0x42, 0xac, 0x11, 0x00, 0x02};
uint8_t broadcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

ethernet_create(&eth, broadcast, src_mac, ETH_P_ARP);
arp_create_request(&arp, src_mac,
	inet_addr("192.168.1.10"), inet_addr("192.168.1.1"));

packet_add(&packet, &eth, sizeof(eth), ETHERNET);
packet_add(&packet, &arp, sizeof(arp), ARP);
```

## Consideraciones

- ARP no tiene checksum.
- No existe `arp_complete` porque no hay longitudes ni checksums derivados que recalcular al final.
- La coherencia importante en este proyecto esta entre Ethernet y ARP: `ethertype`, MAC origen y MAC destino.
- ARP solo tiene sentido sobre Ethernet en esta libreria.
