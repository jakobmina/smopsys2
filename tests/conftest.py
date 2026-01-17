import pytest
import ctypes
import os

@pytest.fixture(scope="session")
def qcore_lib():
    lib_path = os.path.abspath("libqcore.so")
    if not os.path.exists(lib_path):
        pytest.fail("libqcore.so not found. Run 'make' first.")
    
    lib = ctypes.CDLL(lib_path)
    return lib

@pytest.fixture
def to_fixed():
    def _to_fixed(val):
        return int(val * 65536)
    return _to_fixed

@pytest.fixture
def from_fixed():
    def _from_fixed(val):
        # Handle signed 32-bit integer 
        if val >= 0x80000000:
            val -= 0x100000000
        return val / 65536.0
    return _from_fixed
