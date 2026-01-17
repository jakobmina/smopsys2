#ifndef QCORE_MATH_H
#define QCORE_MATH_H

#include <stdint.h>

// Definición de constantes áureas en punto fijo Q16.16
#define PHI 0x00019E37      // 1.618033... en punto fijo
#define PI_FIXED 0x0003243F // 3.14159... en punto fijo

typedef int32_t fixed_t;

// Lagrangian components structure per Rule 3.1
typedef struct {
    fixed_t L_symp; // Symplectic component (Conservative)
    fixed_t L_metr; // Metriplectic component (Dissipative) 
} LagrangianState;

// Q16.16 Arithmetic Helpers
fixed_t int_to_fixed(int32_t i);
fixed_t fixed_to_int(fixed_t f);
fixed_t mult_q16(fixed_t a, fixed_t b);
fixed_t div_q16(fixed_t a, fixed_t b);
fixed_t fixed_cos(fixed_t angle);

#define FIX_MUL(a, b) mult_q16(a, b)
#define FIX_DIV(a, b) div_q16(a, b)

// Core Physics Functions
fixed_t calculate_golden_operator(int32_t n);
LagrangianState compute_lagrangian(fixed_t u, fixed_t v);

#endif // QCORE_MATH_H
