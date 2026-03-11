<div align="center">

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

### Pila soportada

`ft_packet` puede ensamblar las siguientes capas:

| Capa       | Soporte                                       |
|------------|-----------------------------------------------|
| `Ethernet` | Creación de cabecera de trama                 |
| `ARP`      | Creación de request y reply                   |
| `IPv4`     | Campos de cabecera y opciones                 |
| `ICMP`     | Echo, reply, redirect, unreachable, timestamp |
| `UDP`      | Creación de cabecera y checksum               |
| `TCP`      | Flags, checksum y opciones                    |
| `Payload`  | Inserción de datos arbitrarios                |

## 🔧 Instalación

```bash
git clone https://github.com/Kobayashi82/ft_packet.git
cd ft_packet
make
```

La compilación genera la librería estática `lib/libft_packet.a`

## 🖥️ Uso

### Integración

Enlaza contra la librería generada:

```bash
cc example.c -Iinc -Llib -lft_packet
```

### Flujo básico

El flujo previsto es:

1. `Crear cabeceras` con `ethernet_create`, `ip_create`, `icmp_create_echo`, `udp_create` o `tcp_create`
2. `Añadir cada bloque` a un `t_packet` con `packet_add`
3. `Añadir payload` si el protocolo de transporte lo permite
4. `Finalizar el paquete` con `icmp_complete`, `udp_complete` o `tcp_complete`
5. `Reutilizar o reiniciar` el paquete con `packet_clear`

### Ejemplo

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

## 📡 Funcionamiento Interno

### Reglas de ensamblado

`ft_packet` guarda los bytes finales dentro de un buffer fijo (`MAX_PACKET_LEN = 2048`) y mantiene punteros tipados a las cabeceras ya insertadas. Esto permite validar el orden de las capas mientras se construye el paquete.

Composiciones válidas típicas:

```text
Ethernet + ARP
Ethernet + IP + ICMP + Payload
Ethernet + IP + UDP  + Payload
Ethernet + IP + TCP  + TCP options + Payload
IP + ICMP/UDP/TCP + Payload
```

### Lógica de completado

Las funciones de completado actualizan los campos dependientes del protocolo antes de enviar o analizar el paquete:

| Función         | Finaliza                                             |
|-----------------|------------------------------------------------------|
| `icmp_complete` | Checksum ICMP y longitud/checksum IP cuando aplica   |
| `udp_complete`  | Longitud UDP, checksum UDP y longitud/checksum IP    |
| `tcp_complete`  | Data offset TCP, checksum TCP y longitud/checksum IP |

### Cobertura de checksums

La librería sigue el modelo estándar de checksum para cada protocolo:

| Cabecera   | Campo de longitud  | Cobertura del checksum             |
|------------|--------------------|------------------------------------|
| `Ethernet` | No                 | FCS gestionado por la NIC          |
| `ARP`      | No                 | Sin checksum                       |
| `IP`       | Cabecera + payload | Solo cabecera IP                   |
| `ICMP`     | No                 | Cabecera + payload                 |
| `UDP`      | Cabecera + payload | Pseudo-header + cabecera + payload |
| `TCP`      | Cabecera + payload | Pseudo-header + cabecera + payload |

## 📄 Licencia

Este proyecto está licenciado bajo la WTFPL – [Do What the Fuck You Want to Public License](http://www.wtfpl.net/about/).

---

<div align="center">

**🧭 Desarrollado como parte del curriculum de 42 School 🧭**

*"Building packets the hard way"*

</div>
