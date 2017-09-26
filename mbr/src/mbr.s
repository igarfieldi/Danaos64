.code16

.global			_start

.set			MBR_SIZE,				0x200
.set			MBR_START,				0x7C00
.set			MBR_RELOCATED,			0x0600
.set			MBR_CODE_END, 			436

.set			BOOTLOADER_START,		0x7C00

.set			PT_SIZE,				16
.set			PT_COUNT,				4
.set			PT_ACTIVE,				0x80
.set			PT_OFF_ACTIVE,			0
.set			PT_OFF_START_HEAD,		1
.set			PT_OFF_START_SECTOR,	2
.set			PT_OFF_START_CYLINDER,	3
.set			PT_OFF_SYSTEM_ID,		4
.set			PT_OFF_END_HEAD,		5
.set			PT_OFF_END_SECTOR,		6
.set			PT_OFF_END_CYLINDER,	7
.set			PT_OFF_LBA,				8
.set			PT_OFF_TOTAL_SECTORS,	12


.section		.text

_start:
	jmp			actual_start
	nop
	
bpb:
	.ascii		"DanaOS  "
	.word		512
	.byte		1
	.word		1
	.byte		2
	.word		0
	.word		0
	.byte		0xF8
	.word		0
ebpb:
	.word		64
	.word		1
	.word		0	

actual_start:
	// Disable interrupts
	cli
	
	// Set the segment registers
	xorw		%ax, %ax
	movw		%ax, %ds
	movw		%ax, %es
	movw		%ax, %fs
	movw		%ax, %gs
	movw		%ax, %ss
	movw		%ax, %sp
	
	// Relocate MBR
	movw		$MBR_SIZE, %cx
	movw		$MBR_START, %si
	movw		$MBR_RELOCATED, %di
	rep movsb
	
	jmp			$0, $loader
	
loader:
	cli
	movb		%dl, (boot_drive)			// Save the boot drive
	
	movw		$partition_table, %bx
	movw		$PT_COUNT, %cx
find_partition:
	movb		(%bx), %al
	test		$PT_ACTIVE, %al
	jnz			partition_found
	add			$PT_SIZE, %bx
	dec			%cx
	jnz			find_partition
	sti
	movb		$7, %bl
	movw		$pt_missing_msg, %si
	call		_printMsg
	jmp			.hang
	
partition_found:
	sti
	movb		$PT_COUNT, %ch
	subb		%cl, %ch
	movb		%ch, (boot_partition)	// Save the boot partition index
	movw		%bx, %si				// Save the partition entry address for the bootloader
	
	movb		$1, %al					// Sectors to be read
	movb		3(%si), %ch				// Starting cylinder 0:7
	movb		2(%si), %cl				// Starting sector + cylinder 8:9
	movb		1(%si), %dh				// Starting head
	movb		(boot_drive), %dl		// Drive
	movw		$BOOTLOADER_START, %bx	// Target address
	movb		$0x02, %ah				// BIOS interrupt number
	int			$0x13
	
	// Restore boot drive for the bootloader (si contains the parition entry address already)
	movb		(boot_drive), %dl
	
	jmp			$0, $BOOTLOADER_START
	
.hang:
	cli
	hlt
	jmp			.hang
	
.type		_printChar, @function
_printChar:					// Print a single character	
	mov		1, %cx			// Repeat count
	mov		0, %bh			// Page number
	mov		$0x0E, %ah		// Function code for interrupt
	int		$0x10
	ret

.type		_printMsg, @function
_printMsg:					// Prints a string until it hits '\0'
	lodsb					// Load from si to al
	or		%al, %al		// Check if the current character is '\0' and thus end-of-string
	jz		printMsgEnd		// If yes, jump to the end
	call	_printChar		// Otherwise print the current character and loop again
	jmp		_printMsg
printMsgEnd:
	ret

boot_drive:
	.byte		0
boot_partition:
	.byte		0

pt_missing_msg:	.asciz "No active partition found!"

// End of code - beginning of MBR data
.fill			436 - (. - _start), 1, 0

// "Unique disk ID"
.fill			10, 1, 0
// Partition table (for testing! replace from a program to get an actually booting device)
partition_table:
.byte			0x80					// First partition: present and active (for testing)
.byte			0x0						// Starting head
.byte			0x2						// Starting sector + cylinder 8:9
.byte			0x0						// Starting cylinder 0:7
.byte			0x0						// System ID
.byte			0x0						// Ending head
.byte			0x1						// Ending sector + cylinder 8:9
.byte			0x0						// Ending cylinder 0:7
.int			0x0						// LBA
.int			0x0						// Total sectors
// 3 other partition entries (not present)
.fill			3 * PT_SIZE, 1, 0

// Boot signature
.byte			0x55
.byte			0xAA

