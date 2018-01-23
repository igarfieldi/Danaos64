.global		switch_context

/**
 * void switch_context(task_context *curr, task_context *next)
 */
switch_context:
	// We may clobber EAX and such since this is a regular function call, not an interrupt
	
	movl		4(%esp), %eax						// Current context
	movl		8(%esp), %ecx						// Next context
	
	// Emulate interrupt stack setup
	// ISR frame
	pushfl											// EFLAGS
	push		%cs									// CS
	pushl		8(%esp)								// EIP
	subl		$8, %esp							// Error code and interrupt number
	// Registers
	pushl		%edx
	pushl		%ecx
	pushl		%eax
	pushl		%ebp
	// Save new stack into current context
	movl		%esp, (%eax)
	// Now we need to swap to the other context's stack; we do that by pushing 
	// and popping its esp
	pushl		(%ecx)
	popl		%esp
	
	// Now restore the rest of the registers, just like an ISR would
	popl		%ebp
	popl		%eax
	popl		%ecx
	popl		%edx
	
	addl		$8, %esp					// Remove error code and interrupt number from stack
	
	
	iret
