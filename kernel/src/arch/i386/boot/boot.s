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
.global			_phys_bitmap
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
.set			KERNEL_PD_INDEX,		KERNEL_VIRTUAL_OFFSET >> 22
// Constants for paging structure
.set			CR0_PAGING,				0x80000000
.set			CR4_PSE,				0x00000010
.set			PAGING_PRESENT_4MB_RW,	0x00000083
.set			PAGING_4MB_SIZE,		0x00400000
.set			PAGING_PD_ENTRIES,		1024
.set			PAGING_PD_ENTRY_SIZE,	4


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

	// Setup initial kernel stack
	movl		$(kernel_stack - KERNEL_VIRTUAL_OFFSET) + KERNEL_STACK_SIZE, %esp

	push		%eax
	push		%ebx

	// Identity-map everything until the kernel memory space
	movl		$(_page_directory - KERNEL_VIRTUAL_OFFSET), %edi
	movl		$PAGING_PRESENT_4MB_RW, %edx
	movl		$KERNEL_PD_INDEX, %ecx
.identity_map:
	movl		%edx, (%edi)
	addl		$PAGING_4MB_SIZE, %edx
	addl		$PAGING_PD_ENTRY_SIZE, %edi
	dec			%ecx
	jnz			.identity_map

	// Map the kernel memory space to the beginning of physical memory
	movl		$PAGING_PRESENT_4MB_RW, %edx
	movl		$(PAGING_PD_ENTRIES - KERNEL_PD_INDEX), %ecx
.kernel_map:
	movl		%edx, (%edi)
	addl		$PAGING_4MB_SIZE, %edx
	addl		$PAGING_PD_ENTRY_SIZE, %edi
	dec			%ecx
	jnz			.kernel_map

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

	// Re-set kernel stack to use virtual address
	pop			%ebx
	pop			%eax
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
	// Identity mapping
	.long		0*0x00400000 + PAGING_PRESENT_4MB_RW								// Maps 0x00000000 - 0x003FFFFFF
	.long		1*0x00400000 + PAGING_PRESENT_4MB_RW								// Maps 0x00400000 - 0x007FFFFFF
	.long		2*0x00400000 + PAGING_PRESENT_4MB_RW								// Maps 0x00800000 - 0x00BFFFFFF
	.skip		PAGING_PD_ENTRY_SIZE * (KERNEL_PD_INDEX-3), 0						// Doesn't map 0x01000000 - 0xBFFFFFFF
	// Re-map to the beginning
	.long		0*0x00400000 + PAGING_PRESENT_4MB_RW								// Maps 0xC0000000 - 0xC03FFFFFF
	.long		1*0x00400000 + PAGING_PRESENT_4MB_RW								// Maps 0xC0400000 - 0xC07FFFFFF
	.long		2*0x00400000 + PAGING_PRESENT_4MB_RW								// Maps 0xC0800000 - 0xC0BFFFFFF
	.skip		PAGING_PD_ENTRY_SIZE * (PAGING_PD_ENTRIES-KERNEL_PD_INDEX-3), 0		// Doesn't map 0xC0C00000 - 0xFFFFFFFF
	
// The initial bitmap for physical memory. We need this to solve the hen-egg-problem of finding
// memory for the bitmap but also needing to know where we can put it.
// This, however, inflates the kernel size
_phys_bitmap:
	.skip		0x20000, 0		// 4GB of RAM / (Page size * 8 bits per byte)

// Read-only data segment
.section .rodata

// Uninitialized data segment
.section .bss
// Reserve space for initial kernel stack
	.align		32
	.comm		kernel_stack, KERNEL_STACK_SIZE
