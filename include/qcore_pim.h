#ifndef QCORE_PIM_H
#define QCORE_PIM_H

#include <stdint.h>

#define MEM_RESERVOIR_SIZE 1024

typedef struct {
    float weight;      // El "peso" neuronal (Memoria)
    float probability; // El "prior" bayesiano (Procesador)
    uint64_t metadata; // Estado de fragmentación energética
} LaminarCell;

// Ubicamos la matriz exactamente en la sección protegida
__attribute__((section(".smop_laminar_mem"), aligned(4096)))
extern LaminarCell bayesian_pim_core[MEM_RESERVOIR_SIZE];

// Rutina de actualización Bayesiana-Neuronal (RISC-V Assembly)
// fa0: Golden Prior (float)
extern void smopsys_bayesian_update(LaminarCell* core, uint32_t count, float golden_prior);

#endif // QCORE_PIM_H
