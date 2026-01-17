#ifndef QCORE_BAYES_H
#define QCORE_BAYES_H

#include <stdint.h>
#include "qcore_math.h"

// Estructura del "Prior" Bayesiano (El estado del atractor)
typedef struct {
    int32_t mu[2];          // Media [Fase, Entropía]
    int32_t cov[2][2];      // Matriz de Covarianza 2x2
    int32_t inv_cov[2][2];  // Inversa (Cacheada para velocidad)
} bayesian_attractor_t;

// Initialization
void init_bayesian_attractor(bayesian_attractor_t *attractor);

// 1. Actualización de la Covarianza (Manual)
void update_belief(bayesian_attractor_t *attractor, int32_t input_phase, int32_t input_entropy);

// 2. Cálculo de Distancia de Mahalanobis (Al cuadrado)
// Retorna D^2. Si D^2 > Umbral, es una anomalía.
int32_t calculate_mahalanobis_sq(bayesian_attractor_t *attractor, int32_t input_phase, int32_t input_entropy);

#endif // QCORE_BAYES_H
