import pytest
import ctypes

# Security State Enum
LAMINAR_ACTIVE = 0
HIBERNATION = 1
SINGULARITY = 2

# Thresholds
ANOMALY_THRESHOLD = 0x00060000
CRITICAL_BREACH_LEVEL = 0x00100000

# Majorana Byte Union
class MajoranaByte(ctypes.Union):
    _fields_ = [("raw", ctypes.c_uint8)]

def test_phase_encryption_reversibility(qcore_lib):
    """
    Test that phase encryption is reversible (XOR property).
    """
    encrypt = qcore_lib.apply_phase_encryption
    encrypt.argtypes = [ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t, MajoranaByte]
    
    # Create test buffer
    buffer_size = 10
    buffer = (ctypes.c_uint32 * buffer_size)()
    for i in range(buffer_size):
        buffer[i] = 0xDEADBEEF + i
    
    # Store original values
    original = [buffer[i] for i in range(buffer_size)]
    
    # Create quantum state
    q_state = MajoranaByte()
    q_state.raw = 0x42  # Arbitrary phase
    
    # Encrypt
    encrypt(buffer, buffer_size, q_state)
    
    # Verify data changed
    encrypted = [buffer[i] for i in range(buffer_size)]
    assert encrypted != original
    
    # Decrypt (apply same operation)
    encrypt(buffer, buffer_size, q_state)
    
    # Verify data restored
    decrypted = [buffer[i] for i in range(buffer_size)]
    assert decrypted == original

def test_security_state_transitions(qcore_lib):
    """
    Test security state transitions based on surprise levels.
    """
    heartbeat = qcore_lib.security_heartbeat
    get_state = qcore_lib.get_security_state
    
    heartbeat.argtypes = [ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t, ctypes.c_int32, MajoranaByte]
    get_state.restype = ctypes.c_int
    
    # Create test buffer
    buffer_size = 10
    buffer = (ctypes.c_uint32 * buffer_size)()
    for i in range(buffer_size):
        buffer[i] = 0xCAFEBABE + i
    
    q_state = MajoranaByte()
    q_state.raw = 0x55
    
    # Test 1: Low surprise -> Should stay LAMINAR_ACTIVE
    low_surprise = 0x00001000
    heartbeat(buffer, buffer_size, low_surprise, q_state)
    state = get_state()
    assert state == LAMINAR_ACTIVE
    
    # Test 2: Medium surprise -> Should enter HIBERNATION
    medium_surprise = ANOMALY_THRESHOLD + 0x1000
    heartbeat(buffer, buffer_size, medium_surprise, q_state)
    state = get_state()
    assert state == HIBERNATION
    
    # Test 3: Low surprise again -> Should return to LAMINAR_ACTIVE (condensation)
    heartbeat(buffer, buffer_size, low_surprise, q_state)
    state = get_state()
    assert state == LAMINAR_ACTIVE

def test_forensic_log_write(qcore_lib):
    """
    Test that forensic log function exists and can be called.
    Note: In test environment, we can't verify actual memory writes to FORENSIC_MEM_ADDR
    as it's a hardware-specific address. We just ensure the function doesn't crash
    when called with valid parameters.
    """
    # Check if function exists
    if not hasattr(qcore_lib, 'write_forensic_log'):
        pytest.skip("write_forensic_log not exported in test library")
    
    write_log = qcore_lib.write_forensic_log
    write_log.argtypes = [ctypes.c_int32, MajoranaByte]
    write_log.restype = None
    
    q_state = MajoranaByte()
    q_state.raw = 0x7F
    surprise = 0x00080000
    
    # In test environment (QCORE_TEST_ENV), the forensic log write
    # should be safe as it writes to mock_mmio_buffer
    # We just verify the function can be called without crashing
    try:
        write_log(surprise, q_state)
        # If we reach here without exception, the function executed
        assert True
    except Exception as e:
        pytest.fail(f"write_forensic_log raised exception: {e}")
