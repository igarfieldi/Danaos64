.code16

.global			_start

.set			BOOTLOADER_START,		0x7C00
.set			BOOTLOADER_STACK,		0x0800
.set			KERNEL_ADDR,			0x9000
.set			KERNEL_SEGMENT,			0x0900
.set			SECTOR_SIZE,			0x0200
.set			SECTOR_SHIFT,			0x9
.set			MAX_SECTORS,			64
.set			SEGMENT_SIZE,			16
.set			SEGMENT_INCREASE,		0x800
.set			CODE_SEGMENT,			0x8
.set			DATA_SEGMENT,			0x10

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
	

load_bootloader:
	sti

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
	movw		$(BOOTLOADER_START + SECTOR_SIZE), %bx	// Target address: since the first segment is already loaded, go one past
	movb		$0x02, %ah								// BIOS interrupt number
	int			$0x13
	jc			.read_failure
	
after_load:
	
	// Print infos
	call		_clearScreen
	movb		$7, %bl
	movw		$msg_loader_size, %si
	call		_printMsg
	movw		(bootloader_sectors), %ax
	call		_printNumber
	call		_printNewline
	movw		$msg_boot_drive, %si
	call		_printMsg
	movw		(boot_drive), %ax
	call		_printNumber
	call		_printNewline
	movw		$msg_boot_part, %si
	call		_printMsg
	movw		(boot_partition_entry), %ax
	call		_printNumber
	call		_printNewline
	
before_kernel_load:
	// Now load the kernel
	movw		(boot_partition_entry), %si
	movw		(boot_sector_end), %cx
	movb		1(%si), %dh								// Starting head
	movw		(boot_drive), %dx						// Drive
	movw		$KERNEL_SEGMENT, %bx
	movw		%bx, %es								// We need to switch segments
	movw		$0x0, %bx
	
	// Load at most 64 sectors

// TODO: fix that cylinder bits are weird and not proper for larger kernels!
loop_kernel_load:
	movw		(kernel_sectors), %ax
	cmp			$MAX_SECTORS, %ax
	jl			not_capped
	movw		$MAX_SECTORS, %ax
not_capped:
	movb		$0x02, %ah								// BIOS interrupt number
	int			$0x13
	jc			.read_failure
	
after_interrupt:
	// Get the actually-read sector count
	xor			%ah, %ah
	subw		%ax, (kernel_sectors)
	
	// Check if more sectors need to be read
	cmp			$0, (kernel_sectors)
	je			after_kernel_load
	// Update the sector and cylinder stuff
	movw		$drive_sector_count, %si
	call		_updateCHS
	// Increase the memory location
	movw		%es, %bx
	addw		$SEGMENT_INCREASE, %bx
	movw		%bx, %es
	movw		$0x0, %bx
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
	
.read_failure:
	push		%ax
	movb		$0x8, %ah
	int			$0x13
after_test:
	movb		$7, %bl
	movw		$msg_read_failure, %si
	call		_printMsg
	pop			%ax
	
	movb		%ah, %al
	xorb		%ah, %ah
	call		_printNumber
	
.hang16:
	cli
	hlt
	jmp			.hang16

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
	push		$KERNEL_ADDR
	call		parse_elf
	addl		$4, %esp
	movl		%eax, (kernel_entry)
	
	cmp			$0, %eax
	je			.hang32

jump_kernel:
	movl		(kernel_entry), %edi
	// Jump to the kernel
	jmp			*%edi
	
.hang32:
	cli
	hlt
	jmp			.hang32

boot_partition_entry:
	.word		0
boot_drive:
	.word		0
boot_sector_end:
	.word		0

msg_boot_drive:		.asciz "Boot drive: "
msg_boot_part:		.asciz "Boot partition: "
msg_loader_size:	.asciz "Bootloader size: "
msg_read_failure:	.asciz "Failed to read media: "

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

drive_sector_count:
	.byte		0
drive_cylinder_count_low:
	.byte		0
drive_cylinder_count_high:
	.byte		0
drive_head_count:
	.byte		0

kernel_entry:
	.int		0
kernel_phys:
	.int		0x100000

.fill			512 - 6 - (. - _start), 1, 0
bootloader_sectors:
	.word		0x04
kernel_sectors:
	.word		0x78								// Kernel size gets written here by IMG builder
boot_signature:
	.byte		0x55
	.byte		0xAA

