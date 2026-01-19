#ifndef QCORE_UART_H
#define QCORE_UART_H

#include <stdint.h>

// Base address for UART0 on QEMU virt machine
#define UART0_BASE 0x10000000

// Register offsets (8-bit width)
#define UART_RBR 0x00 // Receiver Buffer Register (Read)
#define UART_THR 0x00 // Transmitter Holding Register (Write)
#define UART_IER 0x01 // Interrupt Enable Register
#define UART_FCR 0x02 // FIFO Control Register
#define UART_LCR 0x03 // Line Control Register
#define UART_MCR 0x04 // Modem Control Register
#define UART_LSR 0x05 // Line Status Register

// Line Status Register bits
#define UART_LSR_DR   0x01 // Data Ready
#define UART_LSR_THRE 0x20 // Transmitter Holding Register Empty

// Function prototypes
void uart_init(void);
void uart_putc(char c);
void uart_puts(const char* s);
void uart_print_hex(uint32_t val);
int  uart_has_data(void);
char uart_getc(void);
char uart_getc_nonblocking(void);

#endif // QCORE_UART_H
