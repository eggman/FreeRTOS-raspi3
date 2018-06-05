/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "demo.h"

/* ARM Generic Timer */
#define CORE0_TIMER_IRQCNTL    ((volatile uint32_t *)(0x40000040))
static uint32_t timer_cntfrq = 0;
static uint32_t timer_tick = 0;

void enable_cntv(void)
{
	uint32_t cntv_ctl;
	cntv_ctl = 1;
	asm volatile ("msr cntv_ctl_el0, %0" :: "r" (cntv_ctl));
}
/*-----------------------------------------------------------*/

void write_cntv_tval(uint32_t val)
{
	asm volatile ("msr cntv_tval_el0, %0" :: "r" (val));
	return;
}
/*-----------------------------------------------------------*/

uint32_t read_cntfrq(void)
{
	uint32_t val;
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (val));
	return val;
}
/*-----------------------------------------------------------*/

void init_timer(void)
{
	timer_cntfrq = timer_tick = read_cntfrq();
	write_cntv_tval(timer_cntfrq);    // clear cntv interrupt and set next 1 sec timer.
	return;
}
/*-----------------------------------------------------------*/

void timer_set_tick_rate_hz(uint32_t rate)
{
	timer_tick = timer_cntfrq / rate ;
	write_cntv_tval(timer_tick);
}
/*-----------------------------------------------------------*/

void vConfigureTickInterrupt( void )
{
	/* init timer device. */
	init_timer();

	/* set tick rate. */
	timer_set_tick_rate_hz(configTICK_RATE_HZ);

	/* timer interrupt routing. */
	*CORE0_TIMER_IRQCNTL = 1 << 3; /* nCNTVIRQ routing to CORE0.*/

	/* start & enable interrupts in the timer. */
	enable_cntv();
}
/*-----------------------------------------------------------*/

void vClearTickInterrupt( void )
{
	write_cntv_tval(timer_tick);    // clear cntv interrupt and set next timer.
	return;
}
/*-----------------------------------------------------------*/

void vApplicationIRQHandler( uint32_t ulCORE0_INT_SRC )
{
	uint32_t ulInterruptID;
	ulInterruptID = ulCORE0_INT_SRC & 0x0007FFFFUL;

	/* call handler function */
	if(ulInterruptID & (1 << 3))
	{
		/* Generic Timer */
		FreeRTOS_Tick_Handler();
	}
	if(ulInterruptID & (1 << 8))
	{
		/* Peripherals */
		irq_handler();
	}
}

