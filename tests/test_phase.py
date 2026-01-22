import pytest
import ctypes

def test_phase_correction_cycle(qcore_lib):
    """Verify that the phase monitor triggers a correction every 7 cycles."""
    class PhaseState(ctypes.Structure):
        _fields_ = [("accumulator", ctypes.c_int32),
                    ("cycle_count", ctypes.c_uint32),
                    ("total_corrections", ctypes.c_uint32)]
    
    state = PhaseState(0, 0, 0)
    
    qcore_lib.phase_init(ctypes.byref(state))
    update_func = qcore_lib.phase_update_pentagonal
    update_func.argtypes = [ctypes.POINTER(PhaseState), ctypes.c_int32]
    update_func.restype = ctypes.c_int

    # Innovation of 0.2 (Q16.16: 13107) to avoid hitting threshold before cycle 7
    innovation = 13107
    
    for i in range(1, 7):
        corrected = update_func(ctypes.byref(state), innovation)
        assert corrected == 0
    
    # 7th cycle should correct
    corrected = update_func(ctypes.byref(state), innovation)
    assert corrected == 1
    assert state.cycle_count == 7
    assert state.total_corrections == 1

def test_phase_residue_stability(qcore_lib):
    """Verify that the phase stays within laminar limits."""
    class PhaseState(ctypes.Structure):
        _fields_ = [("accumulator", ctypes.c_int32),
                    ("cycle_count", ctypes.c_uint32),
                    ("total_corrections", ctypes.c_uint32)]
    
    state = PhaseState(0, 0, 0)
    qcore_lib.phase_init(ctypes.byref(state))
    
    is_laminar = qcore_lib.phase_is_laminar
    is_laminar.argtypes = [ctypes.POINTER(PhaseState)]
    is_laminar.restype = ctypes.c_int
    
    update_func = qcore_lib.phase_update_pentagonal
    update_func.argtypes = [ctypes.POINTER(PhaseState), ctypes.c_int32]
    
    assert is_laminar(ctypes.byref(state)) == 1
    
    # Mock some cycles with the Golden Operator logic
    # After many cycles, it should still be laminar due to corrections
    for i in range(100):
        update_func(ctypes.byref(state), 32768) # 0.5 rad
    
    assert is_laminar(ctypes.byref(state)) == 1
    assert state.total_corrections > 0
