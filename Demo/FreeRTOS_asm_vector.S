.org 0
.text

	.globl _freertos_vector_table


.section .vectors, "a"

_freertos_vector_table:

.set	VBAR, _freertos_vector_table

.org VBAR
	b	_boot

.org (VBAR + 0x80)
	b	.

.org (VBAR + 0x100)
	b	.

.org (VBAR + 0x180)
	b	.


.org (VBAR + 0x200)
	b	.

.org (VBAR + 0x280)
	b	.

.org (VBAR + 0x300)
	b	.

.org (VBAR + 0x380)
	b	.



.org (VBAR + 0x400)
	b	.

.org (VBAR + 0x480)
	b	.

.org (VBAR + 0x500)
	b	.

.org (VBAR + 0x580)
	b	.

.org (VBAR + 0x600)
	b	.

.org (VBAR + 0x680)
	b	.

.org (VBAR + 0x700)
	b	.

.org (VBAR + 0x780)
	b	.
