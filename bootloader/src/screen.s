/* Contains screen related 16 bit real mode routines */
.code16

.global		_clear_screen, _print_char, _print_msg, _print_number, _print_newline

.section	.text

.type		_clear_screen, @function	
_clear_screen:						// Clears the screen by setting the video mode
	mov		$0x3, %al				// Video mode 3: 80x25
	mov		$0x0, %ah				// Function code for interrupt
	int		$0x10
	ret

.type		_print_char, @function
_print_char:						// Print a single character	
	mov		$1, %cx					// Repeat count
	mov		$0, %bh					// Page number
	mov		$0x0E, %ah				// Function code for interrupt
	int		$0x10
	ret

.type		_print_msg, @function
_print_msg:							// Prints a string until it hits '\0'
	lodsb							// Load from si to al
	or		%al, %al				// Check if the current character is '\0' and thus end-of-string
	jz		.print_msg_end			// If yes, jump to the end
	call	_print_char				// Otherwise print the current character and loop again
	jmp		_print_msg
.print_msg_end:
	ret
	
.type		_print_number, @function
_print_number:
	xorw	%dx, %dx				// Zero-extend ax
	movw	$10, %cx
	div		%cx						// Divide by 10 to get the digit
	cmp		$0, %ax					// Check if we have a quotient of 0 (only remainder)
	jz		.print_remainder
	push	%dx
	call	_print_number			// Not zero: need to print the quotient
	pop		%dx
	
.print_remainder:
	movb	%dl, %al				// Print the remainder (always between 0-9)
	addb	$'0', %al
	call	_print_char
	ret
	
.type		_print_newline, @function
_print_newline:
	movb	$'\r', %al
	call	_print_char
	movb	$'\n', %al
	call	_print_char
	ret
