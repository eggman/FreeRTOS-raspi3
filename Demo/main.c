#include <stddef.h>
#include <stdint.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

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
/*-----------------------------------------------------------*/

void TaskA(void *pvParameters)
{
	(void) pvParameters;

    for( ;; )
    {
		uart_puthex(xTaskGetTickCount());
		uart_putchar('\n');
		vTaskDelay(500 / portTICK_RATE_MS);
    }
}

/*-----------------------------------------------------------*/

TimerHandle_t timer;
uint32_t count=0;
void interval_func(TimerHandle_t pxTimer)
{
	(void) pxTimer;
	count++;
	uart_puts("timer count: ");
	uart_puthex(count);
	uart_putchar('\n');
}
/*-----------------------------------------------------------*/

void main(void)
{
	TaskHandle_t xHandle;
	QueueHandle_t q;

	uart_init();
	uart_puts("hello world\n");

	xTaskCreate(TaskA, "Task A", 512, NULL, tskIDLE_PRIORITY, &xHandle);

	timer = xTimerCreate("print_every_1000ms",(1000 / portTICK_RATE_MS), pdTRUE, (void *)0, interval_func);
	if(timer != NULL)
	{
		xTimerStart(timer, 0);
	}

	q = xQueueCreate(16, 16);
	xQueueSend(q, xHandle, 0);

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
