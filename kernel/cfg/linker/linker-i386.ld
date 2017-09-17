/* Designate entry point of the kernel (located in boot.s) */
ENTRY(_entry)
/* Specify the output format */
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
	. = 0x100000;

	/* Kernel code */
	.text ALIGN(4K) : AT(ADDR(.text))
	{
		*(.multiboot)
		*(.text)
	}

	/* Read-only data (constants etc.) */
	.rodata ALIGN(4K) : AT(ADDR(.rodata))
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data))
	{
		*(.data)
	}

	/* Read-write data (uninitialized). Initial kernel stack is also located here */
	.bss ALIGN(4K) : AT(ADDR(.bss))
	{
		*(COMMON)
		*(.bss)
	}

	.symtab :
	{
		SYMTAB_START = .;
        *(.symtab*)
		SYMTAB_END = .;
		*(.strtab*)
		*(.shstrtab*)
    }
}
