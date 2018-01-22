/**
 * boot/boot.s
 * 
 * Pushes necessary arguments for the kernel and calls the main function.
 * Gets called from bootstrap code with longmode enabled.
**/

#define ASM

.global			_kernel_entry
.extern         kernelMain, _init, _fini

// Kernel constants
.set			KERNEL_STACK_SIZE,		    32768

// Kernel entry point
.section        .text
.code64
_kernel_entry:
	// Re-set the stack
.test:
    hlt
    jmp         .test
    movabs      $KERNEL_STACK_SIZE, %rcx
    movabs      $kernel_stack, %rsp
	addq		%rcx, %rsp
	
.callKernel:
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


// Uninitialized data segment
.section .bss
// Reserve space for initial kernel stack
	.align		32
	.comm		kernel_stack, KERNEL_STACK_SIZE
