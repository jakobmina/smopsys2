/* * Smopsys2 Linker Script
 * Arquitectura: RISC-V Bare-Metal
 * Objetivo: Aislar la región MMQI y establecer el flujo de arranque.
 */

OUTPUT_ARCH( "riscv" )
ENTRY( _start )

MEMORY
{
  /* * RAM: Donde vive el Kernel (Código + Datos).
   * Base estándar RISC-V (QEMU Virt / Hardware físico): 0x80000000 
   * Tamaño: 128MB (Suficiente para lógica metripléctica)
   */
  RAM  (rwx) : ORIGIN = 0x80000000, LENGTH = 128M

  /* * MMQI_REGION: La Zona Prohibida / Puerto Cuántico.
   * Reservamos esto explícitamente para que el linker lance un ERROR
   * si intentamos poner código aquí por accidente.
   */
  MMQI (rw)  : ORIGIN = 0xF8000000, LENGTH = 4K
}

SECTIONS
{
  /* El código comienza al inicio de la RAM */
  . = 0x80000000;

  /* 1. Código Ejecutable (.text) */
  .text : {
    PROVIDE(_text_start = .);
    *(.text.init) *(.text .text.*) /* Código de inicio primero */
    PROVIDE(_text_end = .);
  } > RAM

  /* 2. Datos de Solo Lectura (.rodata) */
  .rodata : {
    . = ALIGN(16);
    *(.rodata .rodata.*)
  } > RAM

  /* 3. Datos Inicializados (.data) */
  .data : {
    . = ALIGN(16);
    *(.data .data.*)
    *(.sdata .sdata.*) /* Small data para RISC-V (optimización gp) */
  } > RAM

  /* 4. Datos No Inicializados (.bss) */
  /* Importante: Debemos limpiarlos a cero en el arranque */
  .bss : {
    . = ALIGN(16);
    PROVIDE(_bss_start = .);
    *(.bss .bss.*)
    *(.sbss .sbss.*)
    PROVIDE(_bss_end = .);
  } > RAM

  /* 5. Stack (Pila) */
  /* Definimos un stack de 64KB al final de los datos */
  . = ALIGN(16);
  . += 0x10000; /* 64KB Stack size */
  PROVIDE(_stack_top = .);

  /* * CONTROL DE SEGURIDAD TOPOLÓGICA 
   * Verificamos matemáticamente que el final de nuestra memoria RAM usada
   * no esté invadiendo el espacio del Puerto Cuántico.
   */
  ASSERT(. < 0xF8000000, "CRITICAL ERROR: Kernel image overlaps with Quantum Port Region!")
}
