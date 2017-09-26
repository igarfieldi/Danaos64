/* Contains a20 related 16 bit real mode routines */
.code16

.global		_enable_a20

.set		ENABLE_A20_FUNCTION,			2401
.set		QUERY_A20_FUNCTION,				2403
.set		A20_INTERRUPT,					0x15
.set		BOOT_SIGNATURE_ADDRESS_LOW,		0x7DFE
.set		BOOT_SIGNATURE_ADDRESS_HIGH,	0x7DFE

.section	.text

msg_failed_a20:
	.asciz	"Fatal: couldn't enable A20 line!"

.type		_enable_a20, @function
_enable_a20:
	call	.is_a20_enabled										// Check we need to do anything
	jnc		.success_a20
	call	.enable_a20_bios									// Try the BIOS method
	call	.is_a20_enabled
	jnc		.success_a20
	
	// TODO: try more methods!
	movw		$msg_failed_a20, %si
	call		_print_msg
.hang:
	cli
	hlt
	jmp		.hang
	
.success_a20:
	ret

.type		.is_a20_enabled, @function
.is_a20_enabled:
	xorw	%ax, %ax
	movw	%ax, %ds
	movw	BOOT_SIGNATURE_ADDRESS_LOW, %dx						// Get data from lowest segment
	notw	%ax
	movw	%ax, %ds
	cmpw	BOOT_SIGNATURE_ADDRESS_HIGH, %dx					// Get data from highest segment and compare them
	jne		.a20_enabled										// If they aren't equal, A20 is enabled
	
	xorw	%ax, %ax
	movw	%ax, %ds
	rorw	BOOT_SIGNATURE_ADDRESS_LOW							// Do the same as before, but mutate the memory value before
	movw	BOOT_SIGNATURE_ADDRESS_LOW, %dx
	notw	%ax
	movw	%ax, %ds
	cmpw	BOOT_SIGNATURE_ADDRESS_HIGH, %dx					// Get data from highest segment and compare them again
	jne		.a20_enabled										// If they are equal again A20 is disabled
	
	stc															// Set carry flag to signal that A20 is disabled
	jmp		.is_a20_enabled_end
	
.a20_enabled:
	clc															// Clear carry flag to signal that A20 is enabled
	
.is_a20_enabled_end:
	xorw	%ax, %ax
	movw	%ax, %ds
	ret

.type		.enable_a20_bios, @function
.enable_a20_bios:
	movw	$QUERY_A20_FUNCTION, %ax							// Query if the BIOS even supports this method
	int		$A20_INTERRUPT
	jc		.enable_a20_bios_end
	cmpb	$0, %ah
	jne		.enable_a20_bios_end
	
	movw	$ENABLE_A20_FUNCTION, %ax							// Try to enable A20
	int		$A20_INTERRUPT
	
.enable_a20_bios_end:
	ret
	
