/* uart.c */
#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#define GPFSEL1     ((volatile uint32_t *)(0x3f200004))
#define GPPUDCLK0   ((volatile uint32_t *)(0x3f200098))

#define AUX_ENABLES	((volatile uint32_t *)(0x3F215004))
#define AUX_MU_IO	((volatile uint32_t *)(0x3F215040))
#define AUX_MU_IER	((volatile uint32_t *)(0x3F215044))
#define AUX_MU_IIR	((volatile uint32_t *)(0x3F215048))
#define AUX_MU_LCR	((volatile uint32_t *)(0x3F215048))
#define AUX_MU_LSR	((volatile uint32_t *)(0x3F215054))
#define AUX_MU_BAUD	((volatile uint32_t *)(0x3F215068))

struct UARTCTL {
	SemaphoreHandle_t *tx_mux;
	QueueHandle_t     *rx_queue;
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

void uart_putchar_isr(uint8_t c)
{
	xSemaphoreTakeFromISR(uartctl->tx_mux, NULL);
	/* wait mini uart for tx idle. */
	while ( !(*AUX_MU_LSR & (1 << 5)) ) { }
	*AUX_MU_IO = c;
	xSemaphoreGiveFromISR(uartctl->tx_mux, NULL);
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

uint32_t uart_read_bytes(uint8_t *buf, uint32_t length)
{
	uint32_t num = uxQueueMessagesWaiting(uartctl->rx_queue);
	uint32_t i;

	for (i = 0; i < num || i < length; i++) {
		xQueueReceive(uartctl->rx_queue, &buf[i], (portTickType) portMAX_DELAY);
	}

	return i;
}
/*-----------------------------------------------------------*/

typedef void (*INTERRUPT_HANDLER) (void);
typedef struct {
	INTERRUPT_HANDLER fn;
} INTERRUPT_VECTOR;

static INTERRUPT_VECTOR g_vector_table[64];

#define IRQ_ENABLE_1		((volatile uint32_t *)(0x3F00B210))
static void uart_isr_register(void (*fn)(void))
{
	g_vector_table[29].fn = fn;

	/* enable AUX miniuart rx interrupt */
	*AUX_ENABLES = 1;
	*AUX_MU_IIR  = 6; /* clear tx & rx interrupt*/
	*AUX_MU_IER  = 1;

	/* unmask AUX interrupt */
	*IRQ_ENABLE_1 = 1 << 29;
}
/*-----------------------------------------------------------*/

void uart_isr(void)
{
	/* RX data */
	if(*AUX_MU_LSR & 1 << 0) {
		uint8_t c = (uint8_t) 0xFF & *AUX_MU_IO;
		xQueueSendToBackFromISR(uartctl->rx_queue, &c, NULL);
	}
}
/*-----------------------------------------------------------*/

void uart_init(void)
{
	uint32_t r;

    /* GPIO14 GPIO15 */
	r = *GPFSEL1;
	r &= ~(7<<12|7<<15);
	r |= 2<<12|2<<15; /* ALT5 */
	*GPFSEL1 = r;
    r = 150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = (1<<14)|(1<<15);
    r = 150; while(r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;

    /* 115200/8bit */
    *AUX_MU_BAUD = 270;    /* 115200 bps */
    *AUX_MU_LCR = 3;       /* 8 bits */
    /* Mini uart  only support  non parity. */

	uartctl = pvPortMalloc(sizeof (struct UARTCTL));
	uartctl->tx_mux = xSemaphoreCreateMutex();
	uartctl->rx_queue = xQueueCreate(16, sizeof (uint8_t));
	uart_isr_register(uart_isr);
}
/*-----------------------------------------------------------*/
#define IRQ_BASIC_PENDING	((volatile uint32_t *)(0x3F00B200))
#define IRQ_PENDING_1		((volatile uint32_t *)(0x3F00B204))
#define IRQ_PENDING_2		((volatile uint32_t *)(0x3F00B208))

static void handle_range(uint32_t pending, const uint32_t base)
{
	while (pending) {
		/* get index of first set_bit */
		uint32_t bit = 31 - __builtin_clz(pending);
		uint32_t irq = base + bit;

		/* call handler */
		if(g_vector_table[irq].fn)
			g_vector_table[irq].fn();

		/* clear bit */
		pending &= ~(1UL << bit);
	}
}

void irq_handler(void)
{
	uint32_t basic = *IRQ_BASIC_PENDING & 0x00000300;

	if (basic & 0x100)
		handle_range(*IRQ_PENDING_1, 0);
	if (basic & 0x200)
		handle_range(*IRQ_PENDING_2, 32);
}

