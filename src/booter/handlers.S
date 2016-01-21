	.align 4
	.globl timer_handler
	.globl keyboard_handler

timer_handler:
	pusha	# Save registers
	cld

	call	timer_event

	mov	$0x20, %al
	out	%al, $0x20

	popa	# Restore registers
	iret

keyboard_handler:
	pusha	# Save registers
	cld

	call	keyboard_event

	mov	$0x20, %al
	out	%al, $0x20

	popa	# Restore registers
	iret
