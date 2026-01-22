#include "../include/qcore_pim.h"

// Ubicamos los vectores exactamente en la sección protegida
__attribute__((section(".smop_laminar_mem"), aligned(4096)))
LaminarCell pim_tensor_x[TENSOR_BASE_N];

__attribute__((section(".smop_laminar_mem"), aligned(4096)))
LaminarCell pim_tensor_y[TENSOR_BASE_N];

__attribute__((section(".smop_laminar_mem"), aligned(4096)))
LaminarCell pim_tensor_z[TENSOR_BASE_N];
