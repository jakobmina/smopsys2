#include "../include/qcore_security.h"
#include "../include/qcore_port.h"
#include "../include/qcore_math.h"

static security_state_t current_state = LAMINAR_ACTIVE;

/**
 * 1. ENCRIPTADO DE FASE (Capa 0)
 * Aplica una rotación de fase reversible basada en el Operador Dorado.
 */
void apply_phase_encryption(uint32_t* buffer, size_t size, majorana_byte_t q_state) {
    uint32_t phase_key = (uint32_t)q_state.raw << 24 | (uint32_t)q_state.topology.phase_trajectory << 16;
    
    for (size_t i = 0; i < size; i++) {
        // Cifrado simétrico de bajo impacto térmico
        buffer[i] ^= (phase_key ^ 0x5A5A5A5A);
    }
}

/**
 * 2. REGISTRO FORENSE (The Black Box)
 * Guarda la huella digital del ataque antes de la evaporación total.
 */
void write_forensic_log(fixed_t surprise, majorana_byte_t last_q) {
    #ifdef QCORE_TEST_ENV
    // En test environment, usamos un buffer estático para evitar segfault
    static uint32_t forensic_log_buffer[16];
    volatile uint32_t* log = forensic_log_buffer;
    #else
    // En hardware real, escribimos a la dirección física
    volatile uint32_t* log = (uint32_t*)FORENSIC_MEM_ADDR;
    #endif
    
    log[0] = 0xDEADBEEF;             // Magic: Breach detected
    log[1] = (uint32_t)surprise;      // Intensidad de la anomalía
    log[2] = (uint32_t)last_q.raw;    // Último estado del QPU registrado
    // El log se cifra con una llave maestra de hardware (hardwired)
}

/**
 * 3. AUTODESTRUCCIÓN (Singularity Protocol)
 * Inunda la memoria con entropía pura del QPU, borrando el rastro de Landauer.
 */
void trigger_singularity(void) {
    current_state = SINGULARITY;
    
    // Acceso directo a la RAM física para sobreescritura masiva
    uint32_t* ram_ptr = (uint32_t*)0x80000000; // Base de RAM en Smopsys
    size_t ram_size_words = (128 * 1024 * 1024) / 4;

    for (size_t i = 0; i < ram_size_words; i++) {
        // Obtenemos ruido blanco real del puerto cuántico
        // No es pseudo-aleatorio; es el colapso del sensor sin control de fase
        ram_ptr[i] = QPORT->DATA_LATCH ^ 0xFFFFFFFF;
        
        // Pequeño delay para asegurar que el ruido sature el bus
        for(int j=0; j<10; j++) __asm__ volatile("nop");
    }

    // Bloqueo físico del MMQI para evitar re-sincronización
    QPORT->CONTROL_REG = 0x0000000F; // Command: PERMANENT_SHUTDOWN
    
    while(1) {
        #ifdef QCORE_TEST_ENV
        // En entorno de test (x86), usamos un loop vacío
        for(volatile int i = 0; i < 1000; i++);
        #else
        // En hardware real (RISC-V), usamos WFI
        __asm__ volatile("wfi"); // El sistema muere aquí
        #endif
    }
}

/**
 * 4. MONITOR DE SEGURIDAD (El Vigilante)
 * Se integra en el ciclo principal del kernel.
 */
void security_heartbeat(uint32_t* main_buffer, size_t size, fixed_t surprise, majorana_byte_t q_state) {
    
    if (surprise > CRITICAL_BREACH_LEVEL) {
        // --- SECUESTRO DETECTADO ---
        write_forensic_log(surprise, q_state);
        trigger_singularity();
    } 
    else if (surprise > ANOMALY_THRESHOLD) {
        // --- EVAPORACIÓN (Hibernación) ---
        // Los datos se vuelven inaccesibles pero recuperables
        current_state = HIBERNATION;
        apply_phase_encryption(main_buffer, size, q_state);
    } 
    else {
        // --- CONDENSACIÓN (Operación Normal) ---
        if (current_state == HIBERNATION) {
            // Revertimos la fase para recuperar los datos (Condensación)
            apply_phase_encryption(main_buffer, size, q_state); 
            current_state = LAMINAR_ACTIVE;
        }
    }
}

/**
 * Getter para el estado de seguridad actual
 */
security_state_t get_security_state(void) {
    return current_state;
}
