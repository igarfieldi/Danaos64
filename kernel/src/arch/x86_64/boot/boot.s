/**
 * boot/boot.s
 * 
 * Contains the startup code for the kernel. Function '_start' is the entry point for the linker.
 * The bootloader currently has to be multiboot compliant and put the machine into protected mode.
 * Furthermore, the memory for the kernel stack is reserved.
**/

#define ASM

.global			_entry, _start
.global			_pml4, _pdpe_identity, _pdpe_kernel

// Multiboot2 header constants
.set			MAGIC,					    0xE85250D6			// Magic number
.set			ARCHITECTURE,			    0
// Kernel constants
.set			KERNEL_STACK_SIZE,		    32768
// Set the entry address for higher-half kernel
.set			KERNEL_VIRTUAL_OFFSET,	    0xFFFFFFFF80100000
.set			KERNEL_PML4_INDEX,		    511//KERNEL_VIRTUAL_OFFSET >> 38
// Constants for paging structure
.set            LONGMODE_MSR,               0xC0000080
.set			CR0_PAGING,				    0x80000000
.set			CR4_PSE,				    0x00000010
.set			PAGING_PML4_PRESENT_RW, 	0x00000003
.set			PAGING_PDPE_PRESENT_1GB_RW,	0x00000083
.set			PAGING_1GB_SIZE,		    0x40000000
.set			PAGING_PML4_ENTRIES,		512
.set			PAGING_PDPE_ENTRIES,		512
.set			PAGING_PML4_ENTRY_SIZE,	    8
.set			PAGING_PDPE_ENTRY_SIZE,	    8
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

	// Identity-map everything until the kernel memory space
	movl		$_pdpe_identity_phys, %edi
	movl		$PAGING_PDPE_PRESENT_1GB_RW, %edx
	movl		$KERNEL_PML4_INDEX, %ecx
identity_map:
	movl		%edx, (%edi)
	addl		$PAGING_1GB_SIZE, %edx
	addl		$PAGING_PDPE_ENTRY_SIZE, %edi
	dec			%ecx
	jnz			identity_map

	// Map the kernel memory space to the beginning of physical memory
	movl		$_pdpe_kernel_phys, %edi
	movl		$PAGING_PDPE_PRESENT_1GB_RW, %edx
	movl		$(PAGING_PDPE_ENTRIES - KERNEL_PML4_INDEX), %ecx
.kernel_map:
	movl		%edx, (%edi)
	addl		$PAGING_1GB_SIZE, %edx
	addl		$PAGING_PDPE_ENTRY_SIZE, %edi
	dec			%ecx
	jnz			.kernel_map

.set_pml4_entries:
    movl        $_pml4_phys, %eax
    movl        $_pdpe_identity_phys, (%eax)
    addl        $PAGING_PML4_PRESENT_RW, (%eax)
    movl        $_pdpe_kernel_phys, 4088(%eax)
    addl        $PAGING_PML4_PRESENT_RW, 4088(%eax)

after_mapping:
	
    // Set initial page directory
    movl        $_pml4_phys, %ecx
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
    movabs      $longmode, %rcx
    jmp         *%rcx

// Kernel entry point
.section        .text
.code64
longmode:
	// Re-set the stack
    movabs      $KERNEL_STACK_SIZE, %rcx
    movabs      $kernel_stack, %rsp
	addq		%rcx, %rsp
	
_callKernel:
	push        %rax
	push        %rbx
    
	// Let gcc handle the constructor calls
    call        _init
    
    pop         %rbx
    pop         %rax
    movq        %rax, %rdi
    movq        %rbx, %rsi
    
	// Call kernel main function with multiboot info
    call        kernelMain
    
	
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
_pml4:
    .quad       _pdpe_identity_phys - KERNEL_VIRTUAL_OFFSET + PAGING_PML4_PRESENT_RW
	.skip		PAGING_PML4_ENTRY_SIZE * PAGING_PML4_ENTRIES - 2, 0
    .quad       _pdpe_kernel_phys - KERNEL_VIRTUAL_OFFSET + PAGING_PML4_PRESENT_RW

// Read-only data segment
.section .rodata

// Uninitialized data segment
.section .bss
// Reserve space for initial kernel stack
	.align		32
	.comm		kernel_stack, KERNEL_STACK_SIZE
