
.global _aquire_spinlock, _release_spinlock

// Check whether the lock is lockable; if not, busy-wait until it is
_aquire_spinlock:
	movl		8(%esp), %eax
.aquire:
	cli							// Disable interrupts
	lock btsl	$0, (%eax)
	jc			.spin
	
	ret

.spin:
	sti							// Enable interrupts to check for IRQs and such
	pause
	testl		$1, (%eax)
	jnz			.spin
	jmp			.aquire

// Release a (locked or not) spinlock
_release_spinlock:
	movl		8(%esp), %eax
	movl		$0, (%eax)
	ret
