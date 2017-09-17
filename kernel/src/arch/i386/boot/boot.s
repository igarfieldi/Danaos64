/**
 * boot/boot.s
 * 
 * Contains the startup code for the kernel. Function '_start' is the entry point for the linker.
 * The bootloader currently has to be multiboot compliant and put the machine into protected mode.
 * Furthermore, the memory for the kernel stack is reserved.
**/

#define ASM

.global			_entry, _start
.global			_pageDirectory

.code32

// Multiboot2 header constants

.set			MAGIC,					0xE85250D6			// Magic number
.set			ARCHITECTURE,			0

// Kernel constants
.set			KERNEL_STACK_SIZE,		32768

// Multiboot section
.align			8
.section .multiboot
_multibootHeaderStart:
.int			MAGIC
.int			ARCHITECTURE
.int			_multibootHeaderEnd - _multibootHeaderStart
.int			-(MAGIC + ARCHITECTURE + (_multibootHeaderEnd - _multibootHeaderStart))
// Tag - Elf symbols
.align			8
_infoTagStart:
.short			1
.short			0
.int			_infoTagEnd - _infoTagStart
.int			2			// Request boot loader name
.int			9			// Request ELF symbols
_infoTagEnd:
// Final tag
.align			8
.short			0			// Type
.short			0			// Flags
.int			8			// Size
_multibootHeaderEnd:

// Kernel entry point
.section .text

_entry:
	cli

	// Setup initial kernel stack
	movl		$kernel_stack + KERNEL_STACK_SIZE, %esp

	// Push the parameters for the main kernel function here already, since
	// '_init' may overwrite them
	push		%ebx					// Multiboot structure
	push		%eax					// Magic number
	
	// Let gcc handle the constructor calls
	call		_init
	
	// Call kernel main function with multiboot info
	call		kernelMain
	
	// Let gcc handle the desctructor calls 
	call		_fini
	
	// Disable interrupts and halt machine
	cli
.hang:
	hlt
	jmp			.hang

// Data segment
.section .data

// Read-only data segment
.section .rodata

// Uninitialized data segment
.section .bss
// Reserve space for initial kernel stack
	.align		32
	.comm		kernel_stack, KERNEL_STACK_SIZE
