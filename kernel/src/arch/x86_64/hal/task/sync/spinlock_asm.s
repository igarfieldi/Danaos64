
.global _aquire_spinlock, _release_spinlock

// Check whether the lock is lockable; if not, busy-wait until it is
_aquire_spinlock:
.aquire:
	pushf
	cli							// Disable interrupts
	lock btsl	$0, (%rdi)
	jc			.restore_int
	
	popf						// Restore interrupt state
	ret

.restore_int:
	popf						// Restore interrupt state to check for IRQs and such
.spin:
	pause
	testl		$1, (%rdi)
	jnz			.spin
	jmp			.aquire

// Release a (locked or not) spinlock
_release_spinlock:
	movl		$0, (%rdi)
	ret
