/*
 * Smopsys2: Metriplectic Hardware Driver (Arduino)
 * 
 * PROPÓSITO:
 * Interfaz física para la Ecuación de Onda Metripléctica.
 * Convierte las señales digitales del Kernel (Fase y Entropía)
 * en corrientes de potencia para modular la realidad física (Bobinas/Cargas).
 * 
 * DIAGRAMA DE CIRCUITO (Doble Canal Discreto):
 * 
 *                     VCC (12-24V)
 *                     |
 *               +-----+-----+
 *               |           |
 *              [C1]        [C2] (100uF - 1000uF Decoupling)
 *               |           |
 *               +--[LOAD1]--+--[LOAD2]--+
 *               |           |           |
 *    Arduino    |   D1(^)   |   D2(^)   | (Diodos de Libre Circulación)
 *    [D9] ----[1k]--|G      |           |
 *                   | Q1 (N)|           |
 *               +--|S       |           |
 *               |           |           |
 *    Arduino    |           |   Q2 (N)  |
 *    [D10] ---[1k]----------|G          |
 *               |           |S          |
 *              GND         GND         GND
 * 
 * COMPONENTES:
 * - Q1, Q2: MOSFET Canal N (e.g., IRLZ44N para lógica 5V, o IRF540 con driver)
 * - R_gate: 1k Ohm (Protección de corriente del pin Arduino)
 * - D1, D2: 1N4007 o Diodos Schottky (Si carga es inductiva)
 * - C1, C2: Filtros de fuente principal.
 */

// --- CONFIGURACIÓN ---
#define PIN_MOSFET_LAMINAR    9   // Salida PWM Canal Conservativo (Fase)
#define PIN_MOSFET_TURBULENT  10  // Salida PWM Canal Disipativo (Entropía/Shock)
#define BAUD_RATE             115200

// --- PROTOCOLO SERIAL ---
// Formato de Paquete: [HEADER] [VALOR]
#define HEADER_PHASE    0xA0  // Actualizar Canal Laminar
#define HEADER_ENTROPY  0xB0  // Actualizar Canal Turbulento
#define HEADER_RESET    0xFF  // Apagar todo

void setup() {
  // Configurar pines de potencia
  pinMode(PIN_MOSFET_LAMINAR, OUTPUT);
  pinMode(PIN_MOSFET_TURBULENT, OUTPUT);
  
  // Estado inicial seguro (Apagado)
  analogWrite(PIN_MOSFET_LAMINAR, 0);
  analogWrite(PIN_MOSFET_TURBULENT, 0);

  // Iniciar comunicación con el Kernel Smopsys
  Serial.begin(BAUD_RATE);
}

void loop() {
  if (Serial.available() >= 2) {
    uint8_t header = Serial.read();
    uint8_t value  = Serial.read(); // 0-255 PWM

    // Interpretación Topológica
    switch (header) {
      case HEADER_PHASE:
        // Canal Conservativo: Modulación suave
        // Mapea directamente la "Fase" del sistema a la intensidad del campo.
        analogWrite(PIN_MOSFET_LAMINAR, value);
        break;

      case HEADER_ENTROPY:
        // Canal Disipativo: Descarga de Residuos
        // Se activa cuando la distancia de Mahalanobis detecta una anomalía.
        analogWrite(PIN_MOSFET_TURBULENT, value);
        break;

      case HEADER_RESET:
        // Kill Switch
        analogWrite(PIN_MOSFET_LAMINAR, 0);
        analogWrite(PIN_MOSFET_TURBULENT, 0);
        break;
        
      default:
        // Ruido en la línea -> Ignorar o purgar buffer
        break;
    }
  }
}
