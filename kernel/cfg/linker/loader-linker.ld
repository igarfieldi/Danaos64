/* Designate entry point of the kernel (located in boot.s) */
ENTRY(_loaderentry)
/* Specify the output format */
OUTPUT_FORMAT(binary)

SECTIONS
{	
	//. = 0x10000;

	/* Kernel code */
	.text ALIGN(4K) : AT(ADDR(.text))
	{
		*(.init)
		*(.fini)
		*(.ctors)
		*(.dtors)
		*(.text)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data))
	{
		*(.data)
	}

	/* Read-only data (constants etc.) */
	.rodata ALIGN(4K) : AT(ADDR(.rodata))
	{
		*(.eh_frame)
		*(.rodata)
	}

	/* Read-write data (uninitialized). Initial kernel stack is also located here */
	.bss ALIGN(4K) : AT(ADDR(.bss))
	{
		*(COMMON)
		*(.bss)
	}

	.symtab : AT(ADDR(.symtab))
	{
		SYMTAB_START = .;
        *(.symtab*)
		SYMTAB_END = .;
		*(.strtab*)
		*(.shstrtab*)
    }
    
    /DISCARD/ :
    {
    	*(.comment)
    }
}
