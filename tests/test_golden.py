import pytest
import ctypes

# Struct definition to match C
class LagrangianState(ctypes.Structure):
    _fields_ = [("L_symp", ctypes.c_int32),
                ("L_metr", ctypes.c_int32)]

def test_golden_operator_sign(qcore_lib, to_fixed, from_fixed):
    """
    Test that the Golden Operator returns values in expected range
    and parity flipping works.
    """
    calc_golden = qcore_lib.calculate_golden_operator
    calc_golden.argtypes = [ctypes.c_int32]
    calc_golden.restype = ctypes.c_int32

    # Test n=0
    # Parity = 1, Phase = 0, Cos(0) = 1. Result should be roughly 1.0
    res_0 = from_fixed(calc_golden(0))
    print(f"n=0, O_n={res_0}")
    assert abs(res_0 - 1.0) < 0.01

    # Test n=1
    # Parity = -1. Result should be negative
    res_1 = from_fixed(calc_golden(1))
    print(f"n=1, O_n={res_1}")
    assert res_1 < 0

def test_lagrangian_computation_rule_3_1(qcore_lib, to_fixed):
    """
    Rule 3.1: Explicit Lagrangian Computation
    Verify compute_lagrangian returns separate L_symp and L_metr
    """
    compute_lag = qcore_lib.compute_lagrangian
    compute_lag.argtypes = [ctypes.c_int32, ctypes.c_int32]
    compute_lag.restype = LagrangianState

    u = to_fixed(1.0)
    v = to_fixed(1.0)
    
    state = compute_lag(u, v)
    
    # Check that we got non-zero return values
    # Based on implementation: L_symp = 1/2 v^2 + 1/2 u^2 = 0.5 + 0.5 = 1.0
    # L_metr = -1/2 v^2 = -0.5
    
    # We convert back to int to check the raw fixed point values roughly
    assert state.L_symp > 0
    assert state.L_metr < 0
