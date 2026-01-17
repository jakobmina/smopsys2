import pytest
import ctypes

def test_scheduler_linkage(qcore_lib):
    """
    Verify the scheduler function exists and runs without crashing.
    Since set_system_mode is internal static state in the mock,
    we are primarily testing that the symbol is exported and callable.
    """
    scheduler = qcore_lib.metriplectic_scheduler
    scheduler.argtypes = [ctypes.c_int32]
    scheduler.restype = None
    
    # Run a few ticks
    for i in range(10):
        scheduler(i)
