# IP Header
Cosas...
## DSCP/ECN

### Differentiated Services Code Point (DSCP): 6 bits

Originalmente definido como el tipo de servicio (ToS), este campo ahora especifica los servicios diferenciados (DiffServ).  
El flujo de datos en tiempo real hace uso del campo DSCP. Un ejemplo es Voice over IP (VoIP), que se utiliza para servicios de voz interactivos.

### Explicit Congestion Notification (ECN): 2 bits

Este campo permite una notificación de congestión de red de extremo a extremo sin necesidad de descartar paquetes.  
La ECN es una característica opcional, disponible cuando ambos extremos la soportan y efectiva solo si también la soporta la red subyacente.

### Estructura del campo DSCP/ECN

```
Bit:    7   6   5   4   3   2   1   0
       |----DSCP (6 bits)-----|ECN(2)|
```

- **DSCP**: 6 bits más significativos (0-63)
- **ECN**: 2 bits menos significativos (0-3)

### Valores ECN por Protocolo

```c
// TCP
uint8_t ecn = 1;  // ECT(1) - Estándar moderno

// UDP
uint8_t ecn = 0;  // Not ECT - Sin control de congestión

// ICMP
uint8_t ecn = 0;  // Not ECT - Mensajes de control
```

### Conversión Rápida

```c
// ToS → DSCP
uint8_t DSCP_ECN = (tos & 0xFC) | ecn;

// DSCP → ToS
uint8_t DSCP = (tos & 0xFC);
uint8_t ECN = tos & 0x03;
```

### Valores ECN

| Valor | Binario | Nombre  |                 Uso                 |
|-------|---------|---------|-------------------------------------|
| 0     | 00      | Not ECT | No compatible con ECN               |
| 1     | 01      | ECT(1)  | Compatible ECN - estándar           |
| 2     | 10      | ECT(0)  | Compatible ECN - alternativo        |
| 3     | 11      | CE      | Congestión detectada (solo routers) |

### Valores DSCP Comunes

|  ToS  |  DSCP   | Nombre  |                Uso                  |
|-------|---------|---------|-------------------------------------|
| 0     | 0       | CS0/BE  | Best Effort                         |
| 32    | 8       | CS1     | Bulk/Respaldo                       |
| 40    | 10      | AF11    | Baja prioridad, baja pérdida        |
| 48    | 12      | AF12    | Baja prioridad, media pérdida       |
| 56    | 14      | AF13    | Baja prioridad, alta pérdida        |
| 64    | 16      | CS2     | Standard                            |
| 72    | 18      | AF21    | Media prioridad, baja pérdida       |
| 80    | 20      | AF22    | Media prioridad, media pérdida      |
| 88    | 22      | AF23    | Media prioridad, alta pérdida       |
| 96    | 24      | CS3     | Crítico                             |
| 104   | 26      | AF31    | Alta prioridad, baja pérdida        |
| 112   | 28      | AF32    | Alta prioridad, media pérdida       |
| 120   | 30      | AF33    | Alta prioridad, alta pérdida        |
| 136   | 34      | AF41    | Video streaming                     |
| 144   | 36      | AF42    | Video streaming, media pérdida      |
| 152   | 38      | AF43    | Video streaming, alta pérdida       |
| 184   | 46      | EF      | VoIP/Tiempo real                    |
| 192   | 48      | CS6     | Control de red                      |
| 224   | 56      | CS7     | Reservado                           |
