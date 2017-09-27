.code16

.global			_start
.global			drive_parameters, _hang

.set			BOOTLOADER_START,		0x7C00
.set			BOOTLOADER_STACK,		0x7C00
.set			KERNEL_TEMP_ADDR,		0x9000
.set			KERNEL_TEMP_SEGMENT,	0x0900
.set			KERNEL_COPY_ADDR,		0x1000000
.set			SEGMENT_SIZE,			16
.set			CODE_SEGMENT,			0x8
.set			DATA_SEGMENT,			0x10
.set			UNREAL_DATA_SEGMENT,	0x8

.set			MULTIBOOT2_MAGIC,		0x36D76289

.section		.bootsector, "ax", @progbits

_start:
	// Disable interrupts
	cli
	// Set the segment registers
	xorw		%ax, %ax
	movw		%ax, %ds
	movw		%ax, %es
	movw		%ax, %fs
	movw		%ax, %gs
	movw		%ax, %ss
	movw		$BOOTLOADER_STACK, %sp
	
	// Save the info passed by the MBR
	movw		%dx, (boot_drive)
	movw		%si, (boot_partition_entry)
	
	// Go to unreal mode (TODO: is this really needed?)
	push		%ds
	push		%es
	lgdt		(unreal_gdt_descriptor)				// Load the rudimentary GDT
	movl		%cr0, %eax							// Enable protected mode by setting the bit
	orl			$1, %eax
	movl		%eax, %cr0
	jmp			unreal_mode							// Jump for i386/i486 (TODO: is that really needed?)
unreal_mode:
	movw		$UNREAL_DATA_SEGMENT, %bx			// Set the right data segment descriptor index
	movw		%bx, %ds
	movw		%bx, %es
	
	// Back to real mode
	andb		$0xFE, %al
	movl		%eax, %cr0
	pop			%es
	pop			%ds
	

	// Enable interrupts since we need them
	sti
	
	// Query the BIOS for disk information (CHS)
	movb		$0x08, %ah
	int			$0x13
	jc			.read_failure
	movb		%dh, (drive_head_count)
	movb		%ch, (drive_cylinder_count_low)
	movb		%cl, (drive_cylinder_count_high)
	andb		$0xC0, (drive_cylinder_count_high)
	movb		%cl, (drive_sector_count)
	andb		$0x7F, (drive_sector_count)

	// Query the BIOS for more information (mainly the number of bytes per sector)
	movb		$0x48, %ah
	movw		(boot_drive), %dx
	movw		$drive_parameters, %si
	int			$0x13
	jc			.read_failure
	
load_bootloader:
	// Load the sectors from the boot drive
	movw		(bootloader_sectors), %ax
	subw		$1, %ax
	movb		3(%si), %ch								// Starting cylinder 0:7
	movb		2(%si), %cl								// Starting sector + cylinder 8:9
	addb		$1, %cl									// Increase sector by one because the first was loaded to begin with
	movw		%cx, (boot_sector_end)					// Save where the kernel should begin (sector right afterwards)
	addw		%ax, (boot_sector_end)
	movb		1(%si), %dh								// Starting head
	movw		(boot_drive), %dx						// Drive
	movw		$BOOTLOADER_START, %bx					// Target address
	addw		(drive_bytes_per_sector), %bx			// since the first segment is already loaded, go one past
	movb		$0x02, %ah								// BIOS interrupt number
	int			$0x13									// Warning: we rely on all sectors to be read at once!
	jc			.read_failure

	jmp			show_info

.read_failure:
	push		%ax
	movw		$msg_read_failure, %si
	call		_print_msg
	pop			%ax
	
	movb		%ah, %al
	xorb		%ah, %ah
	call		_print_number
	
.hang16:
	cli
	hlt
	jmp			.hang16

// Some stuff that needs to be in the first sector
boot_partition_entry:
	.word		0
boot_drive:
	.word		0
boot_sector_end:
	.word		0

drive_sector_count:
	.byte		0
drive_cylinder_count_low:
	.byte		0
drive_cylinder_count_high:
	.byte		0
drive_head_count:
	.byte		0

drive_parameters:
	.word		.drive_parameters_end - drive_parameters
drive_info_flags:
	.word		0
drive_phys_cylinders:
	.int		0
drive_phys_heads:
	.int		0
drive_phys_sectors:
	.int		0
drive_phys_sectors_total:
	.quad		0
drive_bytes_per_sector:
	.word		0
	.int		0					// Pointer to EDD (not used)
.drive_parameters_end:

// Used to get unreal mode (only needs data segment since we won't exceed 64k until we stay in actual protected mode)
unreal_gdt_descriptor:
	.word		unreal_gdt_end - unreal_gdt - 1
	.int		unreal_gdt
unreal_gdt:
	// Null segment
	.word		0
	.word		0
	.byte		0
	.byte		0
	.byte		0
	.byte		0
	// Data segment
	.word		0xFFFF
	.word		0
	.byte		0
	.byte		0x92
	.byte		0xCF
	.byte		0
unreal_gdt_end:

msg_read_failure:	.asciz "Failed to read media: "

// We're gonna waste some space, but guarantee that we don't overwrite any code
.fill			512 - 6 - (. - _start), 1, 0
bootloader_sectors:
	.word		0
kernel_sectors:
	.word		0								// Kernel size gets written here by IMG builder
boot_signature:
	.byte		0x55
	.byte		0xAA

// Beginning of second sector
show_info:
	// Print infos
	call		_clear_screen
	movw		$msg_loader_size, %si
	call		_print_msg
	movw		(bootloader_sectors), %ax
	call		_print_number
	call		_print_newline
	movw		$msg_boot_drive, %si
	call		_print_msg
	movw		(boot_drive), %ax
	call		_print_number
	call		_print_newline
	movw		$msg_boot_part, %si
	call		_print_msg
	movw		(boot_partition_entry), %ax
	call		_print_number
	call		_print_newline
	
	// Enable the A20 line (if it isn't done already)
	call		_enable_a20
	// Fetch the memory map
	call		_get_memmap
	
before_kernel_load:
	// Now load the kernel
	movw		(boot_partition_entry), %si
	movw		(boot_sector_end), %cx
	movb		1(%si), %dh								// Starting head
	movw		(boot_drive), %dx						// Drive
	movw		$KERNEL_TEMP_SEGMENT, %bx
	movw		%bx, %es								// We need to switch segments
	movw		$0x0, %bx
	movl		$KERNEL_COPY_ADDR, %edi
	
	// Load at most as many sectors at a time as there are per track

loop_kernel_load:
	movw		(kernel_sectors), %ax
	cmp			(drive_sector_count), %ax
	jl			.not_capped								// Ensure that we request 1 track at a time
	movw		(drive_sector_count), %ax
.not_capped:
	movb		$0x02, %ah								// BIOS interrupt number
	int			$0x13
	jc			.read_failure
	
	// Copy the kernel to where it is supposed to go
	// Save used registers
	push		%ax
	push		%cx
	push		%dx
	push		%ds
	push		%es
	
	movw		(drive_bytes_per_sector), %cx			// Get the number of bytes that should be copied
	mulw		%cx										// AX still contains the sectors copied
	movw		%ax, %cx
	xorw		%dx, %dx
	movw		%dx, %ds								// Clear the segment registers since we'll use the unreal mode 32 bit addressing
	movw		%dx, %es
	movl		$KERNEL_TEMP_ADDR, %esi					// Re-set the source index
	addr32 rep	movsb									// Copy the bytes; the operand prefix is important so that EDI/ESI will be used
	
	// Restore used registers
	pop			%es
	pop			%ds
	pop			%dx
	pop			%cx
	pop			%ax
	
	
	// Get the actually-read sector count
	xor			%ah, %ah
	subw		%ax, (kernel_sectors)
	
	// Check if more sectors need to be read
	cmp			$0, (kernel_sectors)
	je			after_kernel_load
	// Update the sector and cylinder stuff
	movw		$drive_sector_count, %si
	push		%ax
	call		_update_chs
	
	pop			%ax
	jmp			loop_kernel_load
	
after_kernel_load:
	// Restore proper segment
	movw		$0x0, %bx
	movw		%bx, %es

switch:
	// Switch to protected mode
	cli
	lgdt		(gdt_descriptor)
	movl		%cr0, %eax
	orl			$1, %eax
	movl		%eax, %cr0
	jmp			$CODE_SEGMENT, $protectedMode

.code32
protectedMode:
	movw		$DATA_SEGMENT, %ax
	movw		%ax, %ds
	movw		%ax, %es
	movw		%ax, %fs
	movw		%ax, %gs
	movw		%ax, %ss
	

elf:
	// Parse the ELF file to properly load the kernel
	push		$memmap
	push		$KERNEL_COPY_ADDR
	call		boot_main
	addl		$8, %esp
	
_hang:
	cli
	hlt
	jmp			_hang
	
gdt_descriptor:
	.word		gdt_end - gdt - 1
	.int		gdt
gdt:
	// Null segment
	.word		0
	.word		0
	.byte		0
	.byte		0
	.byte		0
	.byte		0
	// Code segment
codeseg:
	.word		0xFFFF
	.word		0
	.byte		0
	.byte		0x9A
	.byte		0xCF
	.byte		0
	// Data segment
	.word		0xFFFF
	.word		0
	.byte		0
	.byte		0x92
	.byte		0xCF
	.byte		0
gdt_end:

msg_boot_drive:		.asciz "Boot drive: "
msg_boot_part:		.asciz "Boot partition: "
msg_loader_size:	.asciz "Bootloader size: "

kernel_phys:
	.int		0x100000

