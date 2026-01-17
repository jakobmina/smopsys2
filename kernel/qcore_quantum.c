#include "../include/qcore_quantum.h"

static QuantumRegister q_reg;

// Helper: fast inverse square root or normalization mock


void init_quantum_register(void) {
    for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
        q_reg.states[i].amplitude_real = 0;
        q_reg.states[i].amplitude_imag = 0;
        q_reg.states[i].data = 0;
    }
    q_reg.global_phase = 0;
    q_reg.coherence_residue = 0;
}

// "Writes" data by adding it to the superposition
// Instead of overwriting, we add a basis state or enhance amplitude of existing one
void write_superposition(int32_t data) {
    // Simple mock: Find an empty slot or the same slot
    int target_idx = -1;
    
    // Check if data already exists to reinforce it (constructive interference)
    for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
        if (q_reg.states[i].data == data && q_reg.states[i].amplitude_real != 0) {
            target_idx = i;
            break;
        }
    }
    
    // If not found, find empty slot
    if (target_idx == -1) {
        for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
            if (q_reg.states[i].amplitude_real == 0) {
                target_idx = i;
                break;
            }
        }
    }
    
    // If memory full, classic replacement (superposition collapse mock)
    // Or we could use a circular buffer approach. For now, simple overwrite 0.
    if (target_idx == -1) target_idx = 0;

    q_reg.states[target_idx].data = data;
    // Set to Hadamard-like superposition state (equal probability initialization)
    // 1/sqrt(N) roughly 0.125 in Q16.16 for size 64? Just using a standardized visible constant.
    q_reg.states[target_idx].amplitude_real = 0x00002000; // 0.125
    q_reg.states[target_idx].amplitude_imag = 0;

    // Inject "Vacuum Noise" / Leakage to adjacent state (Rule 2.1)
    // This creates the necessary entropy/residue for the wetware
    int noise_idx = (target_idx + 1) % QUANTUM_MEMORY_SIZE;
    if (q_reg.states[noise_idx].amplitude_real == 0) {
        q_reg.states[noise_idx].data = 0; // "Empty" state
        q_reg.states[noise_idx].amplitude_real = 0x00000200; // Small leak (~1/16th of signal)
    }
}

void apply_grover_oracle(int32_t target) {
    // Flip phase of the target state
    for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
        // Simple equality check for "Oracle"
        // In simulation, we can peek at the data. 
        if (q_reg.states[i].data == target) {
            q_reg.states[i].amplitude_real = -q_reg.states[i].amplitude_real;
            q_reg.states[i].amplitude_imag = -q_reg.states[i].amplitude_imag;
        }
    }
}

void apply_diffusion_operator(void) {
    // Inversion about the mean
    fixed_t sum = 0;
    int count = 0;
    
    for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
        // Only count active states
        if (q_reg.states[i].amplitude_real != 0) {
            sum += q_reg.states[i].amplitude_real;
            count++;
        }
    }
    
    if (count == 0) return;
    
    fixed_t mean = sum / count;
    
    for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
        if (q_reg.states[i].amplitude_real != 0) {
            // 2*mean - amplitude
            q_reg.states[i].amplitude_real = (2 * mean) - q_reg.states[i].amplitude_real;
        }
    }
}

// Simulates Grover search to find closest match or exact match
int32_t read_quantum_register(int32_t search_target) {
    // 1. superposition is already active
    
    // 2. Apply Grover iterations (simulating sqrt(N) steps)
    // For N=64, approx 6-8 iterations
    for (int step = 0; step < 6; step++) {
        apply_grover_oracle(search_target);
        apply_diffusion_operator();
    }
    
    // 3. Measure (Collapse) - find state with highest amplitude
    int32_t max_amplitude = -1;
    int32_t best_match_data = 0;
    
    // Also calculate coherence residue (sum of off-diagonal/unused probabilities entropy)
    fixed_t total_prob = 0;
    
    for (int i = 0; i < QUANTUM_MEMORY_SIZE; i++) {
        fixed_t amp = q_reg.states[i].amplitude_real;
        // abs value approximation
        if (amp < 0) amp = -amp;
        
        total_prob += amp;
        
        if (amp > max_amplitude) {
            max_amplitude = amp;
            best_match_data = q_reg.states[i].data;
        }
    }
    
    // Residue is essentially the "waste" energy or non-collapsed probability
    // Used to feed the wetware. Simple metric: Total Prob - Max Prob.
    q_reg.coherence_residue = total_prob - max_amplitude;
    
    return best_match_data;
}

fixed_t get_quantum_residue(void) {
    return q_reg.coherence_residue;
}
