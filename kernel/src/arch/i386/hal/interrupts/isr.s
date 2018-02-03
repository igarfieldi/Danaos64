# Creates interrupt wrappers which call the 'real' ISR

.global			_isr_addresses

.set			ISR_WRAPPER_COUNT, 256
.set			EXCEPTION_COUNT, 32			// Amount of exceptions
.set			ISR_WRAPPER_SIZE, 0x10		// 16 bytes for any wrapper

.section		.text

.macro gen_normal_isr
	.align		16					// We have to align this as the wrapper size apparently varies...
isr_wrapper_\@:
    pushl		$0					// To replace the missing error code, we push a zero onto the stack
    pushl		$\@
	jmp			isr_caller
.endm

.macro gen_error_isr
	.align		16					// We have to align this as the wrapper size apparently varies...
isr_wrapper_\@:
    pushl		$\@
	jmp			isr_caller
.endm

// Gives us the addresses of our wrappers continuously
.altmacro
.macro isr_addrs curr end
	.long		isr_wrapper_\curr
	.if \end - \curr
		isr_addrs %(\curr + 1), \end
	.endif
.endm

// Exceptions
gen_normal_isr	// 0
gen_normal_isr	// 1
gen_normal_isr	// 2
gen_normal_isr	// 3
gen_normal_isr	// 4
gen_normal_isr	// 5
gen_normal_isr	// 6
gen_normal_isr	// 7
gen_error_isr	// 8
gen_normal_isr	// 9
gen_error_isr	// 10
gen_error_isr	// 11
gen_error_isr	// 12
gen_error_isr	// 13
gen_error_isr	// 14
gen_normal_isr	// 15
gen_normal_isr	// 16
gen_error_isr 	// 17
gen_normal_isr	// 18
gen_normal_isr	// 19
gen_normal_isr	// 20
gen_normal_isr	// 21
gen_normal_isr	// 22
gen_normal_isr	// 23
gen_normal_isr	// 24
gen_normal_isr	// 25
gen_normal_isr	// 26
gen_normal_isr	// 27
gen_normal_isr	// 28
gen_normal_isr	// 29
gen_error_isr	// 30
gen_normal_isr	// 31

// Hard- and software interrupts
.rept				ISR_WRAPPER_COUNT - EXCEPTION_COUNT
	gen_normal_isr
.endr


isr_caller:
	cld												// Clear the direction bit in accordance
													// with C calling convention
	
	// Save registers (CPU state)
	pushl		%eax
	pushl		%ecx
	pushl		%edx
	pushl		%ebx
	pushl		%ebp
	pushl		%esi
	pushl		%edi
	// Save stack pointer
	pushl		%esp

	// Argument will be interrupt state
	pushl		%esp
	call		isr_handler							// External function calling the actual handler

_endIsr:
	addl		$4, %esp

	// Restore the stack and registers
	popl		%esp

	popl		%edi
	popl		%esi
	popl		%ebp
	popl		%ebx
	popl		%edx
	popl		%ecx
	popl		%eax
	
	addl		$8, %esp					// Remove error code and interrupt number from stack
	
	iret

.section		.data
.align			32
// Put the addresses into an array with the help of macros
// If the assembler fails with "too deeply nested" just split it up further
_isr_addresses:
	isr_addrs	0, 49
	isr_addrs	50, 99
	isr_addrs	100, 149
	isr_addrs	150, 199
	isr_addrs	200, 249
	isr_addrs	250, 255
