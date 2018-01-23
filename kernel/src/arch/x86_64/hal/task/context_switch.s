.global		switch_context
.global		start_context


/**
 * void switch_context(thread_context *curr, thread_context *next)
 */
switch_context:
	// Save the current state into the old context
	movq		%rbx, (%rdi)
	movq		%rbp, 8(%rdi)
	movq		%r12, 16(%rdi)
	movq		%r13, 24(%rdi)
	movq		%r14, 32(%rdi)
	movq		%r15, 40(%rdi)
	movq		%rsp, 48(%rdi)

	// Load the state from the new context
	movq		(%rsi), %rbx
	movq		8(%rsi), %rbp
	movq		16(%rsi), %r12
	movq		24(%rsi), %r13
	movq		32(%rsi), %r14
	movq		40(%rsi), %r15
	movq		48(%rsi), %rsp

	ret
	
/**
 * void start_context()
 */
start_context:
	popq		%rdi				// First on stack: task object => needs to go into RDI
	ret								// Second on stack: return address == start function
	
