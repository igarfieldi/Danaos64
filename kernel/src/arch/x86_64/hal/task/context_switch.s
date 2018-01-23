.global		switch_context

/**
 * void switch_context(task_context *curr, task_context *next)
 */
switch_context:
	// We may clobber EAX and such since this is a regular function call, not an interrupt
	
	// Emulate interrupt stack setup
	// ISR frame
	movw		%ss, %ax
	pushq		%rax								// SS
	movq		%rsp, %rax
	subq		$16, %rax							// We need the RSP before this function was called (no RIP!)
	push		%rax								// Return RSP
	pushfq											// RFLAGS
	movw		%cs, %ax
	pushq		%rax								// CS
	pushq		32(%rsp)							// EIP
	subq		$16, %rsp							// Error code and interrupt number
	// Registers
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
	// Save new stack into current context
	movq		%rsp, (%rdi)
	// Now we need to swap to the other context's stack; we do that by pushing 
	// and popping its esp
	pushq		(%rsi)
	popq		%rsp
	
	// Now restore the rest of the registers, just like an ISR would
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
