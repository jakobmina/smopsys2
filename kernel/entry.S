# Smopsys2 Entry Point
# Configura el entorno bare-metal antes de saltar a C.

.section .text.boot
.global _start

_start:
    # 1. Deshabilitar interrupciones (Modo silencio total al arrancar)
    csrw mie, zero

    # 2. Configurar el Stack Pointer (sp)
    # _stack_top viene definido desde el linker.ld
    la sp, _stack_top

    # 3. Limpiar el BSS (Variables estáticas a 0)
    # Esto es vital para que las variables de estado (flags) inicien limpias.
    la t0, _bss_start
    la t1, _bss_end
    bge t0, t1, enter_kernel # Si tamaño es 0, saltar

zero_loop:
    sw zero, 0(t0)
    addi t0, t0, 4
    blt t0, t1, zero_loop

enter_kernel:
    # 4. Salto al Núcleo Metripléctico
    # Saltamos a main(). Si main retorna (no debería), colgamos el sistema.
    call kernel_main

hang:
    wfi        # Wait for Interrupt (Ahorro de energía)
    j hang     # Bucle infinito de seguridad
