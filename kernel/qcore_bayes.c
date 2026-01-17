#include "../include/qcore_bayes.h"

// Constante de aprendizaje (Alpha). Digamos 0.05 en Q16.16
#define ALPHA 3276  // ~0.05 * 65536

void init_bayesian_attractor(bayesian_attractor_t *attractor) {
    // Initialize with identity/zero
    attractor->mu[0] = 0;
    attractor->mu[1] = 0;
    
    // Covariance Identity * some variance (e.g. 1.0)
    attractor->cov[0][0] = 0x00010000;
    attractor->cov[0][1] = 0;
    attractor->cov[1][0] = 0;
    attractor->cov[1][1] = 0x00010000;

    // Inverse Identity
    attractor->inv_cov[0][0] = 0x00010000;
    attractor->inv_cov[0][1] = 0;
    attractor->inv_cov[1][0] = 0;
    attractor->inv_cov[1][1] = 0x00010000;
}

// 1. Actualización de la Covarianza (Manual)
void update_belief(bayesian_attractor_t *attractor, int32_t input_phase, int32_t input_entropy) {
    int32_t x[2] = {input_phase, input_entropy};
    int32_t diff[2];

    // --- Paso A: Actualizar Media (EMA) ---
    // mu_new = mu_old + alpha * (x - mu_old)
    for(int i=0; i<2; i++) {
        diff[i] = x[i] - attractor->mu[i];
        // mu += alpha * diff
        attractor->mu[i] += FIX_MUL(ALPHA, diff[i]);
    }

    // --- Paso B: Actualizar Covarianza ---
    // Cov_new = Cov_old + alpha * ( (diff * diff') - Cov_old )
    // Solo calculamos cov[0][0], cov[0][1], cov[1][1] (es simétrica)
    
    // Recalculamos diff con la NUEVA media para mayor estabilidad (Welford-like but EMA)
    int32_t diff_new[2];
    for(int i=0; i<2; i++) diff_new[i] = x[i] - attractor->mu[i];

    // Actualización manual elemento por elemento
    // Sigma_00 (Varianza Fase)
    int32_t term_00 = FIX_MUL(diff[0], diff_new[0]); 
    attractor->cov[0][0] += FIX_MUL(ALPHA, (term_00 - attractor->cov[0][0]));

    // Sigma_11 (Varianza Entropía)
    int32_t term_11 = FIX_MUL(diff[1], diff_new[1]); 
    attractor->cov[1][1] += FIX_MUL(ALPHA, (term_11 - attractor->cov[1][1]));

    // Sigma_01 y 10 (Covarianza cruzada)
    int32_t term_01 = FIX_MUL(diff[0], diff_new[1]);
    int32_t update_cov = FIX_MUL(ALPHA, (term_01 - attractor->cov[0][1]));
    attractor->cov[0][1] += update_cov;
    attractor->cov[1][0] = attractor->cov[0][1]; // Simetría

    // --- Paso C: Invertir Matriz (Determinante y Adjunta) ---
    int32_t a = attractor->cov[0][0];
    int32_t b = attractor->cov[0][1];
    int32_t c = attractor->cov[1][0];
    int32_t d = attractor->cov[1][1];

    // Det = ad - bc
    int32_t det = FIX_MUL(a, d) - FIX_MUL(b, c);

    // Protección contra singularidad (si el flujo es demasiado perfecto/estático)
    // Epsilon approx 0.0001
    if (det >= 0 && det < 10) det = 10;
    if (det < 0 && det > -10) det = -10;

    // Matriz Inversa: (1/det) * [d, -b; -c, a]
    // Nota: FIX_DIV(NUM, DEN) maneja el escalado Q16.16
    attractor->inv_cov[0][0] = FIX_DIV(d, det);
    attractor->inv_cov[0][1] = FIX_DIV(-b, det);
    attractor->inv_cov[1][0] = FIX_DIV(-c, det);
    attractor->inv_cov[1][1] = FIX_DIV(a, det);
}

// 2. Cálculo de Distancia de Mahalanobis (Al cuadrado)
// Retorna D^2. Si D^2 > Umbral, es una anomalía.
int32_t calculate_mahalanobis_sq(bayesian_attractor_t *attractor, int32_t input_phase, int32_t input_entropy) {
    int32_t x[2] = {input_phase, input_entropy};
    int32_t diff[2];
    
    // Vector (x - mu)
    diff[0] = x[0] - attractor->mu[0];
    diff[1] = x[1] - attractor->mu[1];

    // Multiplicación Vector-Matriz-Vector: diff^T * InvCov * diff
    // Desglose manual:
    // T1 = diff[0]*inv[0][0] + diff[1]*inv[1][0]
    // T2 = diff[0]*inv[0][1] + diff[1]*inv[1][1]
    
    int32_t t1 = FIX_MUL(diff[0], attractor->inv_cov[0][0]) + FIX_MUL(diff[1], attractor->inv_cov[1][0]);
    int32_t t2 = FIX_MUL(diff[0], attractor->inv_cov[0][1]) + FIX_MUL(diff[1], attractor->inv_cov[1][1]);

    // Resultado final: t1*diff[0] + t2*diff[1]
    int32_t d_squared = FIX_MUL(t1, diff[0]) + FIX_MUL(t2, diff[1]);

    return d_squared; // Retornamos distancia al cuadrado para evitar sqrt()
}
