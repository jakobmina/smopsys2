#ifndef QCORE_VIZ_H
#define QCORE_VIZ_H

#include <stdint.h>

// ANSI Color Codes
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_CLEAR_SCREEN  "\x1b[2J"
#define ANSI_CURSOR_HOME   "\x1b[H"

// Visualization Prototypes
void visualize_laminar_flow(float entropy);
void display_loading_bar(void);
uint32_t pseudo_random(void);

#endif // QCORE_VIZ_H
