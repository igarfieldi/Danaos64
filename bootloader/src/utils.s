.global		_printMsg, _printChar, _printNumber, _printNewline, _clearScreen, _updateCHS

.code16
.section	.text

.type		_printChar, @function
_printChar:							// Print a single character	
	mov		1, %cx					// Repeat count
	mov		0, %bh					// Page number
	mov		$0x0E, %ah				// Function code for interrupt
	int		$0x10
	ret

.type		_printMsg, @function
_printMsg:							// Prints a string until it hits '\0'
	lodsb							// Load from si to al
	or		%al, %al				// Check if the current character is '\0' and thus end-of-string
	jz		.printMsgEnd			// If yes, jump to the end
	call	_printChar				// Otherwise print the current character and loop again
	jmp		_printMsg
.printMsgEnd:
	ret
	
.type		_printNumber, @function
_printNumber:
	xorw	%dx, %dx				// Zero-extend ax
	movw	$10, %cx
	div		%cx						// Divide by 10 to get the digit
	cmp		$0, %ax					// Check if we have a quotient of 0 (only remainder)
	jz		.printRemainder
	push	%dx
	call	_printNumber			// Not zero: need to print the quotient
	pop		%dx
	
.printRemainder:
	movb	%dl, %al				// Print the remainder (always between 0-9)
	addb	$'0', %al
	call	_printChar
	ret
	
.type		_printNewline, @function
_printNewline:
	movb	$'\r', %al
	call	_printChar
	movb	$'\n', %al
	call	_printChar
	ret

.type		_clearScreen, @function	
_clearScreen:						// Clears the screen by setting the video mode
	mov		$0x3, %al				// Video mode 3: 80x25
	mov		$0x0, %ah				// Function code for interrupt
	int		$0x10
	ret
	
.type		_updateCHS, @function
_updateCHS:
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
	
