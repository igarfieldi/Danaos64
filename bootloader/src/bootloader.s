.code16

.global			_start

.set			BOOTLOADER_START,		0x7C00
.set			BOOTLOADER_STACK,		0x0800
.set			KERNEL_ADDR,			0x8000
.set			KERNEL_SEGMENT,			0x0800
.set			SECTOR_SIZE,			0x0200
.set			SECTOR_SHIFT,			0x9
.set			CODE_SEGMENT,			0x8
.set			DATA_SEGMENT,			0x10

.section		.text

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
	
	// TODO: relocation!
	
	// Since we cannot rely on the MBR to load all of the bootloader, do so know if needed
	movw		$(bootloader_end - _start), %ax
	movw		%ax, %bx								// Copy value for later
	shr			$SECTOR_SHIFT, %ax						// Right-shift instead of division: size / 512 => number of sectors
	andw		$(SECTOR_SIZE - 1), %bx					// Check for remainder
	cmp			$0, %bx
	jnz			load_bootloader							// If we don't have a remainder, we don't need to load another sector
	subw		$1, %ax									// But since the first sector is already in memory we need one less

load_bootloader:
	sti

	// Load the sectors from the boot drive
	
	movb		3(%si), %ch								// Starting cylinder 0:7
	movb		2(%si), %cl								// Starting sector + cylinder 8:9
	addb		$1, %cl									// Increase sector by one because the first was loaded to begin with
	movw		%ax, (boot_sector_end)
	addw		%cx, (boot_sector_end)
	movb		1(%si), %dh								// Starting head
	movw		(boot_drive), %dx						// Drive
	movw		$(BOOTLOADER_START + SECTOR_SIZE), %bx	// Target address: since the first segment is already loaded, go one past
	movb		$0x02, %ah								// BIOS interrupt number
	int			$0x13
	
	// Print infos (doesn't reside in the first sector for testing)
	call		_clearScreen
	movb		$7, %bl
	movw		$msg_loader_size, %si
	call		_printMsg
	movw		$(bootloader_end - _start), %ax
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
	
	// Now load the kernel (for testing)
	movw		(boot_partition_entry), %si
	movw		(kernel_sectors), %ax
	movw		(boot_sector_end), %cx
	movb		1(%si), %dh								// Starting head
	movw		(boot_drive), %dx						// Drive
	movw		$KERNEL_SEGMENT, %bx
	movw		%bx, %es								// We need to switch segments
	movw		$0x0, %bx								// Target address: since the first segment is already loaded, go one past
	movb		$0x02, %ah								// BIOS interrupt number
	int			$0x13
	
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
	
.include		"screen.h"

.code32
protectedMode:
	movw		$DATA_SEGMENT, %ax
	movw		%ax, %ds
	movw		%ax, %es
	movw		%ax, %fs
	movw		%ax, %gs
	movw		%ax, %ss

	// Copy the kernel from where we loaded it to where it is supposed to go
	movl		$KERNEL_ADDR, %esi
	movl		(kernel_entry), %edi
	movl		(kernel_sectors), %eax
	movl		$SECTOR_SIZE, %ecx
	mull		%ecx
	movl		%eax, %ecx
	rep			movsb

jump_kernel:
	movl		(kernel_entry), %edi
	// Jump to the kernel
	jmp			*%edi
	
.hang:
	cli
	hlt
	jmp			.hang

boot_partition_entry:
	.word		0
boot_drive:
	.word		0
boot_sector_end:
	.word		0

msg_boot_drive:		.asciz "Boot drive: "
msg_boot_part:		.asciz "Boot partition: "
msg_loader_size:	.asciz "Bootloader size: "

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

.fill			512 - 8 - (. - _start), 1, 0
kernel_sectors:
	.word		0x78								// Kernel size gets written here by IMG builder
kernel_entry:
	.int		0x100000
boot_signature:
	.byte			0x55
	.byte			0xAA

.align			512, 0

bootloader_end:

