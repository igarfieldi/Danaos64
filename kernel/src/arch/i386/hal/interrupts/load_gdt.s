.global _load_gdt

.set            CODE_SELECTOR, 0x08
.set            DATA_SELECTOR, 0x10

.section    .text
/*
 * void _load_gdt(descriptor gdt_desc)
**/
_load_gdt:
    push        %ebp
    movl        %esp, %ebp

    push        %eax

    // Load the GDT descriptor
    lgdt        8(%ebp)
    // Load the code segment selector by performing a long jump
    jmp         $CODE_SELECTOR, $flushSegmentSelectors

flushSegmentSelectors:
    // Set the remaining segment selectors to the data segment
    movw        $DATA_SELECTOR, %ax
    movw        %ax, %ds
    movw        %ax, %es
    movw        %ax, %fs
    movw        %ax, %gs
    movw        %ax, %ss

    pop         %eax

    pop         %ebp
    ret
