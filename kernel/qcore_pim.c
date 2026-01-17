#include "../include/qcore_pim.h"

// Ubicamos la matriz exactamente en la sección protegida
__attribute__((section(".smop_laminar_mem"), aligned(4096)))
LaminarCell bayesian_pim_core[MEM_RESERVOIR_SIZE];
