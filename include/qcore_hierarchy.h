#ifndef QCORE_HIERARCHY_H
#define QCORE_HIERARCHY_H

#include <stdint.h>
#include "qcore_pim.h"

// Direccionamiento Virtual a Fisico
// x, y: Coordenadas en el Manifold Cuadrado (0-1023)
// branch_id: Indice de la rama probabilistica (0-87)
typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} VirtualNeuronAddress;

// Mapea un ID unico (0 a 88G) a una direccion fisica cubica (x, y, z)
VirtualNeuronAddress map_brain_to_manifold(uint64_t brain_neuron_id);

// Digitaliza un estado basado en el producto tensorial de las celdas fisicas
int digitize_hierarchical_neurons(uint32_t x, uint32_t y, uint32_t z);

#endif // QCORE_HIERARCHY_H
