# ICMP

## Que es ICMP

ICMP (`Internet Control Message Protocol`) se usa para mensajes de control y diagnostico sobre IPv4. En esta libreria se monta encima de una cabecera IP y opcionalmente con payload.

## Estructura basica

```text
[ type ][ code ][ checksum ][ union data ]
   1       1         2            4
```

Tamano base: 8 bytes.

## Campos

| Campo | Descripcion |
|-------|-------------|
| `type` | Tipo de mensaje ICMP |
| `code` | Subtipo dentro del tipo |
| `checksum` | Checksum sobre cabecera + payload |
| `un.echo.id` | Identificador para echo/timestamp |
| `un.echo.sequence` | Secuencia para echo/timestamp |
| `un.gateway` | Gateway en Redirect |
| `un.frag.mtu` | MTU en Fragmentation Needed |

## Tipos soportados en la libreria

| Tipo | Nombre |
|------|--------|
| `0` | Echo Reply |
| `3` | Destination Unreachable |
| `5` | Redirect |
| `8` | Echo Request |
| `11` | Time Exceeded |
| `13` | Timestamp Request |
| `14` | Timestamp Reply |

## API disponible en `ft_packet`

```c
int icmp_set_type(t_icmp *header, uint8_t type);
int icmp_set_code(t_icmp *header, uint8_t code);
int icmp_set_checksum(t_icmp *header, uint16_t data_len, const void *data);
int icmp_set_id(t_icmp *header, uint16_t id);
int icmp_set_seq(t_icmp *header, uint16_t seq);
int icmp_inc_seq(t_icmp *header);
int icmp_set_gateway(t_icmp *header, uint32_t gateway);
int icmp_set_mtu(t_icmp *header, uint16_t mtu);

int icmp_create_echo(t_icmp *header, uint16_t id, uint16_t seq);
int icmp_create_echo_reply(t_icmp *header, uint16_t id, uint16_t seq);
int icmp_create_dest_unreachable(t_icmp *header, uint8_t code);
int icmp_create_redirect(t_icmp *header, uint8_t code, uint32_t gateway);
int icmp_create_time_exceeded(t_icmp *header, uint8_t code);
int icmp_create_timestamp(t_icmp *header, uint16_t id, uint16_t seq);
int icmp_create_timestamp_reply(t_icmp *header, uint16_t id, uint16_t seq);
```

## Uso tipico

```c
t_ip ip = {0};
t_icmp icmp = {0};
char payload[] = "hello";

ip_create(&ip, 0, 0, 0, 0x1234, 1, 0, 0, 64,
	IPPROTO_ICMP, inet_addr("192.168.1.10"), inet_addr("8.8.8.8"));
icmp_create_echo(&icmp, 0x42, 1);

packet_add(&packet, &ip, sizeof(ip), IP);
packet_add(&packet, &icmp, sizeof(icmp), ICMP);
packet_add(&packet, payload, sizeof(payload) - 1, PAYLOAD);
icmp_complete(&packet);
```

## Que hace `icmp_complete`

- Calcula `ip.length` si hay cabecera IP.
- Recalcula el checksum IP.
- Recalcula el checksum ICMP sobre cabecera + payload.

## Consideraciones

- ICMP no tiene campo de longitud propio; depende del `total length` de IP.
- Si hay payload, el checksum lo cubre entero.
- `Echo Request` y `Echo Reply` son las pruebas mas practicas para validar la ruta ICMP de la libreria.
