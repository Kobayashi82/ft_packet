<div align="center">

![WIP](https://img.shields.io/badge/work%20in%20progress-yellow?style=for-the-badge)
![System & Kernel](https://img.shields.io/badge/System-brown?style=for-the-badge)
![Network Packets](https://img.shields.io/badge/Network-Packets-blue?style=for-the-badge)
![Protocols](https://img.shields.io/badge/Protocol-Ethernet--ARP--IPv4--ICMP--UDP--TCP-green?style=for-the-badge)
![C Language](https://img.shields.io/badge/Language-C-red?style=for-the-badge)

*Una librería estática en C para construir paquetes de red*

</div>

<div align="center">
  <img src="/images/ft_packet.jpg">
</div>

# ft_packet

[README in English](README.md)

`ft_packet` es una librería de bajo nivel en C para construir paquetes de red desde cero. Expone headers públicos y helpers para ensamblar tramas `Ethernet`, `ARP`, `IPv4`, `ICMP`, `UDP` y `TCP`, incluyendo opciones de IP/TCP, payload y cálculo final de checksums.

### ¿Qué es ft_packet?

Este proyecto está centrado en la composición de paquetes, no en su captura ni en su envío. Sus objetivos principales son:

- `Construir tramas raw` con un orden de capas controlado
- `Configurar campos de protocolo` mediante helpers dedicados
- `Añadir payload y opciones` manteniendo coherencia en la estructura
- `Completar longitudes y checksums` antes de enviar o inspeccionar el paquete
- `Exponer una API pública pequeña` para experimentación y herramientas de red

### Pila soportada

`ft_packet` puede ensamblar las siguientes capas:

| Capa       | Soporte |
|------------|---------|
| `Ethernet` | Creación de cabecera de trama |
| `ARP`      | Creación de request y reply |
| `IPv4`     | Campos de cabecera y opciones |
| `ICMP`     | Echo, reply, redirect, unreachable, timestamp |
| `UDP`      | Creación de cabecera y checksum |
| `TCP`      | Flags, checksum y opciones |
| `Payload`  | Inserción de datos arbitrarios |

## 🔧 Instalación

```bash
git clone https://github.com/Kobayashi82/ft_packet.git
cd ft_packet
make
```

La compilación genera una librería estática:

```bash
lib/libft_packet.a
```

Para ejecutar el tester automático de API:

```bash
make test
```

Para una prueba real enviando un ping construido con la librería:

```bash
sudo make test-live TARGET_IP=127.0.0.1
```

## 🖥️ Uso

### Integración en compilación

Incluye los headers públicos y enlaza contra la librería generada:

```bash
cc example.c -Iinc -Llib -lft_packet
```

### Flujo básico

El flujo previsto es:

1. `Crear cabeceras` con helpers como `ethernet_create`, `ip_create`, `icmp_create_echo`, `udp_create` o `tcp_create`
2. `Añadir cada bloque` a un `t_packet` con `packet_add`
3. `Añadir payload` si el protocolo de transporte lo permite
4. `Finalizar el paquete` con `icmp_complete`, `udp_complete` o `tcp_complete`
5. `Reutilizar o reiniciar` el paquete con `packet_clear`

### Resumen del API público

| Módulo | Capacidades principales |
|--------|-------------------------|
| `packet.h` | `packet_add`, `packet_clear` y helpers de completado |
| `frame/ethernet.h` | Setters Ethernet y `ethernet_create` |
| `frame/arp.h` | Helpers para ARP request y reply |
| `frame/ip.h` | Setters IPv4, checksum y opciones IP |
| `frame/icmp.h` | Creación de mensajes ICMP y checksum |
| `frame/udp.h` | Creación de cabecera UDP y pseudo-header checksum |
| `frame/tcp.h` | Flags TCP, checksum y opciones TCP |

### Ejemplo mínimo

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

## 📡 Funcionamiento Interno

### Reglas de ensamblado

`ft_packet` guarda los bytes finales dentro de un buffer fijo (`MAX_PACKET_LEN = 2048`) y mantiene punteros tipados a las cabeceras ya insertadas. Esto permite validar el orden de las capas mientras se construye el paquete.

Composiciones válidas típicas:

```text
Ethernet + ARP
Ethernet + IP + ICMP + Payload
Ethernet + IP + UDP + Payload
Ethernet + IP + TCP + TCP options + Payload
IP + ICMP/UDP/TCP + Payload
```

### Lógica de completado

Los helpers de completado actualizan los campos dependientes del protocolo antes de enviar o analizar el paquete:

| Helper | Finaliza |
|--------|----------|
| `icmp_complete` | Checksum ICMP y longitud/checksum IP cuando aplica |
| `udp_complete`  | Longitud UDP, checksum UDP y longitud/checksum IP |
| `tcp_complete`  | Data offset TCP, checksum TCP y longitud/checksum IP |

### Cobertura de checksums

La librería sigue el modelo estándar de checksum para cada protocolo:

| Cabecera | Campo de longitud | Cobertura del checksum |
|----------|-------------------|------------------------|
| `Ethernet` | No | FCS gestionado por la NIC |
| `ARP`      | No | Sin checksum |
| `IP`       | Cabecera + payload | Solo cabecera IP |
| `ICMP`     | No | Cabecera + payload |
| `UDP`      | Cabecera + payload | Pseudo-header + cabecera + payload |
| `TCP`      | Cabecera + payload | Pseudo-header + cabecera + payload |

## 🗺️ Documentación del proyecto

El repositorio incluye notas por protocolo y un plan manual de pruebas:

| Archivo | Contenido |
|---------|-----------|
| `doc/Ethernet.md` | Notas sobre tramas Ethernet |
| `doc/ARP.md`      | Referencia de ARP |
| `doc/IPv4.md`     | Cabecera IPv4 y opciones |
| `doc/ICMP.md`     | Tipos de mensajes ICMP |
| `doc/UDP.md`      | Comportamiento UDP y checksum |
| `doc/TCP.md`      | Flags y opciones TCP |
| `doc/test_plan.md`| Escenarios recomendados de validación |

### Validación sugerida

El plan de pruebas actual está orientado a inspeccionar paquetes con herramientas como:

- `make test` para una pasada rápida de regresión sobre la API pública
- `make test-live TARGET_IP=<ip>` para enviar un `ICMP Echo Request` real y esperar el `Echo Reply`

- `Wireshark` o `tcpdump` para validar layout y checksums
- `netcat` para probar payloads UDP
- Una segunda máquina o servicio para validar handshakes TCP
- Privilegios `root` cuando se usen raw sockets

## ⚠️ Limitaciones y Consideraciones

### Alcance actual

- `Solo librería estática`: la compilación por defecto no genera ejecutable
- `Enfoque en construcción`: el envío y la orquestación con sockets quedan fuera de este repositorio
- `Solo IPv4`: el API público actual trabaja sobre Ethernet, ARP e IPv4
- `Buffer fijo`: el tamaño de paquete está limitado por `MAX_PACKET_LEN`

### Consideraciones prácticas

- `El byte order importa`: quien use la librería debe ser consistente con host/network byte order
- `Pruebas raw`: algunos escenarios requieren privilegios elevados
- `Orden de protocolos`: `packet_add` rechaza combinaciones de capas inválidas
- `Alineación de opciones`: IP y TCP requieren opciones alineadas a múltiplos de 4 bytes

### Uso previsto

Esta librería es adecuada para:

- `Aprender` cómo se representan las cabeceras en memoria
- `Construir paquetes` para experimentos con raw sockets
- `Probar` checksums y manejo de opciones
- `Crear tooling` sobre una API pequeña de packet crafting

## 📄 Licencia

Este proyecto está licenciado bajo la WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🧭 Desarrollado como parte del curriculum de 42 School 🧭**

*"Build the packet, byte by byte"*

</div>
