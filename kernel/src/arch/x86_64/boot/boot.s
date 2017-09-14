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

// Kernel constants
.set			KERNEL_STACK_SIZE,		32768

// Kernel entry point
.section .text

// First part of the code is 32 bits since we're still in protected mode
.code32

_entry:
	cli

	// Setup initial kernel stack
	movl		$kernel_stack + KERNEL_STACK_SIZE, %esp
_enableLongmode:
    // Set initial page directory
    movl        $pml4t, %eax
    movl        %eax, %cr3
    
    // Set PAE paging bit
    movl        %cr4, %eax
    orl         $(1 << 5), %eax
    movl        %eax, %cr4
    
    // Set longmode bit
    movl        $0xC0000080, %ecx
    rdmsr
    orl         $(1 << 8), %eax
    wrmsr
 
	jmp			.hang
    
    // Enable paging
    movl        %cr0, %eax
    orl         $(1 << 31), %eax
    movl        %eax, %cr0
    
    // Enter 64-bit
    lgdt		(gdt64Ptr)
.code64
	// Re-set the stack
	movq		$kernel_stack + KERNEL_STACK_SIZE, %rsp
	
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

.align			0x1000
pml4t:
    .long       pdpt + 0x3
    .skip       0x1000 - 4, 0
pdpt:
    .long       pdt + 0x3
    .skip       0x1000 - 4, 0
pdt:
    .long       pt + 0x3
    .skip       0x1000 - 4, 0
pt:
    .fill       512, 8, 0x3

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
    .short      gdt64Ptr - gdt64 - 1
    .quad       gdt64

// Read-only data segment
.section .rodata

// Uninitialized data segment
.section .bss
// Reserve space for initial kernel stack
	.align		32
	.comm		kernel_stack, KERNEL_STACK_SIZE
