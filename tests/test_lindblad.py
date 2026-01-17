"""
Test Suite for Lindblad Filter (qcore_lindblad.c)

Verifica:
1. Inicialización del estado Bosónico
2. Oscilación del eje BF según el Operador Áureo
3. Modulación de visibilidad por sorpresa
4. Transición Bosónico → Fermiónico
5. Lógica de "laundering" (lavado cuántico)
"""

import ctypes
import pytest


class LindblادState(ctypes.Structure):
    """Estructura de estado del Lindblad Filter"""
    _fields_ = [
        ("bf_axis", ctypes.c_int32),           # fixed_t
        ("visibility_score", ctypes.c_int32),  # fixed_t
        ("cycle_count", ctypes.c_uint32),
        ("gamma_pi", ctypes.c_int32),          # fixed_t
        ("gamma_phi", ctypes.c_int32),         # fixed_t
    ]


def test_lindblad_initialization(qcore_lib):
    """
    Test 1: Verificar inicialización en estado Bosónico
    
    Estado inicial esperado:
    - bf_axis = +1.0 (Bosónico)
    - visibility_score = 1.0 (Completamente visible)
    - gamma_pi / gamma_phi = φ² ≈ 2.618
    """
    lindblad_init = qcore_lib.lindblad_init
    lindblad_init.argtypes = [ctypes.POINTER(LindblادState)]
    lindblad_init.restype = None
    
    state = LindblادState()
    lindblad_init(ctypes.byref(state))
    
    # Verificar estado Bosónico inicial
    ONE_FIXED = 0x00010000  # 1.0 en Q16.16
    
    assert state.bf_axis == ONE_FIXED, f"bf_axis debe ser +1.0, got {state.bf_axis:#x}"
    assert state.visibility_score == ONE_FIXED, f"visibility debe ser 1.0, got {state.visibility_score:#x}"
    assert state.cycle_count == 0, "cycle_count debe iniciar en 0"
    
    # Verificar ratio áureo: Γ_π / Γ_φ = φ² ≈ 2.618
    PHI_SQUARED = 2.618
    ratio = state.gamma_pi / state.gamma_phi
    
    assert abs(ratio - PHI_SQUARED) < 0.1, f"Ratio Γ_π/Γ_φ debe ser ~2.618, got {ratio:.3f}"
    
    print(f"✓ Lindblad inicializado: bf_axis={state.bf_axis:#x}, visibility={state.visibility_score:#x}")
    print(f"✓ Ratio áureo: Γ_π/Γ_φ = {ratio:.3f}")


def test_lindblad_bf_axis_oscillation(qcore_lib):
    """
    Test 2: Verificar oscilación del eje BF según O_n
    
    El eje debe oscilar entre -1 y +1 siguiendo:
    O_n = cos(π n) * cos(π φ n)
    """
    lindblad_init = qcore_lib.lindblad_init
    lindblad_init.argtypes = [ctypes.POINTER(LindblادState)]
    
    lindblad_update = qcore_lib.lindblad_update
    lindblad_update.argtypes = [ctypes.POINTER(LindblادState), ctypes.c_int32, ctypes.c_uint8]
    lindblad_update.restype = None
    
    state = LindblادState()
    lindblad_init(ctypes.byref(state))
    
    # Simular varios ciclos
    bf_values = []
    for cycle in range(10):
        surprise = 0  # Sin sorpresa (coherente)
        majorana_state = 0  # Onda
        
        lindblad_update(ctypes.byref(state), surprise, majorana_state)
        bf_values.append(state.bf_axis)
    
    # Verificar que bf_axis oscila (no es constante)
    unique_values = set(bf_values)
    assert len(unique_values) > 1, "bf_axis debe oscilar, no ser constante"
    
    # Verificar que está acotado en [-1, +1]
    ONE_FIXED = 0x00010000
    for bf in bf_values:
        assert -ONE_FIXED <= bf <= ONE_FIXED, f"bf_axis fuera de rango: {bf:#x}"
    
    print(f"✓ bf_axis oscila correctamente en {len(unique_values)} valores distintos")


def test_lindblad_visibility_modulation(qcore_lib):
    """
    Test 3: Verificar modulación de visibilidad por sorpresa
    
    - Sorpresa alta → Visibilidad baja
    - Sorpresa baja → Visibilidad alta
    """
    lindblad_init = qcore_lib.lindblad_init
    lindblad_init.argtypes = [ctypes.POINTER(LindblادState)]
    
    lindblad_update = qcore_lib.lindblad_update
    lindblad_update.argtypes = [ctypes.POINTER(LindblادState), ctypes.c_int32, ctypes.c_uint8]
    
    lindblad_get_visibility = qcore_lib.lindblad_get_visibility
    lindblad_get_visibility.argtypes = [ctypes.POINTER(LindblادState)]
    lindblad_get_visibility.restype = ctypes.c_int32
    
    # Caso 1: Sorpresa baja (coherente)
    state_low = LindblادState()
    lindblad_init(ctypes.byref(state_low))
    
    LOW_SURPRISE = 0x00010000  # 1.0 en Q16.16
    lindblad_update(ctypes.byref(state_low), LOW_SURPRISE, 0)
    visibility_low = lindblad_get_visibility(ctypes.byref(state_low))
    
    # Caso 2: Sorpresa alta (turbulencia)
    state_high = LindblادState()
    lindblad_init(ctypes.byref(state_high))
    
    HIGH_SURPRISE = 0x00060000  # 6.0 en Q16.16 (MAX_ENTROPY_TOLERANCE)
    lindblad_update(ctypes.byref(state_high), HIGH_SURPRISE, 1)
    visibility_high = lindblad_get_visibility(ctypes.byref(state_high))
    
    # Verificar que alta sorpresa reduce visibilidad
    assert visibility_low > visibility_high, \
        f"Alta sorpresa debe reducir visibilidad: low={visibility_low:#x}, high={visibility_high:#x}"
    
    print(f"✓ Visibilidad modulada: low_surprise={visibility_low:#x}, high_surprise={visibility_high:#x}")


def test_lindblad_fermionic_transition(qcore_lib):
    """
    Test 4: Verificar transición Bosónico → Fermiónico
    
    Con sorpresa sostenida alta, el sistema debe transicionar a Fermiónico
    (visibility_score < 0.3)
    """
    lindblad_init = qcore_lib.lindblad_init
    lindblad_init.argtypes = [ctypes.POINTER(LindblادState)]
    
    lindblad_update = qcore_lib.lindblad_update
    lindblad_update.argtypes = [ctypes.POINTER(LindblادState), ctypes.c_int32, ctypes.c_uint8]
    
    lindblad_should_launder = qcore_lib.lindblad_should_launder
    lindblad_should_launder.argtypes = [ctypes.POINTER(LindblادState)]
    lindblad_should_launder.restype = ctypes.c_int
    
    state = LindblادState()
    lindblad_init(ctypes.byref(state))
    
    # Aplicar sorpresa alta sostenida
    HIGH_SURPRISE = 0x00060000  # 6.0
    for _ in range(5):
        lindblad_update(ctypes.byref(state), HIGH_SURPRISE, 1)  # Colapso a partícula
    
    should_launder = lindblad_should_launder(ctypes.byref(state))
    
    # Después de sorpresa sostenida, debe entrar en modo Fermiónico
    assert should_launder == 1, "Sorpresa sostenida debe activar modo Fermiónico (launder)"
    
    print(f"✓ Transición a Fermiónico: should_launder={should_launder}, visibility={state.visibility_score:#x}")


def test_lindblad_bosonic_stability(qcore_lib):
    """
    Test 5: Verificar estabilidad en modo Bosónico
    
    Con sorpresa baja sostenida, el sistema debe permanecer Bosónico
    """
    lindblad_init = qcore_lib.lindblad_init
    lindblad_init.argtypes = [ctypes.POINTER(LindblادState)]
    
    lindblad_update = qcore_lib.lindblad_update
    lindblad_update.argtypes = [ctypes.POINTER(LindblادState), ctypes.c_int32, ctypes.c_uint8]
    
    lindblad_should_launder = qcore_lib.lindblad_should_launder
    lindblad_should_launder.argtypes = [ctypes.POINTER(LindblادState)]
    lindblad_should_launder.restype = ctypes.c_int
    
    state = LindblادState()
    lindblad_init(ctypes.byref(state))
    
    # Aplicar sorpresa baja sostenida
    LOW_SURPRISE = 0x00008000  # 0.5
    for _ in range(10):
        lindblad_update(ctypes.byref(state), LOW_SURPRISE, 0)  # Coherencia (onda)
    
    should_launder = lindblad_should_launder(ctypes.byref(state))
    
    # Debe permanecer en modo Bosónico (no launder)
    assert should_launder == 0, "Sorpresa baja debe mantener modo Bosónico"
    
    FERMIONIC_THRESHOLD = 0x00004CCC  # 0.3
    assert state.visibility_score > FERMIONIC_THRESHOLD, \
        f"Visibilidad debe estar sobre umbral Fermiónico: {state.visibility_score:#x}"
    
    print(f"✓ Estabilidad Bosónica: should_launder={should_launder}, visibility={state.visibility_score:#x}")


def test_lindblad_metriplectic_compliance(qcore_lib):
    """
    Test 6: Verificar cumplimiento del Mandato Metriplético
    
    El sistema debe tener:
    - Componente conservativa (oscilación BF)
    - Componente disipativa (modulación de visibilidad)
    - Nunca puramente conservativo ni disipativo
    """
    lindblad_init = qcore_lib.lindblad_init
    lindblad_init.argtypes = [ctypes.POINTER(LindblادState)]
    
    lindblad_update = qcore_lib.lindblad_update
    lindblad_update.argtypes = [ctypes.POINTER(LindblادState), ctypes.c_int32, ctypes.c_uint8]
    
    state = LindblادState()
    lindblad_init(ctypes.byref(state))
    
    # Simular ciclos con variación
    bf_changes = 0
    vis_changes = 0
    prev_bf = state.bf_axis
    prev_vis = state.visibility_score
    
    for i in range(20):
        surprise = (i * 0x00010000) // 10  # Sorpresa variable
        majorana = i % 2
        
        lindblad_update(ctypes.byref(state), surprise, majorana)
        
        if state.bf_axis != prev_bf:
            bf_changes += 1
        if state.visibility_score != prev_vis:
            vis_changes += 1
        
        prev_bf = state.bf_axis
        prev_vis = state.visibility_score
    
    # Verificar que ambos componentes son dinámicos
    assert bf_changes > 0, "Componente conservativa (bf_axis) debe oscilar"
    assert vis_changes > 0, "Componente disipativa (visibility) debe modular"
    
    print(f"✓ Cumplimiento Metriplético: bf_changes={bf_changes}, vis_changes={vis_changes}")
