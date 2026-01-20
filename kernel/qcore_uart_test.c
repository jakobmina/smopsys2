#include "qcore_uart.h"
#include <stdio.h>

/**
 * Mock implementation of UART for Host Test Environment.
 * Redirects output to stdout instead of hardware MMIO.
 */

static char mock_input_char = 0;

void set_mock_uart_input(char c) {
    mock_input_char = c;
}

void uart_init(void) {
    // No initialization needed for mock
}

void uart_putc(char c) {
    putchar(c);
    fflush(stdout);
}

void uart_puts(const char* s) {
    printf("%s", s);
    fflush(stdout);
}

void uart_print_hex(uint32_t val) {
    printf("0x%08X", val);
    fflush(stdout);
}

int uart_has_data(void) {
    return mock_input_char != 0;
}

char uart_getc(void) {
    char c = mock_input_char;
    mock_input_char = 0;
    return c;
}

char uart_getc_nonblocking(void) {
    char c = mock_input_char;
    mock_input_char = 0;
    return c;
}
