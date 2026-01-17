#include "../include/qcore_lindblad.h"
#include "../include/qcore_math.h"

// Aliases para compatibilidad
#define fixed_mul mult_q16
#define fixed_div div_q16

// Umbral para transición Bosónico → Fermiónico
// Si visibility_score < 0.3, consideramos el estado como Fermiónico
#define FERMIONIC_THRESHOLD 0x00004CCC  // 0.3 en Q16.16

void lindblad_init(LindblادState* state) {
    state->bf_axis = int_to_fixed(1);  // Inicialmente Bosónico (+1)
    state->visibility_score = int_to_fixed(1);  // Completamente visible
    state->cycle_count = 0;
    
    // Configurar tasas de decoherencia según el ratio áureo
    // Γ_φ = 1.0 (normalizado)
    state->gamma_phi = int_to_fixed(1);
    
    // Γ_π = φ² * Γ_φ ≈ 2.618
    state->gamma_pi = PHI_SQUARED_FIXED;
}

void lindblad_update(LindblادState* state, fixed_t surprise, uint8_t majorana_state) {
    state->cycle_count++;
    
    // ========================================================================
    // PASO 1: Calcular bf_axis usando el Operador Áureo O_n
    // ========================================================================
    // O_n = cos(π n) * cos(π φ n)
    // 
    // Simplificación: Usamos el cycle_count como 'n'
    // cos(π n) alterna entre +1 y -1 cada ciclo (paridad)
    // cos(π φ n) oscila con período áureo
    
    fixed_t cos_pi_n = (state->cycle_count % 2 == 0) ? int_to_fixed(1) : int_to_fixed(-1);
    
    // Para cos(π φ n), aproximamos usando una tabla de lookup o Taylor
    // Simplificación: Usamos una oscilación modulada por φ
    // cos(π φ n) ≈ cos(π * 1.618 * n) 
    // Período ≈ 2/φ ≈ 1.236 ciclos
    
    // Aproximación simple: Oscilación cada ~5 ciclos (período áureo escalado)
    uint32_t phi_phase = (state->cycle_count * 1618) / 1000;  // Escala por φ
    // Convertir a radianes: (phi_phase % 360) * PI / 180
    fixed_t angle_deg = int_to_fixed(phi_phase % 360);
    fixed_t angle_rad = fixed_mul(angle_deg, PI_FIXED) / 180;
    fixed_t cos_pi_phi_n = fixed_cos(angle_rad);
    
    // O_n = cos(π n) * cos(π φ n)
    state->bf_axis = fixed_mul(cos_pi_n, cos_pi_phi_n);
    
    
    // ========================================================================
    // PASO 2: Modular visibility_score por la sorpresa
    // ========================================================================
    // Lógica:
    // - Sorpresa alta → Decoherencia → Visibilidad baja (Fermiónico)
    // - Sorpresa baja → Coherencia → Visibilidad alta (Bosónico)
    
    // Normalizamos la sorpresa a [0, 1]
    // Asumimos MAX_ENTROPY_TOLERANCE = 393216 (6.0 en Q16.16)
    #define MAX_SURPRISE 393216
    
    fixed_t normalized_surprise = surprise;
    if (normalized_surprise > MAX_SURPRISE) {
        normalized_surprise = MAX_SURPRISE;
    }
    
    // visibility = 1 - (surprise / MAX_SURPRISE)
    fixed_t surprise_factor = fixed_div(normalized_surprise, MAX_SURPRISE);
    state->visibility_score = int_to_fixed(1) - surprise_factor;
    
    // Asegurar que visibility_score esté en [0, 1]
    if (state->visibility_score < 0) {
        state->visibility_score = 0;
    }
    
    
    // ========================================================================
    // PASO 3: Aplicar influencia del majorana_state
    // ========================================================================
    // Si el QPU colapsó a 1 (Partícula), aumentamos la polaridad (Fermiónico)
    // Si colapsó a 0 (Onda), aumentamos la coherencia (Bosónico)
    
    if (majorana_state == 1) {
        // Colapso → Reducir visibilidad (Partícula es localizada)
        fixed_t penalty = int_to_fixed(1) / 10;  // -10% de visibilidad
        state->visibility_score -= penalty;
    } else {
        // Coherencia → Aumentar visibilidad (Onda es extendida)
        fixed_t bonus = int_to_fixed(1) / 20;  // +5% de visibilidad
        state->visibility_score += bonus;
    }
    
    // Clamp final
    if (state->visibility_score < 0) {
        state->visibility_score = 0;
    }
    if (state->visibility_score > int_to_fixed(1)) {
        state->visibility_score = int_to_fixed(1);
    }
}

fixed_t lindblad_get_visibility(const LindblادState* state) {
    return state->visibility_score;
}

int lindblad_should_launder(const LindblادState* state) {
    // Si la visibilidad cae por debajo del umbral Fermiónico,
    // la información debe ser "lavada" (protegida/invisible)
    return (state->visibility_score < FERMIONIC_THRESHOLD) ? 1 : 0;
}
