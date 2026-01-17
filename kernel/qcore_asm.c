#include "../include/qcore_asm.h"
#include "../include/qcore_arch.h"

void disable_interrupts(void) {
#ifdef ARCH_RISCV
    // Disable all interrupts in M-mode by clearing MIE bit in mstatus
    __asm__ volatile ("csrci mstatus, 8");
#endif
}

