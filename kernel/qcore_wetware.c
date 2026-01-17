#include "../include/bios_interface.h"

// Simulated state of the biological network
static IonChannelState neuron_grid[MEA_GRID_SIZE];
static fixed_t network_global_entropy = 0;

void stimulate_biological_layer(fixed_t quantum_residue) {
    // Distribute the quantum residue (entropy) across the electrode grid.
    // This simulates the "Principle of Free Energy" where the network minimizes surprise.
    
    network_global_entropy = quantum_residue;
    
    // Simple diffusion model: residue increases K+ (relaxation) or Na+ (excitation)
    // depending on the phase.
    
    int seed = fixed_to_int(quantum_residue) % MEA_GRID_SIZE;
    if (seed < 0) seed = -seed;
    
    for (int i = 0; i < MEA_GRID_SIZE; i++) {
        // Mock dynamic update
        // If residue is high -> Excitation (Sodium influx)
        if (quantum_residue > 0x00008000) {
            neuron_grid[i].na_occupancy += 0x00001000;
            if (neuron_grid[i].na_occupancy > 0x00010000) neuron_grid[i].na_occupancy = 0x00010000;
        } else {
            // Relaxation (Potassium outflux simulation)
            neuron_grid[i].k_occupancy += 0x00000500;
             if (neuron_grid[i].k_occupancy > 0x00010000) neuron_grid[i].k_occupancy = 0x00010000;
        }
        
        // Decay (Dissipative term)
        neuron_grid[i].na_occupancy -= 0x00000010;
        if (neuron_grid[i].na_occupancy < 0) neuron_grid[i].na_occupancy = 0;
        
        // Write to hardware (simulated)
        write_mea_electrode(i, fixed_to_int(neuron_grid[i].na_occupancy));
    }
}

uint32_t encode_as_spikes(fixed_t value) {
    // Convert a fixed point value to a spike train frequency or count
    // Simple mock: value * 100
    return (uint32_t)(value >> 10); 
}

void write_mea_electrode(int electrode_index, int value) {
    // In bare metal, this would write to a memory mapped IO address
    // e.g., MEA_BASE_ADDR[electrode_index] = value;
    // For now, we update our internal simulation state if it differs
    // (Already updated in stimulate loop, but this represents the physical driver call)
    (void)electrode_index;
    (void)value;
}

fixed_t get_internal_coherence(void) {
    // Returns the "health" or "coherence" of the wetware
    // Low entropy = High Coherence
    return 0x00010000 - network_global_entropy;
}

IonChannelState read_ion_state(int electrode_index) {
    if (electrode_index >= 0 && electrode_index < MEA_GRID_SIZE) {
        return neuron_grid[electrode_index];
    }
    IonChannelState empty = {0, 0};
    return empty;
}
