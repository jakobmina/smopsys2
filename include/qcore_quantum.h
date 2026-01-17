#ifndef QCORE_QUANTUM_H
#define QCORE_QUANTUM_H

#include <stdint.h>
#include "qcore_math.h"

// Number of simulated qubits/states for the associative memory
#define QUANTUM_MEMORY_SIZE 64 

// Represents a single quantum state in the superposition
typedef struct {
    fixed_t amplitude_real; // Real component of the wave function
    fixed_t amplitude_imag; // Imaginary component
    int32_t data;           // The actual data value "stored" in this basis state
} QuantumBasisState;

// The full quantum register
typedef struct {
    QuantumBasisState states[QUANTUM_MEMORY_SIZE];
    fixed_t global_phase;
    fixed_t coherence_residue; // For passing to wetware
} QuantumRegister;

// Public API
void init_quantum_register(void);
void write_superposition(int32_t data);
int32_t read_quantum_register(int32_t search_target); // Returns best match via Grover
fixed_t get_quantum_residue(void);

// Internal helper exposed for testing
void apply_grover_oracle(int32_t target);
void apply_diffusion_operator(void);

#endif // QCORE_QUANTUM_H
