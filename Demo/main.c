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

void TaskA(void *pvParameters)
{
	(void) pvParameters;

    for( ;; )
    {
		uart_puthex(xTaskGetTickCount());
		uart_putchar('\n');
    }
}

/*-----------------------------------------------------------*/

void main(void)
{
	xTaskHandle xHandle;
	uart_puts("hello world\n");

	xTaskCreate(TaskA, "Task A", 512, NULL, tskIDLE_PRIORITY, &xHandle);

	vTaskStartScheduler();
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
