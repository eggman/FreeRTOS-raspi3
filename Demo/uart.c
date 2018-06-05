/* uart.c */
#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#define AUX_MU_IO   ((volatile uint32_t *)(0x3F215040))
#define AUX_MU_LSR  ((volatile uint32_t *)(0x3F215054))

struct UARTCTL {
	SemaphoreHandle_t *tx_mux;
};
struct UARTCTL *uartctl;

void uart_putchar(uint8_t c)
{
	xSemaphoreTake(uartctl->tx_mux, (portTickType) portMAX_DELAY);
	/* wait mini uart for tx idle. */
	while ( !(*AUX_MU_LSR & (1 << 5)) ) { }
	*AUX_MU_IO = c;
	xSemaphoreGive(uartctl->tx_mux);
}
/*-----------------------------------------------------------*/

void uart_puts(const char* str)
{
	for (size_t i = 0; str[i] != '\0'; i ++)
		uart_putchar((uint8_t)str[i]);
}
/*-----------------------------------------------------------*/

void uart_puthex(uint64_t v)
{
	const char *hexdigits = "0123456789ABSDEF";
	for (int i = 60; i >= 0; i -= 4)
		uart_putchar(hexdigits[(v >> i) & 0xf]);
}
/*-----------------------------------------------------------*/

void uart_init(void)
{
	uartctl = pvPortMalloc(sizeof (struct UARTCTL));
	uartctl->tx_mux = xSemaphoreCreateMutex();
}

