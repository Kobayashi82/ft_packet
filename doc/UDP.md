# UDP

## Que es UDP

UDP (`User Datagram Protocol`) es un protocolo de transporte sin conexion, con cabecera minima y sin control de estado. En esta libreria se monta sobre IPv4 y puede llevar payload arbitrario.

## Estructura de la cabecera

```text
[ src_port ][ dst_port ][ length ][ checksum ]
	2           2          2          2
```

Tamano fijo: 8 bytes.

## Campos

| Campo | Descripcion |
|-------|-------------|
| `src_port` | Puerto origen |
| `dst_port` | Puerto destino |
| `length` | Cabecera UDP + payload |
| `checksum` | Pseudo-header IP + UDP + payload |

## API disponible en `ft_packet`

```c
int udp_set_src_port(t_udp *header, uint16_t src_port);
int udp_set_dst_port(t_udp *header, uint16_t dst_port);
int udp_set_length(t_udp *header, uint16_t data_len);
int udp_set_checksum(t_udp *header, uint32_t src_addr,
    uint32_t dst_addr, uint16_t data_len, const void *data);

int udp_create(t_udp *header, uint16_t src_port,
    uint16_t dst_port, uint16_t data_len);
int udp_create_checksum(t_udp *header, uint16_t src_port,
    uint16_t dst_port, uint32_t src_addr, uint32_t dst_addr,
    uint16_t data_len, const void *data);
```

## Uso tipico

```c
t_ip ip = {0};
t_udp udp = {0};
char payload[] = "udp message";
uint32_t src = inet_addr("10.0.0.1");
uint32_t dst = inet_addr("10.0.0.2");

ip_create(&ip, 0, 0, 0, 0x2222, 1, 0, 0, 64, IPPROTO_UDP, src, dst);
udp_create(&udp, 12345, 8080, 0);

packet_add(&packet, &ip, sizeof(ip), IP);
packet_add(&packet, &udp, sizeof(udp), UDP);
packet_add(&packet, payload, sizeof(payload) - 1, PAYLOAD);
udp_complete(&packet, src, dst);
```

## Que hace `udp_complete`

- Ajusta `udp.length` a `8 + payload_len`.
- Ajusta `ip.length`.
- Recalcula checksum IP.
- Recalcula checksum UDP usando pseudo-header IPv4.

## Notas practicas

- El checksum UDP depende de `src_addr` y `dst_addr`.
- Si cambias IP origen o destino despues de completar el paquete, debes recalcular.
- `udp_create_checksum` sirve si ya tienes el payload y quieres calcular el checksum en el momento de crear la cabecera.
