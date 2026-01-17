#ifndef BIOS_INTERFACE_H
#define BIOS_INTERFACE_H

#include <stdint.h>
#include "qcore_math.h"

#define MEA_GRID_SIZE 64 // 8x8 electrode array
#define ION_THRESHOLD_NA 0x00008000 // Sodium threshold (0.5)
#define ION_THRESHOLD_K  0x00004000 // Potassium threshold (0.25)

// Interface with the Wetware (BNN via MEA)
// Takes the "waste" entropy from the quantum system to feed the biological network
void stimulate_biological_layer(fixed_t quantum_residue);

// Helper functions for MEA (Micro-Electrode Array)
uint32_t encode_as_spikes(fixed_t value);
void write_mea_electrode(int electrode_index, int value);

// Monitoring
// Returns the collective coherence/entropy of the biological network
fixed_t get_internal_coherence(void);

// Second Quantization specific
typedef struct {
    fixed_t na_occupancy; // Sodium Ion level
    fixed_t k_occupancy;  // Potassium Ion level
} IonChannelState;

IonChannelState read_ion_state(int electrode_index);

#endif // BIOS_INTERFACE_H
