/**
 * boot/bootstrap.s
 * 
 * Contains the code switching to longmode. Function '_entry' is the entry point for the linker.
 * The bootloader currently has to be multiboot compliant and put the machine into protected mode.
**/

#define ASM

.global			_entry
.global			_pml4, _pdpe_identity, _pdpe_kernel
.extern			_kernel_entry

// Multiboot2 header constants
.set			MAGIC,					    0xE85250D6			// Magic number
.set			ARCHITECTURE,			    0
// Set the entry address for higher-half kernel
.set			KERNEL_VIRTUAL_OFFSET,	    0xFFFFFFFF80000000
.set			KERNEL_PML4_INDEX,		    511//KERNEL_VIRTUAL_OFFSET >> 38
// Constants for paging structure
.set            LONGMODE_MSR,               0xC0000080
.set			CR0_PAGING,				    0x80000000
.set			CR4_PSE,				    0x00000010
// Paging flags
.set			PAGING_PRESENT,	 			0x0000000000000001
.set			PAGING_RW,			 		0x0000000000000002
.set			PAGING_USER,				0x0000000000000004
.set			PAGING_WRITE_THROUGH,		0x0000000000000008
.set			PAGING_LARGE_SIZE,			0x0000000000000080
.set			PAGING_GLOBAL,				0x0000000000000100
.set			PAGING_PAT,					0x0000000000001000
.set			PAGING_NX,					0x8000000000000000
// Page sizes
.set			PAGING_1GB_PAGE_SIZE,	    0x40000000
.set			PAGING_2MB_SIZE,		    0x00200000
.set			PAGING_4KB_SIZE,		    0x00001000
// Paging structure sizes and entry counts
.set			PAGING_PML4_ENTRIES,		512
.set			PAGING_PDP_ENTRIES,			512
.set			PAGING_PD_ENTRIES,			512
.set			PAGING_PT_ENTRIES,			512
.set			PAGING_PML4_ENTRY_SIZE,	    8
.set			PAGING_PDP_ENTRY_SIZE,	    8
.set			PAGING_PD_ENTRY_SIZE,	    8
.set			PAGING_PT_ENTRY_SIZE,	    8
// Physical addresses of paging structures
.set            _pml4_phys,                 0x1000
.set            _pdpe_identity_phys,        0x2000
.set            _pdpe_kernel_phys,          0x3000

// First part of the code is 32 bits since we're still in protected mode
.code32

// Multiboot section
.section		.multiboot, "ax", @progbits

.align			8
_multibootHeaderStart:
.long			MAGIC
.long			ARCHITECTURE
.long			_multibootHeaderEnd - _multibootHeaderStart
.long			-(MAGIC + ARCHITECTURE + (_multibootHeaderEnd - _multibootHeaderStart))
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
.section .bootstrap, "ax", @progbits

_entry:
    cli

	// Setup initial kernel stack
	movl		$temp_kernel_stack_end, %esp

	push		%eax
	push		%ebx

    // Set initial page directory
    movl        $(_pml4 - KERNEL_VIRTUAL_OFFSET), %ecx
    movl        %ecx, %cr3

    // Set PAE paging bit
    movl        %cr4, %eax
    orl         $(1 << 5), %eax
    movl        %eax, %cr4
    
    // Set longmode bit
    movl        $LONGMODE_MSR, %ecx
    rdmsr
    orl         $(1 << 8), %eax
    wrmsr
enable_paging:
    // Enable paging
    movl        %cr0, %eax
    orl         $(1 << 31), %eax
    movl        %eax, %cr0
    
    // Restore multiboot data
    pop			%ebx
    pop			%eax
    
    // Enter 64-bit
    lgdt		(gdt64Ptr)
    jmp			$8, $longmode_temp

temp_kernel_stack:
    .skip       0x8
temp_kernel_stack_end:

gdt64:
    // Null descriptor
    .short      0
    .short      0
    .byte       0
    .byte       0
    .byte       0
    .byte       0
    // Code descriptor
gdt64Code:
    .short      0
    .short      0
    .byte       0
    .byte       0b10011010
    .byte       0b00100000
    .byte       0
    // Data descriptor
    .short      0
    .short      0
    .byte       0
    .byte       0b10010010
    .byte       0b00000000
    .byte       0
gdt64Ptr:
    .short      (gdt64Ptr - gdt64 - 1)
    .quad		gdt64


// Small 64-bit stub to actually jump to 64 bit longmode
.code64 
longmode_temp:
    movabs      $_kernel_entry, %rcx
    jmp         *%rcx


// Data segment
.section .data

.align			4096

// First entry here is for identity mapping the physical memory
// Other entry is for mapping the kernel virtual memory at -2GB back to address 0 physical memory
_pml4:
	.quad		(_pdp - KERNEL_VIRTUAL_OFFSET) + (PAGING_PRESENT + PAGING_RW)		// Maps 0x0000'0000'0000 - 0x007F'FFFF'FFFF
	.skip		PAGING_PML4_ENTRY_SIZE * PAGING_PML4_ENTRIES - 2, 0
    .quad       (_pdp - KERNEL_VIRTUAL_OFFSET) + (PAGING_PRESENT + PAGING_RW)		// Maps 0xFF80'0000'0000 - 0xFFFF'FFFF'FFFF

// First entry here is for identity mapping the physical memory
// Other entry is for mapping the kernel virtual memory at -2GB back to address 0 physical memory
_pdp:
	.quad		(_pd - KERNEL_VIRTUAL_OFFSET) + (PAGING_PRESENT + PAGING_RW)		// Maps 0x0000'0000'0000 - 0x0000'3FFF'FFFF
	.skip		PAGING_PDP_ENTRY_SIZE * PAGING_PDP_ENTRIES - 3, 0
	.quad		(_pd - KERNEL_VIRTUAL_OFFSET) + (PAGING_PRESENT + PAGING_RW)		// Maps 0x007F'8000'0000 - 0x007F'BFFF'FFFF
	.skip		1, 0

_pd:
	.quad		0x00000000 + (PAGING_PRESENT + PAGING_LARGE_SIZE + PAGING_RW)		// Maps 0x0000'0000'0000 - 0x0000'001F'FFFF
	.quad		0x00200000 + (PAGING_PRESENT + PAGING_LARGE_SIZE + PAGING_RW)		// Maps 0x0000'0020'0000 - 0x0000'003F'FFFF
	.quad		0x00400000 + (PAGING_PRESENT + PAGING_LARGE_SIZE + PAGING_RW)		// Maps 0x0000'0040'0000 - 0x0000'005F'FFFF
	.skip		PAGING_PD_ENTRY_SIZE * PAGING_PD_ENTRIES - 3, 0
