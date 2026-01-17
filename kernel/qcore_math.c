#include "../include/qcore_math.h"

// Taylor series constants in Q16.16
#define ONE_FIXED 0x00010000
#define HALF_FIXED 0x00008000 // 0.5

fixed_t int_to_fixed(int32_t i) {
    return i << 16;
}

fixed_t fixed_to_int(fixed_t f) {
    return f >> 16;
}

fixed_t mult_q16(fixed_t a, fixed_t b) {
    return (fixed_t)(((int64_t)a * b) >> 16);
}

fixed_t div_q16(fixed_t a, fixed_t b) {
    if (b == 0) return 0x7FFFFFFF; // Max Int (Saturation)
    return (fixed_t)(((int64_t)a << 16) / b);
}

// Simple range reduction to [-PI, PI]
fixed_t mod_2pi(fixed_t angle) {
    fixed_t two_pi = mult_q16(PI_FIXED, int_to_fixed(2));
    while (angle > PI_FIXED) angle -= two_pi;
    while (angle < -PI_FIXED) angle += two_pi;
    return angle;
}

// Simple Cosine approximation (Maclaurin series)
fixed_t fixed_cos(fixed_t angle) {
    angle = mod_2pi(angle);
    
    // x^2
    fixed_t x2 = mult_q16(angle, angle);
    // x^2 / 2
    fixed_t term2 = mult_q16(x2, HALF_FIXED);
    
    // For better accuracy with large angles in [-PI, PI], we might need x^4/24 term
    // But for "Conceptual" code, just adding range reduction helps significantly.
    // However, 1 - x^2/2 is only good for small x.
    // Let's add x^4 / 24.
    // 24 in fixed is 24 << 16 = 0x00180000
    // 1/24 in fixed approx 0x00000AAA (0.04166)
    
    #define INV_24_FIXED 0x00000AAA
    
    fixed_t x4 = mult_q16(x2, x2);
    fixed_t term4 = mult_q16(x4, INV_24_FIXED);
    
    return ONE_FIXED - term2 + term4;
}

// Operador Áureo: Ón = cos(πn) * cos(πφn)
fixed_t calculate_golden_operator(int32_t n) {
    // Paridad local (cos(πn)) alterna entre 1 y -1
    fixed_t parity = (n % 2 == 0) ? ONE_FIXED : -ONE_FIXED;
    
    // Geometría global cuasiperiódica (cos(πφn))
    fixed_t n_fixed = int_to_fixed(n);
    fixed_t phi_n = mult_q16(PHI, n_fixed);
    fixed_t phase = mult_q16(PI_FIXED, phi_n);
    
    // Range reduction is now handled inside fixed_cos
    fixed_t geometry = fixed_cos(phase); 
    
    return mult_q16(parity, geometry);
}

// Rule 3.1: Explicit Lagrangian Computation
LagrangianState compute_lagrangian(fixed_t u, fixed_t v) {
    LagrangianState state;
    
    // L_symp = Kinetic - Potential (Hamiltonian-like structure)
    // H = T + V. For conservative motion d_symp = {u, H}
    // We retain the Energy H as the marker for the symplectic part
    // For simplicity: H = 1/2 v^2 + 1/2 u^2 (Harmonic Oscillator)
    fixed_t v2 = mult_q16(v, v);
    fixed_t u2 = mult_q16(u, u);
    fixed_t H = mult_q16(HALF_FIXED, v2) + mult_q16(HALF_FIXED, u2);
    
    state.L_symp = H;

    // L_metr = Entropy Production (Potential of Dissipation)
    // d_metr = [u, S]. S represents entropy.
    // We return S. For simplicity S = -1/2 v^2 (frictional damping)
    state.L_metr = -mult_q16(HALF_FIXED, v2); 

    return state;
}
