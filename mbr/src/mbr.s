.code16

.global			_start

.set			LOADER_SIZE,		0x200
.set			LOADER_START,		0x7C00
.set			LOADER_RELOCATED,	0x0600

.section		.text

_start:
	// Disable interrupts
	cli
	// Set the segment registers
	xorw		%ax, %ax
	movw		%ax, %ds
	movw		%ax, %es
	movw		%ax, %fs
	movw		%ax, %gs
	movw		%ax, %ss
	movw		%ax, %sp
	
	// Relocate MBR
	movw		$LOADER_SIZE, %cx
	movw		$LOADER_START, %si
	movw		$LOADER_RELOCATED, %di
	rep movsb
	
	jmp			$0, $loader
	
loader:
	sti
	call		_clearScreen

	movb		$7, %bl
	movw		$msg, %si
	call		_printMsg
	
.hang:
	cli
	hlt
	jmp			.hang
	
.include		"screen.h"
	
msg:			.asciz "Hello World!"

.fill			510 - (. - _start), 1, 0
.byte			0x55
.byte			0xAA

