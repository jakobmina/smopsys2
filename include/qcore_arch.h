#ifndef QCORE_ARCH_H
#define QCORE_ARCH_H

#include <stdint.h>

// Detect Architecture
#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_X86_64
#elif defined(__riscv) || defined(__riscv__)
    #define ARCH_RISCV
#else
    #define ARCH_UNKNOWN
#endif

// Architecture Primitive: Read CPU Cycle Counter / Hardware Tick
static inline uint64_t get_hardware_tick(void) {
#if defined(ARCH_X86_64)
    uint32_t lo, hi;
    // rdtsc: Read Time-Stamp Counter
    __asm__ volatile ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
#elif defined(ARCH_RISCV)
    uint64_t n;
    // rdtime: Read Time CSR (Code Size Reduced for RV32/64)
    __asm__ volatile ("rdtime %0" : "=r" (n));
    return n;
#else
    // Fallback for mock/simulation
    return 0xDEADBEEF;
#endif
}

// Architecture Primitive: CPU Relax / Yield (Efficiency)
static inline void cpu_relax_yield(void) {
#if defined(ARCH_X86_64)
    __asm__ volatile ("pause" ::: "memory");
#elif defined(ARCH_RISCV)
    // RISC-V hint instruction for pause/yield usually map to div usually or dedicated mnemonic
    // standard 'pause' hint is available in newer extensions
    __asm__ volatile ("nop"); 
#else
    // do nothing
#endif
}

#endif // QCORE_ARCH_H
