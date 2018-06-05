#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "demo.h"

/* Prototypes for the standard FreeRTOS callback/hook functions implemented
within this file. */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );

static inline void io_halt(void)
{
    asm volatile ("wfi");
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
