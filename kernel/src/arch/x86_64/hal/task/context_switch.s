.global		switch_context


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
	movq		(%rsp), %rax		// Return address (RIP)
	movq		%rax, 56(%rdi)

	// Load the state from the new context
	movq		(%rsi), %rbx
	movq		8(%rsi), %rbp
	movq		16(%rsi), %r12
	movq		24(%rsi), %r13
	movq		32(%rsi), %r14
	movq		40(%rsi), %r15
	movq		48(%rsi), %rsp
	movq		56(%rsi), %rax		// Return address (RIP)
	movq		%rax, (%rsp)

	ret
