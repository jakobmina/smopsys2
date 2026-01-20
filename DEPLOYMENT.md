# DEPLOYMENT.md - Smopsys2 Quantum Kernel Deployment Guide

**Versión**: 2.0 | **Autor**: Jacobo Tlacaelel Mina Rodriguez | **Última Actualización**: Enero 2026

---

## Tabla de Contenidos

1. [Arquitectura de Despliegue](#arquitectura-de-despliegue)
2. [Hardware Soportado](#hardware-soportado)
3. [Entornos de Ejecución](#entornos-de-ejecución)
4. [Pipeline de Build & Deploy](#pipeline-de-build--deploy)
5. [Debugging & Troubleshooting](#debugging--troubleshooting)
6. [Compliance Metripléctico](#compliance-metripléctico)

---

## Arquitectura de Despliegue

### Nivel 1: Isomorfismo Matemático (Topología Abstracta)

```
┌──────────────────────────────────────────────────────────────┐
│                 SMOPSYS2 DEPLOYMENT TOPOLOGY                 │
├──────────────────────────────────────────────────────────────┤
│                                                               │
│  SOURCE CODE (Git)                                           │
│       ↓                                                       │
│  BUILD ARTIFACTS                                             │
│    ├─ smopsys.elf (Bare-Metal Binary)                        │
│    ├─ libqcore.so (Test Library)                             │
│    └─ smopsys2.docker (OCI Image)                            │
│       ↓                                                       │
│  DEPLOYMENT TARGETS                                          │
│    ├─ QEMU virt (Emulation) ← Default Development           │
│    ├─ HiFive Unleashed (RISC-V Hardware)                     │
│    ├─ VisionFive 2 (RISC-V SBC)                              │
│    ├─ Custom RISC-V Platform (u-boot + kernel)              │
│    └─ Kubernetes Cluster (Containerized)                    │
│       ↓                                                       │
│  RUNTIME ENVIRONMENT                                         │
│    ├─ Machine Mode (Bare-Metal) [HIGH CONTROL]              │
│    ├─ Supervisor Mode (u-boot) [MEDIUM CONTROL]             │
│    └─ User Mode (Linux Host) [VIRTUALIZED]                  │
│       ↓                                                       │
│  HARDWARE INTERFACE                                          │
│    ├─ MMIO Bridge (0xF8000000) ← Quantum Port               │
│    ├─ UART (Serial Console)                                  │
│    ├─ MEA Controller (Biological Layer)                      │
│    └─ Security Heartbeat (Real-time Monitoring)             │
│                                                               │
└──────────────────────────────────────────────────────────────┘
```

---

## Hardware Soportado

### Nivel 2: Isomorfismo Dimensional (Especificaciones Físicas)

#### **1. QEMU virt (Simulación - Desarrollo Local)**

| Característica | Especificación |
|---|---|
| **Arquitectura** | RISC-V (RV64IMA) |
| **CPU Simulada** | U54 (5-core) |
| **Memoria** | 128 MB - 4 GB (configurable) |
| **Periféricos** | UART, Timer, CLINT, PLIC |
| **Bootloader** | None (Direct kernel load) |
| **Ventaja** | No requiere hardware físico |
| **Desventaja** | Sin acceso a periféricos reales |

**Comando de Ejecución:**
```bash
qemu-system-riscv64 \
  -machine virt \
  -nographic \
  -serial mon:stdio \
  -bios none \
  -kernel smopsys.elf \
  -m 256M
```

**Salida Esperada:**
```
SMOPSYS2: INITIATING ENERGY RECOVERY...
[ LAMINAR FLOW LOCKED ]
[==================================================] 100%
Entering Bifurcation Loop...
```

---

#### **2. HiFive Unleashed (Hardware Real - Prototipo)**

| Característica | Especificación |
|---|---|
| **SoC** | SiFive FU540-C000 (4-core U54 + 1 E51) |
| **RAM** | 8 GB DDR4 |
| **Storage** | microSD (32 GB recomendado) |
| **Periféricos** | Gigabit Ethernet, PCIe, UART, GPIO |
| **Bootloader** | FSBL + OpenSBI + u-boot |
| **Debugger** | OpenOCD + GDB via JTAG |
| **Precio** | ~$999 (Descontinuado, buscar en eBay) |
| **Estado** | ⚠️ Soportado pero EOL |

**Diagrama de Boot:**
```
┌─────────────────────┐
│  Power On (RISC-V)  │
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  ZSBL (in ROM)      │  ← Factory firmware
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  FSBL (Flash)       │  ← SiFive First Stage Bootloader
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  OpenSBI (Flash)    │  ← SBI Firmware Layer
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  u-boot (Flash)     │  ← Bootloader estándar
└──────────┬──────────┘
           ↓
┌─────────────────────┐
│  Linux Kernel OR    │
│  Smopsys2.elf       │  ← Nuestro kernel bare-metal
└─────────────────────┘
```

**Alternativa: Saltar u-boot (Bare-Metal Directo)**

Si quieres ejecutar Smopsys2 sin Linux de por medio:

1. Reemplazar u-boot en flash con OpenSBI → Smopsys2
2. O usar modo de desarrollo donde OpenSBI cede control a M-mode

```c
// En kernel/entry.S, detectar si estamos en M-mode
#ifdef HIFIVE_UNLEASHED
    // OpenSBI ha establecido S-mode, detectar y switchear a M-mode
    // (Requiere patching de OpenSBI o modo sin bootloader)
#endif
```

---

#### **3. VisionFive 2 (SBC Moderno - Recomendado)**

| Característica | Especificación |
|---|---|
| **SoC** | StarFive JH7110 (4-core U74, 1 S7) |
| **RAM** | 4/8 GB LPDDR4 |
| **Storage** | eMMC 32GB + microSD |
| **Periféricos** | USB 3.0, HDMI, Gigabit Ethernet, PWM |
| **Debugger** | USB-JTAG integrado (xc3s) |
| **Precio** | ~$150-200 |
| **Estado** | ✅ Activo, upstream kernel |
| **Bootloader** | u-boot (standard Linux distro flow) |

**Ventaja sobre HiFive**: Mejor relación costo/rendimiento, soporte comunitario activo.

---

#### **4. Custom RISC-V Platform (Genérico)**

Para **cualquier plataforma RISC-V** que cumpla con:
- ✅ Especificación RISC-V Unprivileged ISA (v1.9 mínimo)
- ✅ Mínimo 64 MB RAM
- ✅ UART para consola serie
- ✅ Bootloader que pueda cargar ELF (u-boot, UEFI, etc.)

**Adaptación Requerida:**

```c
// include/qcore_port.h
#ifdef CUSTOM_PLATFORM
    #define QPORT_BASE_ADDR 0xXXXXXXXX  // Tu dirección MMIO real
    #define UART0_BASE      0xYYYYYYYY  // Tu dirección UART
    #define MEA_BASE_ADDR   0xZZZZZZZZ  // Tu dirección MEA (si aplica)
#endif
```

---

## Entornos de Ejecución

### Nivel 3: Isomorfismo Físico (Principios de Operación Real)

### A. QEMU Virtual (Modo Simulación - Default)

**Flujo Completo:**

```bash
# 1. Compilar kernel
make kernel

# 2. Ejecutar en QEMU
qemu-system-riscv64 -machine virt -nographic -serial mon:stdio -bios none -kernel smopsys.elf

# 3. Interacción desde consola
# El kernel mostrará menú de modo:
# [S] Simulation Mode (recomendado)
# [H] Hardware Mode (fallback)
# Presionar S y Enter

# 4. Salida esperada
# SMOPSYS2: INITIATING ENERGY RECOVERY...
# [====...====] 100% (Loading bar)
# Entering Bifurcation Loop...
# !! ANOMALY DETECTED !! (ocasional si sorpresa > threshold)

# 5. Salir: Ctrl+A, X
```

**Debugging en QEMU:**

```bash
# Terminal 1: Iniciar QEMU con gdbserver
qemu-system-riscv64 -machine virt -nographic \
  -serial mon:stdio -bios none -kernel smopsys.elf \
  -gdb tcp::1234 -S  # Pausa en boot para debug

# Terminal 2: Conectar GDB
riscv64-unknown-elf-gdb smopsys.elf
(gdb) target remote localhost:1234
(gdb) b kernel_main
(gdb) c  # Continue
(gdb) si  # Step instruction
```

---

### B. HiFive Unleashed Bare-Metal (Hardware Real)

#### **Opción B1: Despliegue sin Linux (Máximo Control)**

**Requisitos Hardware:**
- USB-TTL adapter (consola serie)
- Adaptador JTAG (FlashPro5 o compatible OpenOCD)
- Cable micro-USB (alimentación)

**Procedimiento:**

```bash
# Paso 1: Compilar para HiFive
CROSS_COMPILE=riscv64-unknown-elf- make kernel

# Paso 2: Conectar JTAG y acceder a OpenOCD
# (Asumir que tienes OpenOCD instalado y configurado para HiFive)
openocd -f board/sifive_hifive.cfg

# Paso 3: En otra terminal, GDB para cargar el kernel
riscv64-unknown-elf-gdb smopsys.elf
(gdb) target remote localhost:3333
(gdb) load  # Carga en RAM
(gdb) continue

# Paso 4: Monitorear consola serie
screen /dev/ttyUSB0 115200
# O usar picocom
picocom -b 115200 /dev/ttyUSB0
```

**Alternativa: Flashear Directamente a SPI-NOR (Permanente)**

```bash
# ⚠️ PELIGRO: Esto sobrescribe el bootloader
# Solo si tienes backup de FSBL/OpenSBI

openocd -f board/sifive_hifive.cfg

# En OpenSBI telnet:3333
> flash probe 0
> flash erase_sector 0 0 255  # Borrar SPI-NOR completo
> program smopsys.elf 0x20000000  # Flashear a 512MB offset (si no pisas FSBL)
```

---

#### **Opción B2: Despliegue sobre Linux (Kernel Module)**

Si prefieres mantener Linux en ejecución y cargar Smopsys2 como módulo:

```bash
# Paso 1: Compilar módulo kernel
cd platforms/linux
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

# Paso 2: Insertar módulo
sudo insmod smopsys.ko

# Paso 3: Monitorear logs
sudo dmesg -f | grep smopsys

# Salida esperada:
# [    5.123456] Smopsys: Loading Metriplectic Core...
# [    5.124789] Smopsys: Metriplectic Ticker Started.
```

**Ventaja**: No necesitas rebootear, debug desde Linux estándar.
**Desventaja**: Menor control de timing real-time (competencia con scheduler Linux).

---

### C. VisionFive 2 (Recomendado Actualmente)

**Flujo Similar a HiFive pero con xc3s USB-JTAG:**

```bash
# Paso 1: Asegurar que tienes u-boot cargado (viene pre-instalado)
# Verificar: Conectar puerto micro-USB, ejecutar
screen /dev/ttyUSB0 115200
# Deberías ver prompt de u-boot

# Paso 2: Compilar Smopsys2 para JH7110 (compatible con genérico RV64)
CROSS_COMPILE=riscv64-linux-gnu- make kernel

# Paso 3: Copiar a tarjeta microSD
# a) Insertar microSD en lector
# b) Crear partición FAT32
sudo mkfs.fat -F 32 /dev/sdX1

# c) Copiar kernel
sudo cp smopsys.elf /media/usb0/

# Paso 4: En VisionFive, u-boot cargar desde SD
# En consola u-boot:
=> fatload mmc 0:1 0x80200000 smopsys.elf
=> go 0x80200000
```

**O usar script u-boot (boot.scr):**

```bash
# Crear script
cat > boot.cmd << 'EOF'
fatload mmc 0:1 0x80200000 smopsys.elf
go 0x80200000
EOF

# Compilar a formato binario (mkimage)
mkimage -T script -A riscv -C none -d boot.cmd boot.scr

# Copiar a SD
cp boot.scr /media/usb0/

# En u-boot:
=> source mmc 0:1 boot.scr
```

---

## Pipeline de Build & Deploy

### CI/CD Automatizado (GitHub Actions)

**Crear `.github/workflows/deploy.yml`:**

```yaml
name: Build & Deploy Smopsys2

on:
  push:
    branches: [main, develop]
    tags: ['v*']

env:
  REGISTRY: ghcr.io
  IMAGE_NAME: ${{ github.repository }}

jobs:
  build-kernel:
    runs-on: ubuntu-22.04
    strategy:
      matrix:
        target: [qemu, hifive, visionfive]

    steps:
      - uses: actions/checkout@v3

      - name: Install RISC-V Toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            gcc-riscv64-unknown-elf \
            binutils-riscv64-unknown-elf \
            qemu-system-riscv64

      - name: Build Kernel (${{ matrix.target }})
        run: |
          make clean
          CROSS_COMPILE=riscv64-unknown-elf- make kernel
          cp smopsys.elf smopsys-${{ matrix.target }}.elf

      - name: Run Tests (QEMU only)
        if: matrix.target == 'qemu'
        run: |
          make test_lib
          python3 -m pytest tests/ -v

      - name: Upload Artifact
        uses: actions/upload-artifact@v3
        with:
          name: kernel-${{ matrix.target }}
          path: smopsys-${{ matrix.target }}.elf
          retention-days: 30

  build-docker:
    runs-on: ubuntu-22.04
    permissions:
      contents: read
      packages: write

    steps:
      - uses: actions/checkout@v3

      - name: Set up Docker Buildx
        uses: docker/setup-buildx-action@v2

      - name: Log in to Container Registry
        uses: docker/login-action@v2
        with:
          registry: ${{ env.REGISTRY }}
          username: ${{ github.actor }}
          password: ${{ secrets.GITHUB_TOKEN }}

      - name: Extract metadata
        id: meta
        uses: docker/metadata-action@v4
        with:
          images: ${{ env.REGISTRY }}/${{ env.IMAGE_NAME }}
          tags: |
            type=ref,event=branch
            type=semver,pattern={{version}}
            type=sha

      - name: Build & Push Docker Image
        uses: docker/build-push-action@v4
        with:
          context: .
          push: ${{ github.event_name != 'pull_request' }}
          tags: ${{ steps.meta.outputs.tags }}
          labels: ${{ steps.meta.outputs.labels }}

  deploy:
    needs: build-kernel
    if: startsWith(github.ref, 'refs/tags/v')
    runs-on: ubuntu-22.04

    steps:
      - name: Download Kernel Artifacts
        uses: actions/download-artifact@v3

      - name: Create Release
        uses: softprops/action-gh-release@v1
        with:
          files: kernel-*/smopsys-*.elf
          draft: false
          prerelease: ${{ contains(github.ref, 'alpha') || contains(github.ref, 'beta') }}
```

---

## Debugging & Troubleshooting

### Problema 1: "Load Access Fault" en MMIO Bridge

```
[Exception] Load Access Fault at pc=0x80xxxxxx, addr=0xF8000000
Kernel Halt
```

**Causa**: Intento de acceder a 0xF8000000 en modo no simulado sin hardware.

**Solución**:
```c
// En qport_handshake(), selecciona Simulation Mode
// El flag g_simulation_mode evita accesos reales a MMIO
if (g_simulation_mode) {
    // Usar mock buffer en lugar de hardware
    mock_mmio_buffer[...] = ...
}
```

---

### Problema 2: Tests Fallan en Hardware Real

**Error**: `test_bridge.py::test_bridge_handshake TIMEOUT`

**Causa**: Timing diferente en hardware real vs. QEMU.

**Solución**:
```bash
# Aumentar timeouts en tests
# En tests/conftest.py
@pytest.fixture
def timeout_scale():
    # En hardware: 10x más lento que QEMU
    return 10 if os.environ.get('REAL_HW') else 1
```

---

### Problema 3: UART no muestra salida

**Síntoma**: Consola serie silenciosa.

**Diagnóstico**:
```bash
# 1. Verificar conexión física
screen /dev/ttyUSB0 115200

# 2. Si sin salida, probar reset manual
# (Presionar botón RESET en placa)

# 3. Si aún nada, verificar baudrate
# VisionFive: 115200 (default)
# HiFive: 115200
# Custom: Revisar devicetree

# 4. Último recurso: JTAG debug
openocd -f board/tu_placa.cfg
# (Conectar con GDB y leer UART_THR directamente)
```

---

## Compliance Metripléctico

### Checklist de Validación Pre-Deploy

- [ ] **Conservativo (Preservación)**
  - [ ] Git commit con tag semántico (v1.2.3)
  - [ ] Tests 100% pasando
  - [ ] Documentación actualizada
  - [ ] Kernel binary reproducible (bit-for-bit)

- [ ] **Disipativo (Monitoreo)**
  - [ ] Security heartbeat habilitado
  - [ ] Logs capturando anomalías
  - [ ] Métricas de decoherencia Lindblad correctas
  - [ ] Forensic log listo para análisis post-mortem

- [ ] **Topológico (Correspondencia)**
  - [ ] Arquitectura (Isomorfismo Nivel 3) validada
  - [ ] Hardware target coincide con binary
  - [ ] Offset de memoria QPORT correcto (0xF8000000 o custom)

---

## Referencia Rápida

| Tarea | Comando |
|-------|---------|
| Build local | `make kernel` |
| Test | `make test_lib && pytest tests/` |
| Docker | `docker build -t smopsys2 .` |
| QEMU | `qemu-system-riscv64 -machine virt ... -kernel smopsys.elf` |
| HiFive JTAG | `openocd -f board/sifive_hifive.cfg` + GDB |
| VisionFive SD | `fatload mmc 0:1 0x80200000 smopsys.elf && go 0x80200000` |
| Release | `git tag v1.0.0 && git push --tags` |

---

**Autor**: Jacobo Tlacaelel Mina Rodriguez
**Licencia**: Apache 2.0
**Última revisión**: Enero 2026
