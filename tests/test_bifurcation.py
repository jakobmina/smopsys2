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

# Majorana Byte Union
class MajoranaByte(ctypes.Union):
    _fields_ = [("raw", ctypes.c_uint8)]
    # Python ctypes doesn't support bitfields natively well, 
    # so we'll manipulate 'raw' for verification.

# Constants
CMD_PREPARE   = 0x04
STATUS_READY   = 0x01
STATUS_DATA_READY = 0x04
MAGIC_VAL = 0x51425247
STATUS_TEMP_OK = 0x02

def test_scheduler_phase_generation(qcore_lib):
    """
    Test that the scheduler returns valid phase trajectories (0-127).
    """
    get_phase = qcore_lib.metriplectic_scheduler_get_next_phase
    get_phase.restype = ctypes.c_uint8
    
    for i in range(100):
        phase = get_phase()
        assert 0 <= phase <= 127

def test_bridge_sync_cycle(qcore_lib):
    """
    Test bridge_tick_sync with majorana byte structure.
    """
    mock_buffer = ctypes.c_uint8.in_dll(qcore_lib, "mock_mmio_buffer")
    mmio = ctypes.cast(ctypes.byref(mock_buffer), ctypes.POINTER(QuantumPort)).contents

    # Force Init State
    mmio.MAGIC_SIG = MAGIC_VAL
    mmio.STATUS_REG = STATUS_TEMP_OK 
    
    # Define the sync function argtypes
    # It takes a pointer to majorana_byte_t
    bridge_sync = qcore_lib.bridge_tick_sync
    bridge_sync.argtypes = [ctypes.POINTER(MajoranaByte)]
    
    cycle_data = MajoranaByte()
    cycle_data.raw = 0x2A # Phase: 42 (00101010) -> Bits 0-6: 42. Bit 7: 0.
    
    # Simulator Thread
    def qpu_collapse_sim():
        time.sleep(0.05)
        if mmio.CONTROL_REG == CMD_PREPARE:
            # Check what was written (should be 0x2A masked to 0x7F)
            # Simulate a collapse outcome:
            # Let's flip the Majorana Bit (Bit 7, 0x80)
            # And keep phase same 0x2A. Result: 0xAA.
            mmio.DATA_LATCH = 0xAA
            mmio.STATUS_REG |= STATUS_DATA_READY

    sim_thread = threading.Thread(target=qpu_collapse_sim)
    sim_thread.start()

    # Call blocking sync
    bridge_sync(ctypes.byref(cycle_data))
    
    sim_thread.join()
    
    # The function should have updated the structure in place.
    # We expect 0xAA
    assert cycle_data.raw == 0xAA
