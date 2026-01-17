 <img width="500" height="300" alt="image" src="https://github.com/user-attachments/assets/5103cd8c-2a2f-45c0-983c-933819dcd6dd"/>

<p aling="center">

# **Smopsys: The Metriplectic Kernel** <img width="30" height="30" alt="image" src="https://github.com/user-attachments/assets/38bfd29d-b3f9-4cd7-be6d-4fe5a121b5e0" />

  <img src="https://img.shields.io/badge/KERNEL-SMOPSYS-cyan?style=for-the-badge&logoWidth=25&logo=LOGO" alt="Smopsys Status"> <img src="https://img.shields.io/badge/SECURITY-METRIPLECTIC_SCORCH-black?style=for-the-badge" alt="Security Level"> <img src="https://img.shields.io/badge/HARDWARE-MMQI_ACTIVE-lightblue?style=for-the-badge" alt="Hardware Bridge">
</p>

**Smopsys** is a bare-metal experimental kernel designed to implement the **Metriplectic Mandate**: a physics-based operating system architecture that rigorously balances conservative (Hamiltonian/Quantum) and dissipative (Entropic/Dissipative) dynamics.

## **Deployment & Execution**

To run the Smopsys2 kernel in simulation mode (QEMU):

```bash
qemu-system-riscv64 -machine virt -nographic -bios none -kernel smopsys.elf
```

*To exit QEMU: Press `Ctrl+A` then `X`.*

## **Core Features**

The system logic is no longer driven by a clock, but by **Quantum Collapse** events.

### The Bifurcation Loop (`kernel/main.c`)

The kernel executes a strict 4-step cycle:

1. **Proposal**: The **Golden Operator** suggests a phase trajectory.
2. **Collapse**: The **MMQI Bridge** halts the CPU until the QPU responds (`majorana_byte_t`).
3. **Judgment**: The **Bayesian Cognitive Layer** calculates the "Surprise" (Mahalanobis Distance).
4. **Reaction**:
    - **Low Surprise**: Update Belief (Learn). Conservative Mode.
    - **High Surprise**: Convert Entropy to Free Energy (Stimulate Wetware). Dissipative Mode.

## Architecture Overview

### 1. MMQI Hardware Interface (`kernel/qcore_bridge.c`)

- **Direct Quantum Coupling**: `0xF8000000`.
- **Deadman Switch**: System halts if QPU signature (`0x51425247`) is missing.

### 2. Cognitive Layer (`kernel/qcore_bayes.c`)

- **Bayesian Attractor**: Tracks system state ($\mu, \Sigma$).
- **Anomaly Detection**: Used to decide between Conservative and Dissipative modes.

### 3. Lindblad Filter (`kernel/qcore_lindblad.c`) **[NEW]**

- **Bosonic-Fermionic Axis**: Oscillates according to Golden Operator $O_n = \cos(\pi n) \cdot \cos(\pi \phi n)$.
- **Visibility Modulation**: Information transitions between visible (Bosonic, $S_n = +1$) and protected (Fermionic, $S_n = -1$) states.
- **Decoherence Rates**: $\Gamma_\pi / \Gamma_\phi = \phi^2 \approx 2.618$ (Golden Ratio squared).
- **Quantum Laundering**: High-entropy information is "washed" into superposition instead of being learned classically.

## Building and Verification

### Prerequisites

- GCC (RISC-V or x86_64)
- Python 3 + Pytest

### Compilation

To build the RISC-V Kernel Image (requires `riscv64-unknown-elf-gcc`):

```bash
make kernel
```

To build the Host Test Library (for simulation/testing):

```bash
make test_lib
```

### Running Tests

The test suite validates the physics logic and mocks the hardware interfaces.

```bash
# Clean and test
make clean
# Build Test Library with Mock Environment
make test_lib
# Run tests
python3 -m pytest tests/
```

### Test Coverage

**Status**: ✅ **23/23 tests passing (100%)**

- `test_bifurcation.py`: Verifies the Majorana Phase Proposal and Bridge Sync.
- `test_bridge.py`: Verifies MMQI Handshake and Tick synchronization.
- `test_bayes.py`: Verifies Bayesian belief update and anomaly detection.
- `test_lindblad.py`: Verifies Bosonic-Fermionic oscillation and visibility modulation.
- `test_security.py`: Verifies phase encryption, state transitions, and forensic logging.
- `test_golden.py`: Verifies Golden Operator and Lagrangian computation.
- `test_storage.py`: Verifies quantum memory and wetware coupling.

### Docker Build

To build and run the complete system in Docker:

```bash
# Build the Docker image
docker build -t smopsys2 .

# Run tests in container
docker run --rm smopsys2

# Interactive shell
docker run --rm -it smopsys2 /bin/bash
```

---
*Developed by QuoreMind & Antigravity for the Smopsys Project.*
