/* Contains drive related 16 bit real mode routines */
.code16

.global		_get_chs, _update_chs, _tt

.section	.text
	
.type		_update_chs, @function
_update_chs:
	push	%bx
	movb	%cl, %bl
	andb	$0x3F, %bl				// Ignore the cylinder part of CL
	addb	%al, %bl
	cmpw	0(%si), %bx				// If we're past the sector count we need to increment the cylinder
	jle		.increase_sector
.increase_cylinder:
	movb	%ch, %bl
	movb	%cl, %bh
	andb	$0x3, %bh
	addw	$1, %bx					// Tentatively increase the cylinder index
	cmpw	1(%si), %bx				// Now check if we're past the limit
	jg		.increase_head
	
	movb	%bl, %ch
	movb	%bh, %cl				// Write back the changes
	jmp		.subtract_sector
	
.increase_head:
	andb	$0x7F, %cl
	xorb	%ch, %ch
	addb	$1, %dh					// No need to check since heads are limited to 254 and we can't do anything about it anyway
	
.subtract_sector:
	subb	0(%si), %al				// Subtract the sector count from the sector index to accomodate the increase in cylinder/head

.increase_sector:
	addb	%al, %cl
	
	pop		%bx
	ret
