#include "../include/qcore_viz.h"
#include "../include/qcore_uart.h"

static uint32_t next = 1;

uint32_t pseudo_random(void) {
    next = next * 1103515245 + 12345;
    return (uint32_t)(next / 65536) % 32768;
}

void visualize_laminar_flow(float entropy) {
    char* color;
    
    // El color depende de la entropía (convergencia del Operador Golden)
    if (entropy > 0.8f) color = ANSI_COLOR_RED;    // Caos inicial
    else if (entropy > 0.3f) color = ANSI_COLOR_YELLOW; // Estabilizando
    else color = ANSI_COLOR_GREEN;                 // Flujo Laminar

    uart_puts(color);
    
    // Generamos una línea de "lluvia binaria" basada en la fragmentación
    for (int i = 0; i < 80; i++) {
        if (pseudo_random() % 10 > 7) {
            uart_putc((pseudo_random() % 2) ? '1' : '0');
        } else {
            uart_putc(' '); // Espacios para el efecto de gotas
        }
    }
    uart_puts("\n" ANSI_COLOR_RESET);
}

void display_loading_bar(void) {
    const int width = 50;
    uart_puts(ANSI_COLOR_CYAN "[");
    for (int i = 0; i < width; i++) {
        for (volatile int delay = 0; delay < 100000; delay++); // Delay artificial
        uart_putc('=');
    }
    uart_puts("] 100%\n" ANSI_COLOR_RESET);
}
