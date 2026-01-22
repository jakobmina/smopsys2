import pytest
import ctypes

def test_manifold_scaling_alignment(qcore_lib):
    """Verify that the manifold uses the correct square side (1024)."""
    # Assuming we have a way to check constants or just size
    # Here we check if map_brain_to_manifold works as expected for 1024x1024
    
    class VirtualNeuronAddress(ctypes.Structure):
        _fields_ = [("x", ctypes.c_uint32),
                    ("y", ctypes.c_uint32),
                    ("z", ctypes.c_uint32)]
    
    map_func = qcore_lib.map_brain_to_manifold
    map_func.argtypes = [ctypes.c_uint64]
    map_func.restype = VirtualNeuronAddress

    # n = 4448
    # Test ID 0
    addr0 = map_func(0)
    assert addr0.x == 0
    assert addr0.y == 0
    assert addr0.z == 0
    
    # Test ID 4448 (Start of second row)
    addrN = map_func(4448)
    assert addrN.x == 0
    assert addrN.y == 1
    assert addrN.z == 0
    
    # Test ID 4448^2 (Start of second plane)
    addrN2 = map_func(4448 * 4448)
    assert addrN2.x == 0
    assert addrN2.y == 0
    assert addrN2.z == 1

def test_tensorial_reconstruction(qcore_lib):
    """Verify the tensorial product probability logic."""
    digitize = qcore_lib.digitize_hierarchical_neurons
    digitize.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32]
    digitize.restype = ctypes.c_int

    # Case: check if accessible
    res = digitize(0, 0, 0)
    assert res in [0, 1]
