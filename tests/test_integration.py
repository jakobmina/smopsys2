import pytest
import ctypes

def test_bio_stimulation(qcore_lib, to_fixed):
    """
    Test the Bio-Interface stimulation logic.
    """
    stimulate = qcore_lib.stimulate_biological_layer
    stimulate.argtypes = [ctypes.c_int32]
    stimulate.restype = None
    
    # Stimulate with some coherence value
    # Just ensure it runs without crashing, as the MEA write is valid in mock memory
    val = to_fixed(0.5)
    stimulate(val)

def test_encode_as_spikes(qcore_lib, to_fixed):
    """
    Test the spike encoding logic.
    """
    encode = qcore_lib.encode_as_spikes
    encode.argtypes = [ctypes.c_int32]
    encode.restype = ctypes.c_uint32
    
    # 1.0 (0x10000) shifted right by 8 is 0x100
    val = to_fixed(1.0) # 0x10000
    res = encode(val)
    
    assert res == 0x40
