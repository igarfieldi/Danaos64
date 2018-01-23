.global		switch_context
.global		start_context


/**
 * void switch_context(thread_context *curr, thread_context *next)
 */
switch_context:
	// Save the current state into the old context
	// TODO: other registers like ES, FS etc.
	movl		4(%esp), %eax
	movl		%ebx, (%eax)
	movl		%ebp, 4(%eax)
	movl		%edi, 8(%eax)
	movl		%esi, 12(%eax)
	movl		%esp, 16(%eax)

	// Load the state from the new context
	movl		8(%esp), %eax
	movl		(%eax), %ebx
	movl		4(%eax), %ebp
	movl		8(%eax), %edi
	movl		12(%eax), %esi
	movl		16(%eax), %esp
	
	ret

