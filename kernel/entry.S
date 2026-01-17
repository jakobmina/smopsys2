# Smopsys2 Entry Point
# Configura el entorno bare-metal antes de saltar a C.

.section .text.boot
.global _start

_start:
    # 1. Configurar el vector de excepciones (mtvec)
    la t0, trap_vector
    csrw mtvec, t0

    # 2. Deshabilitar interrupciones
    csrw mie, zero

    # 3. Configurar el Stack Pointer (sp)
    la sp, _stack_top

    # 4. Limpiar el BSS
    la t0, _bss_start
    la t1, _bss_end
    bge t0, t1, enter_kernel

zero_loop:
    sw zero, 0(t0)
    addi t0, t0, 4
    blt t0, t1, zero_loop

enter_kernel:
    call kernel_main

hang:
    wfi
    j hang

# --- Vector de Excepciones Básico ---
.align 4
trap_vector:
    # Si ocurre un trap (como Load Access Fault al tocar hardware inexistente)
    # saltamos aquí. En un sistema real recuperaríamos el estado.
    # Aquí simplemente nos quedamos parados para evitar corrupción.
    j trap_vector
