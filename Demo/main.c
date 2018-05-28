#include <stddef.h>
#include <stdint.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );

static inline void io_halt(void)
{
    asm volatile ("wfi");
}

#define AUX_MU_IO   ((volatile uint32_t *)(0x3F215040))
#define AUX_MU_LSR  ((volatile uint32_t *)(0x3F215054))

void uart_putchar(uint8_t c)
{
    /* wait mini uart for tx idle. */
    while ( !(*AUX_MU_LSR & (1 << 5)) ) { }
    *AUX_MU_IO = c;
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i ++)
        uart_putchar((uint8_t)str[i]);
}

void uart_puthex(uint64_t v)
{
	const char *hexdigits = "0123456789ABSDEF";
	for (int i = 60; i >= 0; i -= 4)
		uart_putchar(hexdigits[(v >> i) & 0xf]);
}

/*-----------------------------------------------------------*/

void main(void)
{
	uart_puts("hello world\n");

	vTaskStartScheduler();

	uart_puthex(xTaskGetTickCount());
	uart_putchar('\n');

	while (1) {
		io_halt();
	}
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
