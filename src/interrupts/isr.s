isr_stub_0:	; divisde by 0
	pushad
	mov	al, 0
	;call	exception_handler
	popad
	sti
	iret

isr_stub_1:
	pushad
	mov	al, 1
	call	exception_handler
	popad
	sti
	iret

isr_stub_2:
	pushad
	mov	al, 2
	call	exception_handler
	popad
	sti
	iret

isr_stub_3:
	pushad
	mov	al, 3
	call	exception_handler
	popad
	sti
	iret

isr_stub_4:
	pushad
	mov	al, 4
	call	exception_handler
	popad
	sti
	iret

isr_stub_5:
	pushad
	mov	al, 5
	call	exception_handler
	popad
	sti
	iret

isr_stub_6:
	pushad
	mov	al, 6
	call	exception_handler
	popad
	sti
	iret

isr_stub_7:
	pushad
	mov	al, 7
	call	exception_handler
	popad
	sti
	iret

isr_stub_8: ; Programmable Interval Timer
	pushad

	mov	al, 8
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret


	%define	PS_2_data	0x60
	%define	PS_2_cmd	0x64

	%define Scancode_set		0x9f100
	%define ASCII_layout		0x9f200

	%define Keycode_buffer		0x9f400
	%define Keycode_register	0x9f002
	%define Keyboard_status		0x9f000

	%define Keyboard_modifier_keys	0x9f001
	%define ASCII_input_register	0x9f003
	%define ASCII_buffer		0x9f500

isr_stub_9: ; keyboard input
	pushad

	xor	eax, eax
	xor	ebx, ebx
	xor	ecx, ecx

	in	al, PS_2_data
	; extended byte detected
	cmp	al, 0xe0
	je	irq1_extend_byte

	; Modifier keys:

	; pressed
	cmp	al, 0x1d	; ctrl
	je	irq1_modifierkey_crtl_p

	cmp	al, 0x38	; alt
	je	irq1_modifierkey_alt_p

	cmp	al, 0x2a	; shift
	je	irq1_modifierkey_shift_p

	cmp	al, 0x3a	; maj lock
	je	irq1_modifierkey_majlock_p

	; released
	cmp	al, 0x9d	; ctrl
	je	irq1_modifierkey_crtl_r

	cmp	al, 0xb8	; alt
	je	irq1_modifierkey_alt_r

	cmp	al, 0xaa	; shift
	je	irq1_modifierkey_shift_r

	cmp	al, 0xba	; maj lock
	je	irq1_modifierkey_majlock_r

irq1_modifierkeys_ret:

	; ignore if it is key release
	or	al, al
	js	irq1_release

	; check if it is a extended byte
	cmp	bl, 0b00000001
	je	irq1_extended_byte
irq1_extended_byte_ret:

	; reset extended byte
	mov	bl, [Keyboard_status]
	and	bl, 0b11111110
	mov	[Keyboard_status], bl

	; get keycode
	mov	cl, [Scancode_set + eax]

	; store keycode
	mov	bl, [Keycode_register]
	mov	[ebx + Keycode_buffer], cl
	
	inc	bl
	mov	[Keycode_register], bl


	; Get ASCII char
	; get char
	mov	al, [ASCII_layout + ecx]
	
	; store ASCII char
	mov	bl, [ASCII_input_register]
	mov	[ebx + ASCII_buffer], al
	
	inc	bl
	mov	[ASCII_input_register], bl

irq1_exit:

	mov	al, 0x20
	out	0x20, al
	
	popad
	sti
	iret

irq1_release:
	mov	bl, [Keyboard_status]
	and	bl, 0b11111110
	mov	[Keyboard_status], bl
	jmp	irq1_exit

irq1_extend_byte:
	mov	bl, [Keyboard_status]
	or	bl, 0b00000001
	mov	[Keyboard_status], bl
	jmp	irq1_exit

irq1_extended_byte:
	or	al, 0x80
	jmp	irq1_extended_byte_ret

	; pressed
irq1_modifierkey_crtl_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b00010000
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

irq1_modifierkey_alt_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b00100000
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

irq1_modifierkey_shift_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b01000000
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

irq1_modifierkey_majlock_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b10000000
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

	; released
irq1_modifierkey_crtl_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b11101111
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

irq1_modifierkey_alt_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b11011111
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

irq1_modifierkey_shift_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b10111111
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

irq1_modifierkey_majlock_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b01111111
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret

isr_stub_10:
	pushad
	mov	al, 10
	call	exception_handler
	popad
	sti
	iret

isr_stub_11:
	pushad
	mov	al, 11
	call	exception_handler
	popad
	sti
	iret

isr_stub_12:
	pushad
	mov	al, 12
	call	exception_handler
	popad
	sti
	iret

isr_stub_13:
	pushad
	mov	al, 13
	call	exception_handler
	popad
	sti
	iret

isr_stub_14:
	pushad
	mov	al, 14
	call	exception_handler
	popad
	sti
	iret

isr_stub_15:
	pushad
	mov	al, 15
	call	exception_handler
	popad
	sti
	iret

isr_stub_16:
	pushad
	mov	al, 16
	call	exception_handler
	popad
	sti
	iret

isr_stub_17:
	pushad
	mov	al, 17
	call	exception_handler
	popad
	sti
	iret

isr_stub_18:
	pushad
	mov	al, 18
	call	exception_handler
	popad
	sti
	iret

isr_stub_19:
	pushad
	mov	al, 19
	call	exception_handler
	popad
	sti
	iret

isr_stub_20:
	pushad
	mov	al, 20
	call	exception_handler
	popad
	sti
	iret

isr_stub_21:
	pushad
	mov	al, 21
	call	exception_handler
	popad
	sti
	iret

isr_stub_22:
	pushad
	mov	al, 22
	call	exception_handler
	popad
	sti
	iret

isr_stub_23:
	pushad
	mov	al, 23
	call	exception_handler
	popad
	sti
	iret

isr_stub_24:
	pushad
	mov	al, 24
	call	exception_handler
	popad
	sti
	iret

isr_stub_25:
	pushad
	mov	al, 25
	call	exception_handler
	popad
	sti
	iret

isr_stub_26:
	pushad
	mov	al, 26
	call	exception_handler
	popad
	sti
	iret

isr_stub_27:
	pushad
	mov	al, 27
	call	exception_handler
	popad
	sti
	iret

isr_stub_28:
	pushad
	mov	al, 28
	call	exception_handler
	popad
	sti
	iret

isr_stub_29:
	pushad
	mov	al, 29
	call	exception_handler
	popad
	sti
	iret

isr_stub_30:
	pushad
	mov	al, 30
	call	exception_handler
	popad
	sti
	iret

isr_stub_31:
	pushad
	mov	al, 31
	call	exception_handler
	popad
	sti
	iret


global isr_stub_table
isr_stub_table:
%assign i 0 
%rep	32
	dd	isr_stub_%+i
%assign i i+1 
%endrep


exception_handler:
	cli

	mov	byte [0xb8000], 'I'
	mov	byte [0xb8002], 'n'
	mov	byte [0xb8004], 't'

	mov	edx, 0xb8008
	call	echo_hexb

	ret

echo_hexb:
	mov	bl, al

	shr	al, 4
	call	echo_hex

	mov	al, bl
	and	al, 0x0f
	call	echo_hex
	
	ret

echo_hex:
	cmp	al, 10
	jge	echo_hex1

	add	al, '0'
	jmp	echo_hex2

echo_hex1:
	add	al, 'a'-10

echo_hex2:
	mov	[edx], al
	add	edx, 2 
	ret
