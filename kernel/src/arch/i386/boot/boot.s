/**
 * boot/boot.s
 * 
 * Contains the startup code for the kernel. Function '_start' is the entry point for the linker.
 * The bootloader currently has to be multiboot compliant and put the machine into protected mode.
 * Furthermore, the memory for the kernel stack is reserved.
**/

#define ASM

.global			_entry
.global			_page_directory
.extern			KERNEL_VIRT_OFFSET

.code32

.section		.text

// Multiboot2 header constants
.set			MAGIC,					0xE85250D6			// Magic number
.set			ARCHITECTURE,			0
// Kernel constants
.set			KERNEL_STACK_SIZE,		32768
// Set the entry address for higher-half kernel
.set			KERNEL_VIRTUAL_OFFSET,	0xC0000000
.set			KERNEL_PAGE_INDEX,		KERNEL_VIRTUAL_OFFSET >> 22
// Constants for paging structure
.set			CR0_PAGING,				0x80000000
.set			CR4_PSE,				0x00000010
.set			PAGING_PRESENT_4MB_RW,	0x00000083
.set			PAGING_PRESENT_4KB_RW,	0x00000003
.set			PAGING_DIR_ENTRIES,		1024
.set			PAGING_ENTRY_SIZE,		4


// Multiboot section
.section		.multiboot, "ax", @progbits
.align			8
_multibootHeaderStart:
	.int		MAGIC
	.int		ARCHITECTURE
	.int		_multibootHeaderEnd - _multibootHeaderStart
	.int		-(MAGIC + ARCHITECTURE + (_multibootHeaderEnd - _multibootHeaderStart))
	// Tag - Elf symbols
.align			8
_infoTagStart:
	.short		1
	.short		0
	.int		_infoTagEnd - _infoTagStart
	.int		2			// Request boot loader name
	.int		9			// Request ELF symbols
_infoTagEnd:
	// Final tag
	.align		8
	.short		0			// Type
	.short		0			// Flags
	.int		8			// Size
_multibootHeaderEnd:

// Kernel entry point
.section .text

_entry:
	cli

	// Load the page directory for identity mapping
	movl		$(_page_directory - KERNEL_VIRTUAL_OFFSET), %ecx
	movl		%ecx, %cr3

	// Enable PSE for 4MB pages
	movl		%cr4, %ecx
	orl			$CR4_PSE, %ecx
	movl		%ecx, %cr4


	// Enable paging
	movl		%cr0, %ecx
	orl			$CR0_PAGING, %ecx
	movl		%ecx, %cr0

	// Long jump to reload segment registers (not sure if needed...)
	movl		$higher_half, %ecx
	jmp			*%ecx

higher_half:
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

.align			4096
_page_directory:
	// Identity map first 4MB, then nothing, then map the virtual space back to the first 4MB
	.int		0x0000000 + PAGING_PRESENT_4MB_RW
	.int		0x0400000 + PAGING_PRESENT_4MB_RW
	.int		0x0800000 + PAGING_PRESENT_4MB_RW
	.int		0x0C00000 + PAGING_PRESENT_4MB_RW
	.int		0x1000000 + PAGING_PRESENT_4MB_RW
	.int		0x1400000 + PAGING_PRESENT_4MB_RW
	.skip		PAGING_ENTRY_SIZE * (KERNEL_PAGE_INDEX - 6), 0
	.int		PAGING_PRESENT_4MB_RW
	.skip		PAGING_ENTRY_SIZE * (PAGING_DIR_ENTRIES - KERNEL_PAGE_INDEX - 1), 0

// Read-only data segment
.section .rodata

// Uninitialized data segment
.section .bss
// Reserve space for initial kernel stack
	.align		32
	.comm		kernel_stack, KERNEL_STACK_SIZE
