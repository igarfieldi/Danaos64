/* Contains screen related 16 bit real mode routines */
.code16

.global		_get_memmap, memmap

.set		FUNCTION_MEMMAP,			0xE820
.set		MEMMAP_INTERRUPT,			0x15
.set		MEMMAP_ENTRY_SIZE,			24
.set		MAX_ENTRIES,				50
.set		SIGNATURE,					0x534D4150

.section	.text

.type		_get_memmap, @function
_get_memmap:
	movw	$memmap_entries, %di				// Initial buffer address (for testing)
	push	%di
	movl	$0, %ebx					// Initial continuation value
	
.memmap_loop:
	movl	$FUNCTION_MEMMAP, %eax
	movl	$MEMMAP_ENTRY_SIZE, %ecx
	movl	$SIGNATURE, %edx
	int		$MEMMAP_INTERRUPT
	
	jc		.memmap_failed				// Do error checking: no carry and proper signature
	cmp		$SIGNATURE, %eax
	jne		.memmap_failed
	
	cmp		$0, %ebx					// Check if we're at the end of the memory map
	je		.memmap_end
	
	addw	$MEMMAP_ENTRY_SIZE, %di
	jmp		.memmap_loop
	
.memmap_end:
	// Write size and version to the memory map
	movw	%di, (memmap_size)
	pop		%di
	subw	%di, (memmap_size)
	addl	$(memmap_entries - memmap), (memmap_size)
	movl	$1, (memmap_version)
	ret

.memmap_failed:
	pop		%di
	ret

memmap:
memmap_tag:
	.int	6
memmap_size:
	.int	0
memmap_entry_size:
	.int	MEMMAP_ENTRY_SIZE
memmap_version:
	.int	0
memmap_entries:
	.fill	MEMMAP_ENTRY_SIZE*MAX_ENTRIES, 1, 0
