#ifndef QCORE_PHASE_H
#define QCORE_PHASE_H

#include "qcore_math.h"

// Residuo de 7 ciclos: 7 - 2pi ≈ 0.7168 rad (0.72 aproximado)
// En Q16.16: 0.7168 * 65536 ≈ 46976
#define PHASE_RESIDUE_072  46976
#define PHASE_THRESHOLD    131072 // 2.0 en Q16.16

typedef struct {
    fixed_t accumulator;
    uint32_t cycle_count;
    uint32_t total_corrections;
} PhaseState;

/**
 * @brief Inicializa el estado de fase.
 */
void phase_init(PhaseState* state);

/**
 * @brief Actualiza la acumulación de fase basada en el Operador Golden.
 * @return 1 si se realizó una corrección de 7 ciclos, 0 en caso contrario.
 */
int phase_update_pentagonal(PhaseState* state, fixed_t innovation);

/**
 * @brief Verifica si el sistema está en flujo laminar o turbulencia.
 * @return 1 si es laminar (dentro de los límites), 0 si es turbulento.
 */
int phase_is_laminar(PhaseState* state);

#endif // QCORE_PHASE_H
