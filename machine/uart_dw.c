#include <string.h>
#include "mtrap.h"
#include "uart_dw.h"
#include "fdt.h"

volatile uint8_t* uart_dw;

#define UART_REG_QUEUE     0x0
#define UART_REG_LINESTAT  0x14
#define UART_REG_STATUS_RX 0x01
#define UART_REG_STATUS_TX 0x20

void uart_dw_putchar(uint8_t ch)
{
//workaround
  uart_dw = (void*)(0x20880000);
  while ((uart_dw[UART_REG_LINESTAT] & UART_REG_STATUS_TX) == 0);
  uart_dw[UART_REG_QUEUE] = ch;
}

int uart_dw_getchar()
{
  if (uart_dw[UART_REG_LINESTAT] & UART_REG_STATUS_RX)
    return uart_dw[UART_REG_QUEUE];
  return -1;
}

struct uart_dw_scan
{
  int compat;
  uint64_t reg;
};

static void uart_dw_open(const struct fdt_scan_node *node, void *extra)
{
  struct uart_dw_scan *scan = (struct uart_dw_scan *)extra;
  memset(scan, 0, sizeof(*scan));
}

static void uart_dw_prop(const struct fdt_scan_prop *prop, void *extra)
{
  struct uart_dw_scan *scan = (struct uart_dw_scan *)extra;
  if (!strcmp(prop->name, "compatible") && !strcmp((const char*)prop->value, "snps,dw-apb-uart")) {
    scan->compat = 1;
  } else if (!strcmp(prop->name, "reg")) {
    fdt_get_address(prop->node->parent, prop->value, &scan->reg);
  }
}

static void uart_dw_done(const struct fdt_scan_node *node, void *extra)
{
  struct uart_dw_scan *scan = (struct uart_dw_scan *)extra;
  if (!scan->compat || !scan->reg || uart_dw) return;

  uart_dw = (void*)(uintptr_t)scan->reg;
  //workaround  
  uart_dw = (void*)(0x20880000);
  /* // http://wiki.osdev.org/Serial_Ports */
  /* uart_dw[0x4] = 0x00;    // Disable all interrupts */
  /* uart_dw[0xc] = 0x80;    // Enable DLAB (set baud rate divisor) */
  /* uart_dw[0x0] = 0x03;    // Set divisor to 3 (lo byte) 38400 baud */
  /* uart_dw[0x4] = 0x00;    //                  (hi byte) */
  /* uart_dw[0xc] = 0x03;    // 8 bits, no parity, one stop bit */
  /* uart_dw[0x8] = 0xC7;    // Enable FIFO, clear them, with 14-byte threshold */
}

void query_uart_dw(uintptr_t fdt)
{
  struct fdt_cb cb;
  struct uart_dw_scan scan;

  memset(&cb, 0, sizeof(cb));
  cb.open = uart_dw_open;
  cb.prop = uart_dw_prop;
  cb.done = uart_dw_done;
  cb.extra = &scan;

  fdt_scan(fdt, &cb);
}
