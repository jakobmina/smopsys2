#ifndef QCORE_SECURITY_H
#define QCORE_SECURITY_H

#include <stdint.h>
#include <stddef.h>
#include "qcore_port.h"
#include "qcore_math.h"

// --- Configuración de Umbrales de Seguridad ---
#define ANOMALY_THRESHOLD      0x00060000 // Umbral de Mahalanobis (D^2)
#define CRITICAL_BREACH_LEVEL  0x00100000 // Nivel de "Secuestro" detectado
#define FORENSIC_MEM_ADDR      0x00001000 // Dirección protegida del log

// --- Estados del Sistema ---
typedef enum {
    LAMINAR_ACTIVE,   // Operación normal cifrada
    HIBERNATION,      // Datos evaporados (en espera)
    SINGULARITY       // Autodestrucción en progreso
} security_state_t;

// --- API Pública ---
void apply_phase_encryption(uint32_t* buffer, size_t size, majorana_byte_t q_state);
void write_forensic_log(fixed_t surprise, majorana_byte_t last_q);
void trigger_singularity(void);
void security_heartbeat(uint32_t* main_buffer, size_t size, fixed_t surprise, majorana_byte_t q_state);
security_state_t get_security_state(void);

#endif // QCORE_SECURITY_H
