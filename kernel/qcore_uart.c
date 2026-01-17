#include "qcore_uart.h"

// Helper to access registers
static volatile uint8_t* uart_reg(uint32_t offset) {
    return (volatile uint8_t*)(UART0_BASE + offset);
}

void uart_init(void) {
    // Disable interrupts
    *uart_reg(UART_IER) = 0x00;
    
    // Enable FIFO
    *uart_reg(UART_FCR) = 0x01;
    
    // Set 8-bit data length (LCR = 3)
    *uart_reg(UART_LCR) = 0x03;
}

void uart_putc(char c) {
    // Wait for Transmitter Holding Register Empty (THRE)
    while ((*uart_reg(UART_LSR) & UART_LSR_THRE) == 0);
    
    *uart_reg(UART_THR) = c;
}

void uart_puts(const char* s) {
    while (*s) {
        uart_putc(*s++);
    }
}

void uart_print_hex(uint32_t val) {
    uart_puts("0x");
    for (int i = 7; i >= 0; i--) {
        uint32_t nibble = (val >> (i * 4)) & 0xF;
        if (nibble < 10) {
            uart_putc('0' + nibble);
        } else {
            uart_putc('A' + (nibble - 10));
        }
    }
}
