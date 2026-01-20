# Smopsys2 Makefile

# --- Toolchain Definitions ---
# Cross-Compiler for RISC-V Kernel
CROSS_COMPILE ?= riscv64-unknown-elf-
CC_RISCV = $(CROSS_COMPILE)gcc
LD_RISCV = $(CROSS_COMPILE)ld

# Host Compiler for Tests
CC_HOST = gcc

# --- Target Definitions ---
KERNEL_ELF = smopsys.elf
TEST_LIB = libqcore.so

# --- Source Files ---
# Kernel Sources (C)
C_SRCS = kernel/qcore_math.c \
         kernel/qcore_scheduler.c \
         kernel/qcore_asm.c \
         kernel/qcore_quantum.c \
         kernel/qcore_wetware.c \
         kernel/qcore_bayes.c \
         kernel/qcore_bridge.c \
         kernel/qcore_security.c \
         kernel/qcore_lindblad.c \
         kernel/qcore_uart.c \
         kernel/qcore_viz.c \
         kernel/qcore_pim.c \
         kernel/main.c

# Kernel Entry (Assembly)
ASM_SRCS = kernel/entry.S \
           kernel/qcore_pim_asm.S

# Shared Sources for Test Lib (Exclude main.c to avoid conflict/entry point issues in lib)
TEST_SRCS = kernel/qcore_math.c \
            kernel/qcore_scheduler.c \
            kernel/qcore_asm.c \
            kernel/qcore_quantum.c \
            kernel/qcore_wetware.c \
            kernel/qcore_bayes.c \
            kernel/qcore_bridge.c \
            kernel/qcore_security.c \
            kernel/qcore_lindblad.c \
            kernel/qcore_uart_test.c \
            kernel/qcore_viz.c

# --- Flags ---
# RISC-V Bare Metal Flags
# -mcmodel=medany: PC-relative addressing for kernel usage
# -ffreestanding: No standard lib environment
# -nostdlib: Do not link libc
CFLAGS_KERNEL = -Wall -Wextra -O2 -mcmodel=medany -ffreestanding -nostdlib -I./include
LDFLAGS_KERNEL = -T kernel.ld -nostdlib

# Host Test Flags
# -DQCORE_TEST_ENV: Enable Mock MMIO buffers
CFLAGS_TEST = -fPIC -I./include -Wall -Wextra -shared -DQCORE_TEST_ENV

# --- Rules ---

.PHONY: all kernel test_lib clean check_toolchain

all: test_lib kernel

# Build RISC-V Kernel Image
kernel: check_toolchain $(KERNEL_ELF)

$(KERNEL_ELF): $(ASM_SRCS) $(C_SRCS) kernel.ld
	$(CC_RISCV) $(CFLAGS_KERNEL) $(ASM_SRCS) $(C_SRCS) $(LDFLAGS_KERNEL) -o $@

# Build Host Test Library
test_lib: $(TEST_LIB)

$(TEST_LIB): $(TEST_SRCS)
	$(CC_HOST) $(CFLAGS_TEST) -o $@ $(TEST_SRCS)

# Helper to check if RISC-V toolchain is present
check_toolchain:
	@which $(CC_RISCV) > /dev/null || (echo "Warning: $(CC_RISCV) not found. Skipping kernel build." && false)

clean:
	rm -f $(KERNEL_ELF) $(TEST_LIB)
