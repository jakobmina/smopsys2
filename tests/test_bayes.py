import pytest
import ctypes

# Define structures
class BayesianAttractor(ctypes.Structure):
    _fields_ = [("mu", ctypes.c_int32 * 2),
                ("cov", (ctypes.c_int32 * 2) * 2),
                ("inv_cov", (ctypes.c_int32 * 2) * 2)]

def test_bayesian_update_convergence(qcore_lib, to_fixed):
    """
    Test convergence of the Bayesian Belief (Mean)
    """
    init_bayes = qcore_lib.init_bayesian_attractor
    update = qcore_lib.update_belief
    
    init_bayes.argtypes = [ctypes.POINTER(BayesianAttractor)]
    update.argtypes = [ctypes.POINTER(BayesianAttractor), ctypes.c_int32, ctypes.c_int32]
    
    attractor = BayesianAttractor()
    init_bayes(ctypes.byref(attractor))
    
    # Input constant stream: Phase=0.5, Entropy=0.2
    target_phase = to_fixed(0.5)
    target_entropy = to_fixed(0.2)
    
    # Run update loop
    for _ in range(100):
        update(ctypes.byref(attractor), target_phase, target_entropy)
        
    # Check if Mean converged (approx)
    mu_phase = attractor.mu[0]
    mu_entropy = attractor.mu[1]
    
    # Allow some margin due to EMA lag and fixed point precision
    assert abs(mu_phase - target_phase) < to_fixed(0.01)
    assert abs(mu_entropy - target_entropy) < to_fixed(0.01)

def test_mahalanobis_anomaly(qcore_lib, to_fixed):
    """
    Test Mahalanobis Distance for anomaly detection
    """
    init_bayes = qcore_lib.init_bayesian_attractor
    update = qcore_lib.update_belief
    calc_mahal = qcore_lib.calculate_mahalanobis_sq
    
    init_bayes.argtypes = [ctypes.POINTER(BayesianAttractor)]
    update.argtypes = [ctypes.POINTER(BayesianAttractor), ctypes.c_int32, ctypes.c_int32]
    calc_mahal.argtypes = [ctypes.POINTER(BayesianAttractor), ctypes.c_int32, ctypes.c_int32]
    calc_mahal.restype = ctypes.c_int32
    
    attractor = BayesianAttractor()
    init_bayes(ctypes.byref(attractor))
    
    # Train on "Normal" data (0.5, 0.5)
    normal_val = to_fixed(0.5)
    for _ in range(50):
        update(ctypes.byref(attractor), normal_val, normal_val)
        
    # Measure a "Normal" point
    dist_normal = calc_mahal(ctypes.byref(attractor), normal_val, normal_val)
    
    # Measure an "Anomaly" point (0.9, 0.9)
    anomaly_val = to_fixed(0.9)
    dist_anomaly = calc_mahal(ctypes.byref(attractor), anomaly_val, anomaly_val)
    
    print(f"Normal Dist: {dist_normal}, Anomaly Dist: {dist_anomaly}")
    
    # Anomaly distance should be significantly higher
    assert dist_anomaly > dist_normal
    assert dist_anomaly > to_fixed(1.0) # Heuristic threshold
