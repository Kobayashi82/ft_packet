# TCP

## Que es TCP

TCP (`Transmission Control Protocol`) es un protocolo orientado a conexion. Su cabecera es mas compleja que UDP porque incluye secuencia, acuse de recibo, flags, ventana y opciones.

## Estructura de la cabecera base

```text
[ src_port ][ dst_port ][ seq ][ ack_num ]
[ data_offset ][ flags ][ window ][ checksum ][ urg_ptr ]
```

Tamano minimo: 20 bytes.
Tamano maximo con opciones: 60 bytes.

## Campos principales

| Campo | Descripcion |
|-------|-------------|
| `src_port` | Puerto origen |
| `dst_port` | Puerto destino |
| `seq` | Numero de secuencia |
| `ack_num` | Numero de acuse |
| `data_offset` | Tamano de cabecera en palabras de 32 bits |
| `flags` | Bits de control (`SYN`, `ACK`, `FIN`, etc.) |
| `window` | Ventana de recepcion |
| `checksum` | Pseudo-header IP + TCP + opciones + payload |
| `urg_ptr` | Puntero urgente |

## Flags soportados

| Flag | Valor |
|------|-------|
| `FIN` | `0x01` |
| `SYN` | `0x02` |
| `RST` | `0x04` |
| `PSH` | `0x08` |
| `ACK` | `0x10` |
| `URG` | `0x20` |
| `ECE` | `0x40` |
| `CWR` | `0x80` |

## Opciones soportadas

| Opcion | Kind |
|--------|------|
| `EOL` | `0` |
| `NOP` | `1` |
| `MSS` | `2` |
| `Window Scale` | `3` |
| `SACK Permitted` | `4` |
| `SACK` | `5` |
| `Timestamps` | `8` |

## API disponible en `ft_packet`

```c
int tcp_create(t_tcp *header, uint16_t src_port, uint16_t dst_port,
	uint32_t seq, uint32_t ack_num, uint8_t flags,
	uint16_t window, uint16_t urg_ptr);

int tcp_option_set_mss(t_tcp_option *opt, uint16_t mss);
int tcp_option_set_window_scale(t_tcp_option *opt, uint8_t shift);
int tcp_option_set_sack_permitted(t_tcp_option *opt);
int tcp_option_set_sack(t_tcp_option *opt, uint8_t num_blocks,
	const uint32_t *blocks);
int tcp_option_set_timestamps(t_tcp_option *opt,
	uint32_t tsval, uint32_t tsecr);
```

## Uso tipico

```c
t_ip ip = {0};
t_tcp tcp = {0};
t_tcp_option opt = {0};
char payload[] = "tcp data";
uint32_t src = inet_addr("10.0.0.1");
uint32_t dst = inet_addr("10.0.0.2");

ip_create(&ip, 0, 0, 0, 0x3333, 1, 0, 0, 64, IPPROTO_TCP, src, dst);
tcp_create(&tcp, 40000, 443, 1, 0, SYN, 65535, 0);
tcp_option_set_mss(&opt, 1460);
tcp_option_set_nop(&opt);
tcp_option_set_window_scale(&opt, 7);

packet_add(&packet, &ip, sizeof(ip), IP);
packet_add(&packet, &tcp, sizeof(tcp), TCP);
packet_add(&packet, &opt, opt.length, TCP_OPTION);
packet_add(&packet, payload, sizeof(payload) - 1, PAYLOAD);
tcp_complete(&packet, src, dst);
```

## Que hace `tcp_complete`

- Calcula `data_offset` segun el tamano real de opciones.
- Ajusta `ip.length`.
- Recalcula checksum IP.
- Recalcula checksum TCP sobre pseudo-header + cabecera + opciones + payload.

## Consideraciones

- `data_offset` se mide en palabras de 32 bits, no en bytes.
- Las opciones deben quedar alineadas a 4 bytes.
- El checksum TCP depende de las IP origen/destino, asi que no puede calcularse de forma aislada.
- Esta libreria monta segmentos TCP, pero no implementa una maquina de estados TCP.
