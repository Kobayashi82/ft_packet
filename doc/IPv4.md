# IPv4

## Estructura de la cabecera

```text
[ ver_ihl ][ dscp_ecn ][ length ][ id ][ frag ]
[ ttl ][ protocol ][ checksum ][ src_addr ][ dst_addr ]
[ options ... ]
```

Tamano minimo: 20 bytes.
Tamano maximo con opciones: 60 bytes.

## Campos principales

| Campo | Descripcion |
|-------|-------------|
| `ver_ihl` | Version IPv4 en el nibble alto e IHL en el nibble bajo |
| `dscp_ecn` | DSCP (6 bits) + ECN (2 bits) |
| `length` | Longitud total del datagrama IP |
| `id` | Identificador para fragmentacion |
| `frag` | Flags (`DF`, `MF`) + fragment offset |
| `ttl` | Time To Live |
| `protocol` | Protocolo superior (`ICMP`, `UDP`, `TCP`) |
| `checksum` | Checksum de la cabecera IP |
| `src_addr` | IPv4 origen |
| `dst_addr` | IPv4 destino |

## API disponible en `ft_packet`

```c
int ip_set_ihl(t_ip *header, uint8_t ihl);
int ip_set_tos(t_ip *header, uint8_t tos);
int ip_set_dscp(t_ip *header, uint8_t dscp);
int ip_set_ecn(t_ip *header, uint8_t ecn);
int ip_set_length(t_ip *header, uint16_t data_len);
int ip_set_id(t_ip *header, uint16_t id);
int ip_set_df(t_ip *header, uint8_t frag_df);
int ip_set_mf(t_ip *header, uint8_t frag_mf);
int ip_set_frag_offset(t_ip *header, uint16_t frag_offset);
int ip_set_ttl(t_ip *header, uint8_t ttl);
int ip_set_protocol(t_ip *header, uint8_t protocol);
int ip_set_checksum(t_ip *header);
int ip_set_src_addr(t_ip *header, uint32_t src_addr);
int ip_set_dst_addr(t_ip *header, uint32_t dst_addr);

int ip_create(t_ip *header, uint8_t dscp, uint8_t ecn,
       uint16_t data_len, uint16_t id, uint8_t frag_df,
       uint8_t frag_mf, uint16_t frag_offset, uint8_t ttl,
       uint8_t protocol, uint32_t src_addr, uint32_t dst_addr);
```

## Uso tipico

```c
t_ip ip = {0};

ip_create(&ip, 0, 0, 0, 0x1234, 1, 0, 0, 64,
       IPPROTO_UDP, inet_addr("10.0.0.1"), inet_addr("10.0.0.2"));
packet_add(&packet, &ip, sizeof(ip), IP);
```

## DSCP y ECN

### Estructura del byte `dscp_ecn`

```text
Bit:    7   6   5   4   3   2   1   0
          |----DSCP (6 bits)-----|ECN(2)|
```

- `DSCP`: 6 bits mas significativos.
- `ECN`: 2 bits menos significativos.

### Valores ECN

| Valor | Binario | Nombre |
|-------|---------|--------|
| `0` | `00` | Not ECT |
| `1` | `01` | ECT(1) |
| `2` | `10` | ECT(0) |
| `3` | `11` | CE |

### Valores DSCP comunes

| DSCP | Nombre | Uso |
|------|--------|-----|
| `0` | CS0 / BE | Best Effort |
| `8` | CS1 | Trafico bulk |
| `16` | CS2 | Servicio estandar |
| `24` | CS3 | Trafico critico |
| `34` | AF41 | Video |
| `46` | EF | Voz / tiempo real |
| `48` | CS6 | Control de red |

## Fragmentacion

La cabecera IP tiene soporte de campos para fragmentacion:

- `DF_FLAG`: `Don't Fragment`
- `MF_FLAG`: `More Fragments`
- `FRAG_OFFSET_MASK`: offset de fragmento

Pero en la practica la libreria no implementa una logica completa de fragmentacion/reensamblado. Puedes ajustar los bits y el offset, pero el proyecto esta orientado a construir datagramas completos.

## Opciones IP

La libreria soporta hasta 40 bytes de opciones IP.

### Opciones disponibles

| Opcion | Valor |
|--------|-------|
| `EOOL` | `0x00` |
| `NOP` | `0x01` |
| `RR` | `0x07` |
| `TS` | `0x44` |
| `RTRALT` | `0x94` |

### API de opciones

```c
int option_set_nop(t_ip_option *opt);
int option_set_eool(t_ip_option *opt);
int option_timestamp_create(t_ip_option *opt,
       uint8_t num_timestamps, uint8_t flags);
int option_set_record_route(t_ip_option *opt,
       uint8_t num_addresses);
int option_set_router_alert(t_ip_option *opt,
       uint16_t alert_value);
```

### Ejemplo con opciones

```c
t_ip ip = {0};
t_ip_option opt = {0};

ip_create(&ip, 0, 0, 0, 0x4321, 1, 0, 0, 64,
       IPPROTO_ICMP, inet_addr("192.168.1.10"), inet_addr("192.168.1.1"));
option_set_record_route(&opt, 3);
option_set_eool(&opt);

packet_add(&packet, &ip, sizeof(ip), IP);
packet_add(&packet, &opt, opt.length, IP_OPTION);
```

## Relacion con `packet_add` y `*_complete`

- `packet_add(..., IP)` inserta la cabecera base.
- `packet_add(..., IP_OPTION)` anade las opciones inmediatamente despues de IP.
- `icmp_complete`, `udp_complete` y `tcp_complete` recalculan `ip.length` e `ip.checksum` al final.

Eso significa que normalmente no hace falta fijar manualmente el `length` final si despues vas a llamar a uno de esos `complete`.

## Consideraciones practicas

- `src_addr` y `dst_addr` deben pasarse en network byte order.
- `protocol` debe ser coherente con la siguiente capa.
- El checksum IP solo cubre la cabecera, no el payload.
- `IHL` se mide en palabras de 32 bits, no en bytes.
- Si anades opciones IP, el `IHL` debe aumentar de `5` a `5 + bytes_opciones / 4`.
