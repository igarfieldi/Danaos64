.global		_printMsg, _printChar, _printNumber, _printNewline, _clearScreen

.code16
.section	.text

.type		_printChar, @function
_printChar:					// Print a single character	
	mov		1, %cx			// Repeat count
	mov		0, %bh			// Page number
	mov		$0x0E, %ah		// Function code for interrupt
	int		$0x10
	ret

.type		_printMsg, @function
_printMsg:					// Prints a string until it hits '\0'
	lodsb					// Load from si to al
	or		%al, %al		// Check if the current character is '\0' and thus end-of-string
	jz		.printMsgEnd		// If yes, jump to the end
	call	_printChar		// Otherwise print the current character and loop again
	jmp		_printMsg
.printMsgEnd:
	ret
	
.type		_printNumber, @function
_printNumber:
	xorw	%dx, %dx		// Zero-extend ax
	movw	$10, %cx
	div		%cx				// Divide by 10 to get the digit
	cmp		$0, %ax			// Check if we have a quotient of 0 (only remainder)
	jz		.printRemainder
	push	%dx
	call	_printNumber	// Not zero: need to print the quotient
	pop		%dx
	
.printRemainder:
	movb	%dl, %al		// Print the remainder (always between 0-9)
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
_clearScreen:				// Clears the screen by setting the video mode
	mov		$0x3, %al		// Video mode 3: 80x25
	mov		$0x0, %ah		// Function code for interrupt
	int		$0x10
	ret

