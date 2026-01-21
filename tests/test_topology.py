import pytest
import ctypes
import math

# Struct definitions to match C
class NeuronIdentity(ctypes.Structure):
    _fields_ = [("moment_n", ctypes.c_int),
                ("weight_value", ctypes.c_double),
                ("spatial_id", ctypes.c_double),
                ("layer", ctypes.c_char_p)]

class WrappedMatrix(ctypes.Structure):
    _fields_ = [("g", ctypes.c_double),
                ("h", ctypes.c_double),
                ("i", ctypes.c_double),
                ("j", ctypes.c_double),
                ("k", ctypes.c_double),
                ("z", ctypes.c_double)]

class SphericalProjection(ctypes.Structure):
    _fields_ = [("magnitude_r", ctypes.c_double),
                ("cos_alpha", ctypes.c_double),
                ("cos_beta", ctypes.c_double),
                ("cos_gamma", ctypes.c_double),
                ("azimuth", ctypes.c_double),
                ("elevation", ctypes.c_double)]

class LagrangianState(ctypes.Structure):
    _fields_ = [("L_symp", ctypes.c_int32),
                ("L_metr", ctypes.c_int32)]

def test_neuron_hierarchy(qcore_lib):
    """Verify that neurons are assigned correct hierarchy layers."""
    get_layer = qcore_lib.get_hierarchy_layer
    get_layer.argtypes = [ctypes.c_double]
    get_layer.restype = ctypes.c_char_p

    assert get_layer(0.8) == b"NUCLEO (Core)"
    assert get_layer(0.5) == b"MANTO (Mantle)"
    assert get_layer(-0.1) == b"CORTEZA (Crust)"

def test_create_neuron(qcore_lib):
    """Verify create_neuron calculation logic."""
    create_neuron = qcore_lib.create_neuron
    create_neuron.argtypes = [ctypes.c_int]
    create_neuron.restype = NeuronIdentity

    # Moment n=1
    # spatial_id = cos(7*1)
    neuron_1 = create_neuron(1)
    assert neuron_1.moment_n == 1
    assert abs(neuron_1.spatial_id - math.cos(7.0)) < 1e-7

def test_matrix_wrapping(qcore_lib):
    """Verify matrix wrapping conjugate pairs."""
    create_neuron = qcore_lib.create_neuron
    create_neuron.argtypes = [ctypes.c_int]
    create_neuron.restype = NeuronIdentity

    process_matrix = qcore_lib.process_matrix_wrapping
    process_matrix.argtypes = [ctypes.POINTER(NeuronIdentity)]
    process_matrix.restype = WrappedMatrix

    neurons = (NeuronIdentity * 6)()
    for i in range(6):
        neurons[i] = create_neuron(i + 1)

    matrix = process_matrix(neurons)
    
    # g = (n1*a) - (n6*f) = 1*cos(7*1) - 6*cos(7*6)
    expected_g = 1.0 * math.cos(7.0 * 1.0) - 6.0 * math.cos(7.0 * 6.0)
    assert abs(matrix.g - expected_g) < 1e-7
    assert abs(matrix.g + matrix.j) < 1e-7 # g = -j

def test_topology_lagrangian_rule_3_1(qcore_lib):
    """
    Rule 3.1: Verify metriplectic lagrangian for topology.
    L_symp should be > 0 (Energy) and L_metr should be < 0 (Dissipation/Entropy marker).
    """
    process_matrix = qcore_lib.process_matrix_wrapping
    process_matrix.restype = WrappedMatrix
    
    compute_topo_lag = qcore_lib.topology_compute_lagrangian
    compute_topo_lag.argtypes = [WrappedMatrix]
    compute_topo_lag.restype = LagrangianState

    # Create dummy matrix
    m = WrappedMatrix(g=1.0, h=1.0, i=1.0, j=-1.0, k=-1.0, z=-1.0)
    
    state = compute_topo_lag(m)
    
    assert state.L_symp > 0
    assert state.L_metr < 0
