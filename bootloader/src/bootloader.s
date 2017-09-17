.code16

.global			_start

.set			BOOTLOADER_START,		0x7C00
.set			BOOTLOADER_STACK,		0x0800
.set			SECTOR_SIZE,			0x0200
.set			SECTOR_SHIFT,			0x9

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
	
	
.hang:
	cli
	hlt
	jmp			.hang
	
.include		"screen.h"

boot_partition_entry:
	.word		0
boot_drive:
	.word		0

msg_boot_drive:		.asciz "Boot drive: "
msg_boot_part:		.asciz "Boot partition: "
msg_loader_size:	.asciz "Bootloader size: "

.fill			510 - (. - _start), 1, 0
.byte			0x55
.byte			0xAA

bootloader_end:

