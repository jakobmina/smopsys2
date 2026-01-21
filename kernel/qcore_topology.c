#include "../include/qcore_topology.h"

// Define basic math functions for freestanding environment if not linked
// In a real bare-metal RISC-V, these might be provided by a slim libm
// or we'd move everything to fixed-point.
// For now, we assume the environment can handle double or we implement placeholders.

#include <math.h> // We'll see if the cross-compiler finds this

// Determina la jerarquía basada en la señal espacial
const char* get_hierarchy_layer(double spatial_sig) {
    if (spatial_sig > 0.7) return "NUCLEO (Core)";
    if (spatial_sig > 0.0) return "MANTO (Mantle)";
    return "CORTEZA (Crust)";
}

// CREACIÓN: Aplica el Operador Aureo y la Jerarquía del 7
NeuronIdentity create_neuron(int n) {
    NeuronIdentity neuron;
    neuron.moment_n = n;

    // 1. Componente Temporal (Peso Aureo)
    // phase = n^PHI * PI
    double phase = pow((double)n, TOPOLOGY_PHI) * TOPOLOGY_PI;
    // value = sin(phase) * (-1)^n
    double parity = (n % 2 == 0) ? 1.0 : -1.0;
    neuron.weight_value = sin(phase) * parity;

    // 2. Componente Espacial (Ciclo 7)
    neuron.spatial_id = cos(SPATIAL_CYCLE * (double)n);
    neuron.layer = get_hierarchy_layer(neuron.spatial_id);

    return neuron;
}

// SOLENOIDE: Genera la Matriz de Wrapeo (Coordenadas Conjugadas)
WrappedMatrix process_matrix_wrapping(NeuronIdentity* neurons) {
    double a = neurons[0].spatial_id; // n=1
    double b = neurons[1].spatial_id; // n=2
    double c = neurons[2].spatial_id; // n=3
    double d = neurons[3].spatial_id; // n=4
    double e = neurons[4].spatial_id; // n=5
    double f = neurons[5].spatial_id; // n=6

    double n1=1.0, n2=2.0, n3=3.0, n4=4.0, n5=5.0, n6=6.0;

    WrappedMatrix m;

    m.g = (n1 * a) - (n6 * f); 
    m.h = (n2 * b) - (n5 * e); 
    m.i = (n3 * c) - (n4 * d); 

    m.j = (n6 * f) - (n1 * a);
    m.k = (n5 * e) - (n2 * b);
    m.z = (n4 * d) - (n3 * c); 

    return m;
}

// TENSOR: Calcula Mu_Phi (Viscosidad Aurea)
double calculate_mu_phi(WrappedMatrix m) {
    double g_abs = (m.g < 0) ? -m.g : m.g;
    double h_abs = (m.h < 0) ? -m.h : m.h;
    double i_abs = (m.i < 0) ? -m.i : m.i;
    double tension_sum = g_abs + h_abs + i_abs;
    return (tension_sum / 3.0) / TOPOLOGY_PHI;
}

// PROYECCIÓN: Tu "Atan3" (Conversión a Esféricas con Cosenos Directores)
SphericalProjection calculate_spherical(WrappedMatrix m) {
    SphericalProjection p;
    
    double x = m.g;
    double y = m.h;
    double z = m.i;

    p.magnitude_r = sqrt(x*x + y*y + z*z);

    if (p.magnitude_r < 1e-9) {
        p.cos_alpha = 0; p.cos_beta = 0; p.cos_gamma = 0;
        p.azimuth = 0; p.elevation = 0;
    } else {
        p.cos_alpha = x / p.magnitude_r;
        p.cos_beta  = y / p.magnitude_r;
        p.cos_gamma = z / p.magnitude_r;

        p.azimuth = atan2(y, x);
        
        double plane_r = sqrt(x*x + y*y);
        p.elevation = atan2(z, plane_r);
    }

    return p;
}

// RULE 3.1: Metriplectic Mandate
LagrangianState topology_compute_lagrangian(WrappedMatrix m) {
    LagrangianState state;
    
    // L_symp: Conservative component (Magnitude of the system)
    double r_sq = m.g*m.g + m.h*m.h + m.i*m.i;
    state.L_symp = (fixed_t)(r_sq * 65536.0); // Convert to Q16.16

    // L_metr: Dissipative component (Viscosity)
    double mu = calculate_mu_phi(m);
    state.L_metr = (fixed_t)(-mu * 65536.0); // Dissipation is negative entropy production

    return state;
}
