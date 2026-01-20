#include "../include/qcore_port.h"
#include "../include/qcore_uart.h"
#include "../include/qcore_viz.h"

// Global flag for hardware presence
int g_simulation_mode = 0;

// Define the mock buffer if in test env
#ifdef QCORE_TEST_ENV
    uint8_t mock_mmio_buffer[1024] __attribute__((aligned(32)));;
#endif

// Helper to relax CPU while waiting
static inline void cpu_relax(void) {
    #if defined(__riscv)
        __asm__ volatile ("nop");
    #elif defined(__x86_64__)
        __asm__ volatile ("pause");
    #else
        // busy loop
    #endif
}

// Global System Phase Zero (calibrated at boot)
static uint32_t system_phase_zero = 0;

// --- Phase 2: Handshake Protocol ---
void qport_handshake(void) {
    uart_puts("--------------------------------------------\n\r");
    uart_puts("BOOT MENU: SELECT QUANTUM BRIDGE MODE\n\r");
    uart_puts(" [S] - Simulation Mode (Recommended for QEMU)\n\r");
    uart_puts(" [H] - Hardware Mode (Quantum Probe)\n\r");
    uart_puts("--------------------------------------------\n\r");
    uart_puts("Defaulting to Simulation in 3s...\n\r");

#ifdef QCORE_TEST_ENV
    int countdown = 1000;
#else
    int countdown = 3000000; // Adjusted for QEMU speed
#endif
    while (countdown > 0) {
        char c = uart_getc_nonblocking();
        if (c == 's' || c == 'S' || countdown == 1) {
            g_simulation_mode = 1;
            uart_puts("\n[ SYSTEM: SIMULATION MODE ACTIVE ]\n\r");
            return;
        }
        if (c == 'h' || c == 'H') {
            uart_puts("\n[ PROBING QUANTUM HARDWARE... ]\n\r");
            if (QPORT->MAGIC_SIG != QPORT_MAGIC_SIG) {
                g_simulation_mode = 1;
                uart_puts(ANSI_COLOR_YELLOW "[ WARNING: HARDWARE NOT FOUND - FAILING BACK TO SIM ]\n\r" ANSI_COLOR_RESET);
                return;
            }
            break; // Hardware found
        }
        countdown--;
        if (countdown % 1000000 == 0) uart_putc('.');
    }

    // 2. Thermal Purge: Send Calibrate Command
    QPORT->CONTROL_REG = CMD_CALIBRATE;
    uart_puts("Locked. Cooling Superconductors...\n\r");
    
    int timeout = 10000000;
    while (!(QPORT->STATUS_REG & STATUS_TEMP_OK)) {
        cpu_relax();
        timeout--;
        if (timeout == 0) {
             g_simulation_mode = 1;
             uart_puts(ANSI_COLOR_RED "\n[ HARDWARE ERROR: THERMAL OVERLOAD ]\n" ANSI_COLOR_RESET);
             return;
        }
    }

    // 3. Phase Lock
    system_phase_zero = QPORT->PHASE_LOCK;
    uart_puts(ANSI_COLOR_GREEN "[ QUANTUM BRIDGE ONLINE ]\n\r" ANSI_COLOR_RESET);
}

// --- Phase 3: Execution Synchronization ---
// The CPU is slave to the QPU collapse event.
void bridge_tick_sync(majorana_byte_t *cycle) {
    if (g_simulation_mode) {
        // En modo simulación, generamos un colapso pseudo-aleatorio
        // y simulamos un pequeño delay de procesamiento.
        for (volatile int i = 0; i < 10000; i++);
        
        uint8_t mock_state = (uint8_t)((pseudo_random() % 2) << 7);
        cycle->raw = (uint8_t)((cycle->raw & 0x7F) | mock_state);
        return;
    }

    // 1. Preparation (Pre-Pulse)
    // Write only the Phase Trajectory bits to the latch (Bits 0-6)
    // We mask bit 7 just in case logic was dirty, though structure handles it.
    QPORT->DATA_LATCH = (uint32_t)(cycle->raw & 0x7F);
    
    // Command QPU to preparing state
    QPORT->CONTROL_REG = CMD_PREPARE_STATE;

    // 2. Estasis (Energy Fragmenting)
    // Wait for the collapse event (Data Ready)
    // The CPU must be silent (NOPs/WFI) to avoid EM noise
    while (!(QPORT->STATUS_REG & STATUS_DATA_READY)) {
        // En un sistema real usaríamos 'wfi' para bajo consumo
        // Aquí usaremos busy-wait estricto según la especificación
        cpu_relax();
    }
    
    // 3. Collapse (Post-Pulse)
    // Read the collapsed reality from the latch
    uint32_t collapsed_data = QPORT->DATA_LATCH;
    
    // Update the Majorana State bit (Bit 7) based on what the hardware saw
    // The hardware might have flipped the bit.
    // We assume hardware provides the FULL byte back.
    cycle->raw = (uint8_t)(collapsed_data & 0xFF);
}
