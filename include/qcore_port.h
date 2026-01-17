#ifndef QCORE_PORT_H
#define QCORE_PORT_H

#include <stdint.h>

// --- Phase 1: Physical Interface Definition ---

// Magic Signature "QBRG" (Quantum Bridge)
#define QPORT_MAGIC_SIG 0x51425247

// Command Bits (CONTROL_REG)
#define CMD_RESET         0x01
#define CMD_CALIBRATE     0x02
#define CMD_PREPARE_STATE 0x04
#define CMD_MEASURE       0x08

// Status Bits (STATUS_REG)
#define STATUS_READY            0x01 // QPU Ready for commands
#define STATUS_TEMP_OK          0x02 // Superconducting temp reached (mK)
#define STATUS_DATA_READY       0x04 // Collapse complete, data valid
#define STATUS_DECOHERENCE_WARN 0x08 // Warning: State degrading

// Register Map Structure
// Must be packed or aligned to 32-bit words as per hardware spec
typedef struct {
    volatile uint32_t MAGIC_SIG;   // 0x00: Read-Only 0x51425247
    volatile uint32_t CONTROL_REG; // 0x04: Write-Only Commands
    volatile uint32_t STATUS_REG;  // 0x08: Read-Only Status
    volatile uint32_t DATA_LATCH;  // 0x0C: Read/Write Majorana Byte
    volatile uint32_t PHASE_LOCK;  // 0x10: Read-Only Cycle Counter
} quantum_port_t;

// Majorana Topological Byte
// Topología de 8 bits: 7 bits de fase continua + 1 bit de estado discreto
typedef union {
    uint8_t raw;
    struct {
        uint8_t phase_trajectory : 7; // Bits 0-6: Fase (0-127)
        uint8_t majorana_state   : 1; // Bit 7: Colapso (0 o 1)
    } topology;
} majorana_byte_t;

// Base Address Configuration
#ifdef QCORE_TEST_ENV
    // In Test Environment, we map to a static buffer to avoid SegFault
    extern uint8_t mock_mmio_buffer[1024];
    #define QPORT_BASE_ADDR ((uintptr_t)mock_mmio_buffer)
#else
    // Production: Physical Address
    #define QPORT_BASE_ADDR 0xF8000000
#endif

// Pointer Accessor
#define QPORT ((quantum_port_t *)QPORT_BASE_ADDR)

// --- Public API Prototypes ---
extern int g_simulation_mode;
void qport_handshake(void);
void bridge_tick_sync(majorana_byte_t *cycle);

#endif // QCORE_PORT_H
