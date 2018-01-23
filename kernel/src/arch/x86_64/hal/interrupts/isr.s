# Creates interrupt wrappers which call the 'real' ISR

.global			_isr_addresses

.set			ISR_WRAPPER_COUNT, 256
.set			EXCEPTION_COUNT, 32			// Amount of exceptions
.set			ISR_WRAPPER_SIZE, 0x10		// 16 bytes for any wrapper

.section		.text

.macro gen_normal_isr
	.align		16					// We have to align this as the wrapper size apparently varies...
isr_wrapper_\@:
    pushq		$0					// To replace the missing error code, we push a zero onto the stack
    pushq		$\@
	jmp			isr_caller
.endm

.macro gen_error_isr
	.align		16					// We have to align this as the wrapper size apparently varies...
isr_wrapper_\@:
    pushq		$\@
	jmp			isr_caller
.endm

// Gives us the addresses of our wrappers continuously
.altmacro
.macro isr_addrs curr end
	.quad		isr_wrapper_\curr
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
	// Save the registers mandated by CDECL (rest will be saved by callee if used)
	pushq		%rdi
	pushq		%rsi
	pushq		%r11
	pushq		%r10
	pushq		%r9
	pushq		%r8
	pushq		%rdx
	pushq		%rcx
	pushq		%rax
	pushq		%rbp
	pushq		%rsp

	// Argument will be interrupt state
	movq		%rsp, %rdi
	call		isr_handler							// External function calling the actual handler

_endIsr:

	// Restore the registers
	popq		%rsp
	popq		%rbp
	popq		%rax
	popq		%rcx
	popq		%rdx
	popq		%r8
	popq		%r9
	popq		%r10
	popq		%r11
	popq		%rsi
	popq		%rdi
	
	addq		$16, %rsp					// Remove error code and interrupt number from stack
	
	iretq

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
