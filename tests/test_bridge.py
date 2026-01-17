import pytest
import ctypes
import time
import threading

# Define Hardware Register Layout
class QuantumPort(ctypes.Structure):
    _fields_ = [("MAGIC_SIG", ctypes.c_uint32),
                ("CONTROL_REG", ctypes.c_uint32),
                ("STATUS_REG", ctypes.c_uint32),
                ("DATA_LATCH", ctypes.c_uint32),
                ("PHASE_LOCK", ctypes.c_uint32)]

# MMIO Constants
CMD_CALIBRATE = 0x02
CMD_PREPARE_STATE = 0x04  # Fixed: was CMD_PREPARE
STATUS_TEMP_OK = 0x02
STATUS_READY   = 0x01
STATUS_DATA_READY = 0x04
MAGIC_VAL = 0x51425247

def test_bridge_handshake(qcore_lib):
    """
    Test qport_handshake()
    Requires setting up the mock memory with correct signature and simulating hardware response.
    """
    # Access the exported mock buffer
    mock_buffer = ctypes.c_uint8.in_dll(qcore_lib, "mock_mmio_buffer")
    mmio = ctypes.cast(ctypes.byref(mock_buffer), ctypes.POINTER(QuantumPort)).contents
    
    # 1. Setup Hardware State (Magic Sig)
    mmio.MAGIC_SIG = MAGIC_VAL
    mmio.STATUS_REG = 0 # Initially cold
    
    # Run Init in a thread because it blocks waiting for Temp OK
    handshake_complete = threading.Event()
    
    def hardware_simulator():
        time.sleep(0.05)  # Reduced delay
        # Check if Command received
        if mmio.CONTROL_REG == CMD_CALIBRATE:
            # Simulate cooling down
            mmio.STATUS_REG |= STATUS_TEMP_OK
            mmio.PHASE_LOCK = 123456
    
    def run_handshake():
        qcore_lib.qport_handshake()  # Fixed function name
        handshake_complete.set()
    
    sim_thread = threading.Thread(target=hardware_simulator)
    handshake_thread = threading.Thread(target=run_handshake)
    
    sim_thread.start()
    handshake_thread.start()
    
    # Wait with timeout
    if not handshake_complete.wait(timeout=1.0):
        pytest.fail("Handshake timed out")
    
    sim_thread.join(timeout=1.0)
    handshake_thread.join(timeout=1.0)
    
    # Assertions
    assert mmio.CONTROL_REG == CMD_CALIBRATE
    # If function returned, it means it successfully passed the Temp OK check

def test_bridge_tick_cycle(qcore_lib):
    """
    Test bridge_tick_sync()
    """
    # Define majorana_byte_t structure
    class MajoranaByte(ctypes.Union):
        _fields_ = [("raw", ctypes.c_uint8)]
    
    mock_buffer = ctypes.c_uint8.in_dll(qcore_lib, "mock_mmio_buffer")
    mmio = ctypes.cast(ctypes.byref(mock_buffer), ctypes.POINTER(QuantumPort)).contents

    # Ensure init passed first or just force state
    mmio.MAGIC_SIG = MAGIC_VAL
    mmio.STATUS_REG = STATUS_TEMP_OK 
    mmio.CONTROL_REG = 0  # Reset control
    mmio.DATA_LATCH = 0
    
    # Simulate QPU Coherence cycle
    tick_complete = threading.Event()
    qpu_ready = threading.Event()
    
    def qpu_simulator():
        qpu_ready.set()  # Signal that QPU is ready
        # Wait for PREPARE command with longer timeout
        for i in range(200):  # Increased timeout
            if mmio.CONTROL_REG == CMD_PREPARE_STATE:
                # Small delay to simulate quantum collapse
                time.sleep(0.01)
                # Collapse!
                mmio.DATA_LATCH = 0xDEADBEEF 
                mmio.STATUS_REG |= STATUS_DATA_READY
                break
            time.sleep(0.005)  # Increased poll interval

    def run_tick(q_cycle):
        try:
            # Set proper argument types
            qcore_lib.bridge_tick_sync.argtypes = [ctypes.POINTER(MajoranaByte)]
            qcore_lib.bridge_tick_sync.restype = None
            qcore_lib.bridge_tick_sync(ctypes.byref(q_cycle))
            tick_complete.set()
        except Exception as e:
            print(f"bridge_tick_sync exception: {e}")
    
    # Create quantum cycle
    q_cycle = MajoranaByte()
    q_cycle.raw = 0x55  # Input seed
    
    sim_thread = threading.Thread(target=qpu_simulator, daemon=True)
    tick_thread = threading.Thread(target=run_tick, args=(q_cycle,), daemon=True)
    
    # Start QPU simulator first
    sim_thread.start()
    
    # Wait for QPU to be ready
    if not qpu_ready.wait(timeout=0.5):
        pytest.fail("QPU simulator failed to start")
    
    # Small delay to ensure QPU is polling
    time.sleep(0.01)
    
    # Now start the tick
    tick_thread.start()
    
    # Wait with longer timeout
    if not tick_complete.wait(timeout=2.0):
        pytest.fail(f"Bridge tick timed out. CONTROL_REG={mmio.CONTROL_REG:#x}, STATUS_REG={mmio.STATUS_REG:#x}")
    
    sim_thread.join(timeout=1.0)
    tick_thread.join(timeout=1.0)
    
    # Verify result - the QPU should have written back to the cycle
    # We expect the full byte from DATA_LATCH
    assert q_cycle.raw == 0xEF, f"Expected 0xEF, got {q_cycle.raw:#x}"
