#include "../include/qcore_phase.h"

void phase_init(PhaseState* state) {
    state->accumulator = 0;
    state->cycle_count = 0;
    state->total_corrections = 0;
}

int phase_update_pentagonal(PhaseState* state, fixed_t innovation) {
    state->cycle_count++;
    state->accumulator += innovation;

    int corrected = 0;
    fixed_t abs_acc = (state->accumulator < 0) ? -state->accumulator : state->accumulator;

    // Regla de los 7 ciclos o límite de fase superado
    if ((state->cycle_count % 7 == 0) || (abs_acc > PHASE_THRESHOLD)) {
        // Corrección del Residuo Pentagonal (0.72 rad)
        // Escalamos el acumulador hacia el residuo de deriva
        // En DIT: acc *= (0.7168 / 2.0)
        // Simplificado: acc >> 1 (aprox 0.5) y aplicar signo del residuo
        
        fixed_t multiplier = 23488; // (0.7168 / 2.0) * 65536 ≈ 0.3584 * 65536
        state->accumulator = mult_q16(state->accumulator, multiplier);
        
        state->total_corrections++;
        corrected = 1;
    }

    return corrected;
}

int phase_is_laminar(PhaseState* state) {
    fixed_t abs_acc = (state->accumulator < 0) ? -state->accumulator : state->accumulator;
    return (abs_acc <= PHASE_THRESHOLD);
}
