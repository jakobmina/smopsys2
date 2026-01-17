import pytest
import ctypes

def test_quantum_assoc_memory_grover(qcore_lib, to_fixed, from_fixed):
    """
    Test 1: Encapsulamiento en Memoria Asociativa Cuántica
    - Write value to quantum superposition
    - Retrieve using "Grover" (simulated)
    """
    # Setup functions
    init_q = qcore_lib.init_quantum_register
    write_q = qcore_lib.write_superposition
    read_q = qcore_lib.read_quantum_register
    
    write_q.argtypes = [ctypes.c_int32]
    read_q.argtypes = [ctypes.c_int32]
    read_q.restype = ctypes.c_int32
    
    # Initialize
    init_q()
    
    # Target value (e.g. n=42)
    target_value = 42
    write_q(target_value)
    
    # Try to retrieve using Grover search for the same target
    retrieved = read_q(target_value)
    
    assert retrieved == target_value, "Grover retrieval failed to find the superposition state"

def test_wetware_stimulation(qcore_lib, to_fixed):
    """
    Test 2: Almacenamiento en Wetware (Principle of Free Energy)
    Verify that stimulating the biological layer changes the ion channel states.
    """
    stimulate = qcore_lib.stimulate_biological_layer
    read_ion = qcore_lib.read_ion_state
    
    stimulate.argtypes = [ctypes.c_int32]
    
    # Define generic return struct for IonChannelState
    class IonChannelState(ctypes.Structure):
        _fields_ = [("na_occupancy", ctypes.c_int32),
                    ("k_occupancy", ctypes.c_int32)]
    
    read_ion.argtypes = [ctypes.c_int]
    read_ion.restype = IonChannelState
    
    # Stimulate with some "Residue" (Mock value > threshold)
    residue = to_fixed(0.8) 
    stimulate(residue)
    
    # Check electrode 0 (just checking one for effect)
    state = read_ion(0)
    
    # Since residue was 0.8 (high/Positive), it should trigger Sodium (Excitation)
    # The code adds 0x1000 to na_occupancy
    assert state.na_occupancy > 0, "Neuronal excitation (Na+) failed"
    
def test_metriplectic_residue_coupling(qcore_lib, to_fixed):
    """
    Test 4: Ventaja Energética (Coupling)
    Ensure quantum residue is generated during retrieval.
    """
    get_residue = qcore_lib.get_quantum_residue
    get_residue.restype = ctypes.c_int32
    
    # Run a retrieval to generate residue
    qcore_lib.read_quantum_register(42)
    
    residue = get_residue()
    # It should be non-zero as it represents the "Waste" entropy
    assert residue != 0, "No quantum residue (entropy) generated"
