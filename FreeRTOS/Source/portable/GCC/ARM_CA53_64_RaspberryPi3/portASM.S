/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

	.text

	/* Variables and functions. */
	.extern ullMaxAPIPriorityMask
	.extern pxCurrentTCB
	.extern _freertos_vector_table

	.globl vPortRestoreTaskContext

; /**********************************************************************/

.macro portRESTORE_CONTEXT

	/* Switch to use the EL0 stack pointer. */
	MSR 	SPSEL, #0

	/* Set the SP to point to the stack of the task being restored. */
	LDR		X0, pxCurrentTCBConst
	LDR		X1, [X0]
	LDR		X0, [X1]
	MOV		SP, X0

	LDP 	X2, X3, [SP], #0x10  /* Critical nesting and FPU context. */

	/* Set the PMR register to be correct for the current critical nesting
	depth. */
	LDR		X0, ullCriticalNestingConst /* X0 holds the address of ullCriticalNesting. */
	STR		X3, [X0]					/* Restore the task's critical nesting count. */

	/* Restore the FPU context indicator. */
	LDR		X0, ullPortTaskHasFPUContextConst
	STR		X2, [X0]

	/* Restore the FPU context, if any. */
	CMP		X2, #0
	B.EQ	1f
	LDP		Q30, Q31, [SP], #0x20
	LDP		Q28, Q29, [SP], #0x20
	LDP		Q26, Q27, [SP], #0x20
	LDP		Q24, Q25, [SP], #0x20
	LDP		Q22, Q23, [SP], #0x20
	LDP		Q20, Q21, [SP], #0x20
	LDP		Q18, Q19, [SP], #0x20
	LDP		Q16, Q17, [SP], #0x20
	LDP		Q14, Q15, [SP], #0x20
	LDP		Q12, Q13, [SP], #0x20
	LDP		Q10, Q11, [SP], #0x20
	LDP		Q8, Q9, [SP], #0x20
	LDP		Q6, Q7, [SP], #0x20
	LDP		Q4, Q5, [SP], #0x20
	LDP		Q2, Q3, [SP], #0x20
	LDP		Q0, Q1, [SP], #0x20
1:
	LDP 	X2, X3, [SP], #0x10  /* SPSR and ELR. */

	/* Restore the SPSR. */
	MSR		SPSR_EL1, X3
	/* Restore the ELR. */
	MSR		ELR_EL1, X2

	LDP 	X30, XZR, [SP], #0x10
	LDP 	X28, X29, [SP], #0x10
	LDP 	X26, X27, [SP], #0x10
	LDP 	X24, X25, [SP], #0x10
	LDP 	X22, X23, [SP], #0x10
	LDP 	X20, X21, [SP], #0x10
	LDP 	X18, X19, [SP], #0x10
	LDP 	X16, X17, [SP], #0x10
	LDP 	X14, X15, [SP], #0x10
	LDP 	X12, X13, [SP], #0x10
	LDP 	X10, X11, [SP], #0x10
	LDP 	X8, X9, [SP], #0x10
	LDP 	X6, X7, [SP], #0x10
	LDP 	X4, X5, [SP], #0x10
	LDP 	X2, X3, [SP], #0x10
	LDP 	X0, X1, [SP], #0x10

	/* Switch to use the ELx stack pointer.  _RB_ Might not be required. */
	MSR 	SPSEL, #1

	ERET

	.endm

/******************************************************************************
 * vPortRestoreTaskContext is used to start the scheduler.
 *****************************************************************************/
.align 8
.type vPortRestoreTaskContext, %function
vPortRestoreTaskContext:
.set freertos_vector_base,	_freertos_vector_table

	/* Install the FreeRTOS interrupt handlers. */
	LDR		X1, =freertos_vector_base
	MSR		VBAR_EL1, X1
	DSB		SY
	ISB		SY

	/* Start the first task. */
	portRESTORE_CONTEXT

.align 8
pxCurrentTCBConst: .dword pxCurrentTCB
ullCriticalNestingConst: .dword ullCriticalNesting
ullPortTaskHasFPUContextConst: .dword ullPortTaskHasFPUContext
.end
