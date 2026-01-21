#ifndef QCORE_TOPOLOGY_H
#define QCORE_TOPOLOGY_H

#include <stdint.h>
#include "qcore_math.h"

// --- CONSTANTES FÍSICAS Y GEOMÉTRICAS ---
// We use the double values if float is supported, or adapt to fixed if necessary.
// For now, following the user's snippet structure.
#define TOPOLOGY_PI 3.14159265358979323846
#define TOPOLOGY_PHI 1.618033988749895
#define SPATIAL_CYCLE 7.0

// --- ESTRUCTURAS DE DATOS ---

// La identidad fundamental de la neurona (Momento + Espacio)
typedef struct {
    int moment_n;           // El "driver" entero (Momento Cuántico)
    double weight_value;    // Componente Temporal (Aureo)
    double spatial_id;      // Componente Espacial (Ciclo 7)
    const char* layer;      // Etiqueta de jerarquía (Núcleo, Manto, Corteza)
} NeuronIdentity;

// La Matriz de Wrapeo (El Solenoide Topológico)
typedef struct {
    double g; // Par Externo (1-6) [Eje X]
    double h; // Par Medio (2-5)   [Eje Y]
    double i; // Par Interno (3-4) [Eje Z]
    double j; // Inverso Externo
    double k; // Inverso Medio
    double z; // Inverso Interno (El Fulcrum)
} WrappedMatrix;

// Proyección Esférica (Orientación de Fase)
typedef struct {
    double magnitude_r;
    double cos_alpha; // Director X
    double cos_beta;  // Director Y
    double cos_gamma; // Director Z (Elevación Nuclear)
    double azimuth;   // Theta
    double elevation; // Phi (Atan2+Z)
} SphericalProjection;

// --- FUNCIONES DEL KERNEL ---

const char* get_hierarchy_layer(double spatial_sig);
NeuronIdentity create_neuron(int n);
WrappedMatrix process_matrix_wrapping(NeuronIdentity* neurons);
double calculate_mu_phi(WrappedMatrix m);
SphericalProjection calculate_spherical(WrappedMatrix m);

// Metriplectic Mandate: Lagrangian computation
LagrangianState topology_compute_lagrangian(WrappedMatrix m);

#endif // QCORE_TOPOLOGY_H
