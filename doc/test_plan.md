# Network Library — Test Plan

## Herramientas necesarias

- **Wireshark** o **tcpdump** para capturar y validar paquetes
- **netcat** (`nc`) para recibir UDP
- Un servidor o segunda máquina para pruebas TCP
- Permisos de root (raw sockets los requieren)

---

## Orden recomendado

Probar de menos a más complejidad: ICMP → ARP → UDP → TCP sin opciones → TCP con opciones → IP con opciones.

---

## 1. ICMP

### 1.1 Echo Request básico (ping)
- Construir: Ethernet + IP + ICMP Echo Request
- Enviar a una IP conocida de la red local
- **Validar en Wireshark:**
  - Checksum IP correcto (verde)
  - Checksum ICMP correcto (verde)
  - Type = 8, Code = 0
  - ID y Sequence correctos
  - El destino responde con Echo Reply (Type = 0)

### 1.2 Echo Reply
- Construir: Ethernet + IP + ICMP Echo Reply con el mismo ID y Sequence del request recibido
- **Validar:** que el origen del request lo recibe y lo acepta como reply válido

### 1.3 Echo Request sin Ethernet (raw socket IP)
- Construir solo IP + ICMP Echo Request
- Enviar con `SOCK_RAW` + `IPPROTO_ICMP`
- **Validar:** mismo comportamiento que 1.1

### 1.4 Destination Unreachable
- Construir: Ethernet + IP + ICMP Destination Unreachable (code = 3, port unreachable)
- **Validar en Wireshark:**
  - Type = 3, Code = 3
  - Checksum correcto
  - Campo `un` a cero

### 1.5 Time Exceeded
- Construir: Ethernet + IP + ICMP Time Exceeded (code = 0, TTL expired)
- **Validar en Wireshark:**
  - Type = 11, Code = 0
  - Checksum correcto

### 1.6 Redirect
- Construir: Ethernet + IP + ICMP Redirect con gateway real
- **Validar en Wireshark:**
  - Type = 5
  - El campo gateway contiene la IP correcta en network byte order

### 1.7 Timestamp Request / Reply
- Construir: Ethernet + IP + ICMP Timestamp Request
- **Validar en Wireshark:**
  - Type = 13 (request) / 14 (reply)
  - ID y Sequence correctos
  - Payload de 12 bytes (3 timestamps)

### 1.8 ICMP con payload
- Construir Echo Request con payload de datos arbitrarios
- **Validar:** que el checksum cubre header + payload correctamente y Wireshark lo valida en verde

---

## 2. ARP

### 2.1 ARP Request
- Construir: Ethernet (dst = FF:FF:FF:FF:FF:FF) + ARP Request
- `sha` = MAC propia, `spa` = IP propia, `tpa` = IP a resolver, `tha` = 00:00:00:00:00:00
- **Validar en Wireshark:**
  - Operation = 1 (request)
  - Direcciones MAC e IP en los campos correctos
  - Ethernet dst es broadcast
  - El destino responde con ARP Reply

### 2.2 ARP Reply
- Construir: Ethernet + ARP Reply en respuesta a un request recibido
- `sha` = MAC propia, `spa` = IP propia, `tha` = MAC del solicitante, `tpa` = IP del solicitante
- **Validar en Wireshark:**
  - Operation = 2 (reply)
  - `sha` y `ethernet.src_mac` coherentes
  - `tha` y `ethernet.dst_mac` coherentes

---

## 3. UDP

### 3.1 UDP básico sin checksum
- Construir: Ethernet + IP + UDP sin payload
- `udp_create()` con checksum = 0
- **Validar en Wireshark:**
  - Length correcto (8 bytes, solo header)
  - Checksum = 0x0000 (válido en UDP, es opcional)

### 3.2 UDP con payload y checksum
- Construir: Ethernet + IP + UDP + payload de texto
- `udp_create_checksum()` con src/dst IP reales
- Escuchar con `nc -u -l <puerto>` en el destino
- **Validar en Wireshark:**
  - Checksum correcto (verde)
  - Length = 8 + len(payload)
  - El payload llega íntegro a netcat

### 3.3 UDP desde `packet_add` + `udp_complete`
- Construir el paquete completo usando `packet_add` para cada capa
- Llamar `udp_complete` al final
- **Validar:** mismos criterios que 3.2, verificando que `ip->length` y `udp->length` se calculan correctamente

---

## 4. TCP

### 4.1 SYN (inicio de conexión)
- Construir: Ethernet + IP + TCP con flag SYN
- `seq` = número aleatorio, `ack_num` = 0, `window` = 65535
- Enviar a un puerto abierto
- **Validar en Wireshark:**
  - Flag SYN activo, resto a 0
  - Data offset = 5 (sin opciones)
  - Checksum correcto (verde)
  - El servidor responde con SYN-ACK

### 4.2 SYN-ACK
- Construir respuesta al SYN recibido
- `seq` = ISN propio, `ack_num` = seq_cliente + 1, flags = SYN | ACK
- **Validar:** el cliente lo acepta y envía ACK

### 4.3 ACK
- Construir: TCP con solo flag ACK
- **Validar en Wireshark:**
  - Solo ACK activo
  - `ack_num` correcto
  - Checksum correcto

### 4.4 PSH + ACK con payload
- Construir: Ethernet + IP + TCP (PSH | ACK) + payload
- **Validar en Wireshark:**
  - Flags PSH y ACK activos
  - Checksum cubre pseudo-header + header + payload
  - El payload llega correcto al destino

### 4.5 FIN
- Construir: TCP con flag FIN (+ ACK normalmente)
- **Validar:** que el destino responde con FIN-ACK y la conexión cierra limpiamente

### 4.6 RST
- Construir: TCP con flag RST
- **Validar en Wireshark:**
  - Solo RST activo
  - Checksum correcto

### 4.7 TCP desde `packet_add` + `tcp_complete`
- Construir paquete completo con `packet_add` para cada capa
- Llamar `tcp_complete` al final
- **Validar:** que `data_offset` = 5, IP length correcto, checksum correcto

---

## 5. TCP con opciones

### 5.1 MSS
- Construir SYN con opción MSS (valor típico: 1460)
- `tcp_option_set_mss()` + `packet_add(..., TCP_OPTION)` + `tcp_complete()`
- **Validar en Wireshark:**
  - Opción MSS presente con valor correcto
  - Data offset = 6 (header 20 + opción 4 = 24 bytes = 6 words)
  - Checksum correcto

### 5.2 Window Scale
- Construir SYN con NOP + Window Scale
- `tcp_option_set_nop()` + `tcp_option_set_window_scale()` (shift típico: 7)
- **Validar en Wireshark:**
  - NOP presente antes de Window Scale (alineación)
  - Shift correcto
  - Data offset actualizado

### 5.3 SACK Permitted
- Construir SYN con SACK Permitted
- **Validar en Wireshark:**
  - Kind = 4, Length = 2
  - Data offset correcto

### 5.4 Timestamps
- Construir con NOP + NOP + Timestamps
- `tsval` = timestamp actual, `tsecr` = 0 (en SYN)
- **Validar en Wireshark:**
  - Dos NOPs antes de la opción (alineación a 4 bytes)
  - tsval y tsecr correctos
  - Data offset = 10 (header 20 + opciones 40 = 60 bytes = 15 words) si se combinan todas

### 5.5 SACK (Selective Acknowledgement)
- Construir ACK con SACK de 1 bloque
- `blocks` = { left_edge, right_edge } en host byte order
- **Validar en Wireshark:**
  - Kind = 5
  - Length = 10 (2 + 8 por bloque)
  - Edges en network byte order correctos

### 5.6 Combinación realista de opciones (SYN completo)
- Construir SYN con: MSS + NOP + Window Scale + SACK Permitted + NOP + NOP + Timestamps
- Esta es la combinación típica que envía Linux en un SYN real
- **Validar:** que el data offset es correcto, checksum correcto, y Wireshark muestra todas las opciones bien parseadas

### 5.7 EOL y padding
- Construir opciones que no sean múltiplo de 4 bytes y añadir EOL
- **Validar:** que el bloque de opciones queda alineado a 4 bytes y data offset es correcto

---

## 6. IP con opciones

### 6.1 NOP
- Construir IP + NOP + ICMP
- `option_set_nop()` + `option_set_eool()` + `packet_add(..., IP_OPTION)`
- **Validar en Wireshark:**
  - IHL = 6 (header 20 + 4 bytes opción = 24 bytes)
  - NOP presente en las opciones
  - Checksum IP correcto

### 6.2 Record Route
- Construir IP con Record Route para 3 direcciones
- `option_set_record_route(opt, 3)`
- **Validar en Wireshark:**
  - Kind = 7
  - Length correcto (3 + 3*4 = 15, pero alineado a 4 = 16 bytes)
  - Pointer = 4 (apunta al primer slot)
  - IHL actualizado

### 6.3 Timestamp
- Construir IP con Timestamp para 2 entradas, flags = 0 (solo timestamps)
- `option_timestamp_create(opt, 2, 0)`
- **Validar en Wireshark:**
  - Kind = 68 (0x44)
  - Pointer = 5
  - Flags = 0
  - IHL correcto

### 6.4 Router Alert
- Construir IP con Router Alert (valor = 0, MLD)
- `option_set_router_alert(opt, RTRALT_MLD)`
- **Validar en Wireshark:**
  - Kind = 148 (0x94)
  - Length = 4
  - Value = 0 en network byte order

### 6.5 Combinación de opciones IP
- Construir NOP + Record Route + EOOL
- **Validar:** IHL correcto, checksum correcto, Wireshark parsea todas las opciones

---

## 7. Casos límite y robustez

### 7.1 Punteros NULL
- Llamar a cada función `_set_*` y `_create_*` con `header = NULL`
- **Validar:** todas devuelven 1 sin crash

### 7.2 `packet_add` en orden incorrecto
- Intentar añadir IP antes de Ethernet cuando ya hay ARP
- Intentar añadir payload sin protocolo de transporte
- **Validar:** todas devuelven 1 sin corromper el buffer

### 7.3 `packet_clear`
- Construir un paquete completo, llamar `packet_clear`, construir otro diferente
- **Validar:** que el segundo paquete no tiene residuos del primero

### 7.4 Payload en el límite
- Añadir payload de exactamente `MAX_PACKET_LEN - packet_len - 1` bytes
- **Validar:** acepta el payload
- Añadir un byte más
- **Validar:** devuelve 1

### 7.5 Opciones IP al límite (40 bytes)
- Construir opciones que ocupen exactamente 40 bytes
- **Validar:** se añaden correctamente, IHL = 15
- Intentar añadir un byte más
- **Validar:** devuelve 1

### 7.6 Opciones TCP al límite (40 bytes)
- Mismo criterio que 7.5 pero para TCP options
- **Validar:** data offset = 15 (el máximo)

---

## Checklist de validación en Wireshark

Para cada paquete enviado, verificar:

- [ ] Checksum IP en verde
- [ ] Checksum del protocolo de transporte en verde (ICMP/UDP/TCP)
- [ ] Campo length coincide con el tamaño real del paquete
- [ ] IHL correcto según si hay IP options o no
- [ ] Data offset TCP correcto según si hay TCP options o no
- [ ] Flags correctos (ningún flag inesperado activo)
- [ ] Byte order correcto en todos los campos de 16 y 32 bits
- [ ] El destino responde de forma esperada (cuando aplica)
