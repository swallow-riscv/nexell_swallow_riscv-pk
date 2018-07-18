#ifndef _RISCV_UART_DW_H
#define _RISCV_UART_DW_H

#include <stdint.h>

extern volatile uint8_t* uart_dw;

void uart_dw_putchar(uint8_t ch);
int uart_dw_getchar();
void query_uart_dw(uintptr_t dtb);

#endif
