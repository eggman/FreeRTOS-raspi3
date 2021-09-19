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

	uart_puts("start TaskA\n");

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
	uint8_t buf[2];
	uint32_t len = 0;

	len = uart_read_bytes(buf, sizeof(buf) - 1);
	if (len)
		uart_puts((char *)buf);
}
/*-----------------------------------------------------------*/

void main(void)
{
	TaskHandle_t task_a;

	uart_init();
	uart_puts("qemu exit: Ctrl-A x / qemu monitor: Ctrl-A c\n");
	uart_puts("hello world\n");

	xTaskCreate(TaskA, "Task A", 512, NULL, tskIDLE_PRIORITY, &task_a);

	timer = xTimerCreate("print_every_10ms",(10 / portTICK_RATE_MS), pdTRUE, (void *)0, interval_func);
	if(timer != NULL)
	{
		xTimerStart(timer, 0);
	}

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
