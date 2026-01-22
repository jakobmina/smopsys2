#include "../include/qcore_hierarchy.h"

// Mapeo lineal de ID Global a Coordenadas (x, y, d)
VirtualNeuronAddress map_brain_to_manifold(uint64_t brain_neuron_id) {
    VirtualNeuronAddress addr;
    
    // Mapeo Cúbico: ID = x + y*n + z*n^2
    uint64_t n = TENSOR_BASE_N;
    addr.x = (uint32_t)(brain_neuron_id % n);
    addr.y = (uint32_t)((brain_neuron_id / n) % n);
    addr.z = (uint32_t)((brain_neuron_id / (n * n)) % n);
    
    return addr;
}

// Digitaliza el estado mediante reconstrucción tensorial
int digitize_hierarchical_neurons(uint32_t x, uint32_t y, uint32_t z) {
    if (x >= TENSOR_BASE_N || y >= TENSOR_BASE_N || z >= TENSOR_BASE_N) return 0;
    
    // El campo de probabilidad virtual se proyecta desde los vectores ortogonales
    // P(x,y,z) = Px * Py * Pz
    float prob_x = pim_tensor_x[x].probability;
    float prob_y = pim_tensor_y[y].probability;
    float prob_z = pim_tensor_z[z].probability;
    
    float total_prob = prob_x * prob_y * prob_z;
    
    // Umbral de disparo: 0.5 (Secreto industrial: ajustado para balance metripléctico)
    return (total_prob > 0.5f) ? 1 : 0;
}
