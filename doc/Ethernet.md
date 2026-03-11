# Ethernet

## Estructura basica

```text
Ethernet II:
[ dst_mac ][ src_mac ][ ethertype ][ payload ]
	6           6           2          ...
```

La cabecera Ethernet que usa la libreria ocupa 14 bytes y corresponde al formato Ethernet II clasico, sin VLAN.

## Campos

| Campo | Tamano | Descripcion |
|-------|--------|-------------|
| `dst_mac` | 6 bytes | MAC destino |
| `src_mac` | 6 bytes | MAC origen |
| `ethertype` | 2 bytes | Protocolo superior (`ETH_P_IP`, `ETH_P_ARP`, etc.) |

## EtherType comunes

| Valor | Nombre |
|-------|--------|
| `0x0800` | IPv4 |
| `0x0806` | ARP |
| `0x86DD` | IPv6 |

## API disponible en `ft_packet`

```c
int ethernet_set_dst_mac(t_ethernet *header, const uint8_t *dst_mac);
int ethernet_set_src_mac(t_ethernet *header, const uint8_t *src_mac);
int ethernet_set_ethertype(t_ethernet *header, uint16_t ethertype);
int ethernet_create(t_ethernet *header, const uint8_t *dst_mac,
	const uint8_t *src_mac, uint16_t ethertype);
```

## Uso tipico

```c
t_ethernet eth = {0};
uint8_t dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t src[6] = {0x02, 0x42, 0xac, 0x11, 0x00, 0x02};

ethernet_create(&eth, dst, src, ETH_P_ARP);
packet_add(&packet, &eth, sizeof(eth), ETHERNET);
```

## Notas practicas

- Ethernet no lleva checksum en esta libreria. El FCS lo calcula la NIC al transmitir.
- Si construyes `Ethernet + IP`, el `ethertype` debe ser `ETH_P_IP`.
- Si construyes `Ethernet + ARP`, el `ethertype` debe ser `ETH_P_ARP`.
- En un ARP request, `ethernet.dst_mac` suele ser broadcast.
- En loopback no hay una trama Ethernet real, asi que las pruebas con raw IP no ejercitan esta cabecera.

## Relacion con `t_packet`

`packet_add(..., ETHERNET)` debe ser la primera capa del paquete. Despues de Ethernet, la libreria permite:

- `ARP`
- `IP`

No es valida una composicion con `Ethernet` despues de haber anadido otra cabecera.
