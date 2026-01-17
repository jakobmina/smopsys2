#ifndef QCORE_SCHEDULER_H
#define QCORE_SCHEDULER_H

#include <stdint.h>
#include "qcore_math.h"

// System Modes
typedef enum {
    MODE_BOSONIC,   // Conservative / Energy Pump
    MODE_FERMIONIC  // Dissipative / Entropy Evaporation
} SystemMode;

// Scheduler Control
void metriplectic_scheduler(int32_t tick);
uint8_t metriplectic_scheduler_get_next_phase(void);
fixed_t calculate_golden_operator(int32_t n);

// External Hardware Logic (to be implemented in asm or hardware drivers)
void set_system_mode(SystemMode mode);
void pump_energy(fixed_t rate);

#endif // QCORE_SCHEDULER_H
