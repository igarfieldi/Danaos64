/* Designate entry point of the kernel (located in boot.s) */
ENTRY(entry)
/* Specify the output format */
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	KERNEL_PHYS_OFFSET = 0x00100000;
	KERNEL_VIRT_OFFSET = 0xC0000000;

	. = KERNEL_PHYS_OFFSET + KERNEL_VIRT_OFFSET;

	KERNEL_VIRT_BEGIN = .;
	KERNEL_PHYS_BEGIN = KERNEL_VIRT_BEGIN - KERNEL_VIRT_OFFSET;

	/* Kernel code */
	.text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRT_OFFSET)
	{
		KERNEL_VIRT_CODE_BEGIN = .;
		KERNEL_PHYS_CODE_BEGIN = KERNEL_VIRT_CODE_BEGIN - KERNEL_VIRT_OFFSET;
		*(.multiboot)
		*(.text)
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
		KERNEL_VIRT_CODE_END = .;
		KERNEL_PHYS_CODE_END = KERNEL_VIRT_CODE_END - KERNEL_VIRT_OFFSET;
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRT_OFFSET)
	{
		KERNEL_VIRT_DATA_BEGIN = .;
		KERNEL_PHYS_DATA_BEGIN = KERNEL_VIRT_DATA_BEGIN - KERNEL_VIRT_OFFSET;
		*(.data)
	}

	/* Read-write data (uninitialized). Initial kernel stack is also located here */
	.bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRT_OFFSET)
	{
		*(COMMON)
		*(.bss)
		KERNEL_VIRT_DATA_END = .;
		KERNEL_PHYS_DATA_END = KERNEL_VIRT_DATA_END - KERNEL_VIRT_OFFSET;
	}

	/* Read-only data (constants etc.) */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRT_OFFSET)
	{
		KERNEL_VIRT_RODATA_BEGIN = .;
		KERNEL_PHYS_RODATA_BEGIN = KERNEL_VIRT_RODATA_BEGIN - KERNEL_VIRT_OFFSET;
		*(.eh_frame)
		*(.rodata)
	}

	.symtab : AT(ADDR(.symtab) - KERNEL_VIRT_OFFSET)
	{
        *(.symtab*)
		*(.strtab*)
		*(.shstrtab*)
		KERNEL_VIRT_RODATA_END = .;
		KERNEL_PHYS_RODATA_END = KERNEL_VIRT_RODATA_END - KERNEL_VIRT_OFFSET;
    }
    
    /DISCARD/ :
    {
    	*(.comment)
    }

	KERNEL_VIRT_END = .;
	KERNEL_PHYS_END = KERNEL_VIRT_END - KERNEL_VIRT_OFFSET;
}
