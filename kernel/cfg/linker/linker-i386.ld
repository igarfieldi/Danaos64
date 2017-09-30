/* Designate entry point of the kernel (located in boot.s) */
ENTRY(_entry)
/* Specify the output format */
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 0x100000;

	KERNEL_PHYS_BEGIN = .;
	/* Kernel code */
	.text ALIGN(4K) : AT(ADDR(.text))
	{
		KERNEL_PHYS_CODE_BEGIN = .;
		*(.multiboot)
		*(.text)
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
		KERNEL_PHYS_CODE_END = .;
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data))
	{
		KERNEL_PHYS_DATA_BEGIN = .;
		*(.data)
	}

	/* Read-write data (uninitialized). Initial kernel stack is also located here */
	.bss ALIGN(4K) : AT(ADDR(.bss))
	{
		*(COMMON)
		*(.bss)
		KERNEL_PHYS_DATA_END = .;
	}

	/* Read-only data (constants etc.) */
	.rodata ALIGN(4K) : AT(ADDR(.rodata))
	{
		KERNEL_PHYS_RODATA_BEGIN = .;
		*(.eh_frame)
		*(.rodata)
	}

	.symtab : AT(ADDR(.symtab))
	{
        *(.symtab*)
		*(.strtab*)
		*(.shstrtab*)
		KERNEL_PHYS_RODATA_END = .;
    }
    
    /DISCARD/ :
    {
    	*(.comment)
    }

	KERNEL_PHYS_END = .;
}
