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

isr_stub_8:
	pushad

	mov	al, 8
	call	exception_handler
	popad
	sti
	iret

isr_stub_9:
	pushad

	mov	al, 8
	call	exception_handler
	popad
	sti
	iret

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


isr_stub_irq0:	; Programmable Interval Timer
	pushad

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq1:	; Keyboard

	%define	PS_2_data	0x60
	%define	PS_2_cmd	0x64

	%define Scancode_set		0x90100
	%define ASCII_layout		0x90200

	%define Keycode_buffer		0x90400
	%define Keycode_register	0x90002
	%define Keycode_status		0x90000

	%define Keyboard_modifier_keys	0x90001
	%define ASCII_input_register	0x90003
	%define ASCII_buffer		0x90500

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

	cmp	al, 0x2a	; l shift
	je	irq1_modifierkey_shift_p
	cmp	al, 0x36	; r shift
	je	irq1_modifierkey_shift_p

	; released
	cmp	al, 0x9d	; ctrl
	je	irq1_modifierkey_crtl_r

	cmp	al, 0xb8	; alt
	je	irq1_modifierkey_alt_r

	cmp	al, 0xaa	; l shift
	je	irq1_modifierkey_shift_r
	cmp	al, 0xb6	; r shift
	je	irq1_modifierkey_shift_r

irq1_modifierkeys_ret1:

	; ignore if it is key release
	or	al, al
	js	irq1_release

	; check if it is a extended byte
	mov	bl, [Keycode_status]
	cmp	bl, 0b00000001
	je	irq1_extended_byte
irq1_extended_byte_ret:

	; reset extended byte
	and	bl, 0b11111110
	mov	[Keycode_status], bl

	; get keycode
	mov	cl, [Scancode_set + eax]

	; store keycode
	mov	bl, [Keycode_register]
	mov	[ebx + Keycode_buffer], cl
	
	inc	bl
	mov	[Keycode_register], bl


	; verify for numlock or capslock
	cmp	cl, 0x60	; capslock
	je	irq1_modifierkey_majlock

	cmp	cl, 0x31	; numlock
	je	irq1_modifierkey_numlock

irq1_modifierkeys_ret2:

	; Get ASCII char
	mov	ch, [Keyboard_modifier_keys]
	test	ch, 0x3
	jnz	irq1_exit


	mov	al, cl
	; b4: capslock modifier / b5: numlock
	shl	al, 3

	mov	ah, ch

	js	irq1_modifierkey_jmp0

	shr	ah, 4
	jmp	irq1_modifierkey_jmp1

irq1_modifierkey_jmp0:
	shr	ah, 5

irq1_modifierkey_jmp1:
	shr	ch, 2	; b2: shift modifier
	
	xor	ch, ah

	; ch > offset of 256
	and	ch, 1

get_char:
	xor	ebx, ebx
	; get char
	mov	al, [ASCII_layout + ecx]
	
	or	al, al
	jz	irq1_exit

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
	mov	bl, [Keycode_status]
	and	bl, 0b11111110
	mov	[Keycode_status], bl
	jmp	irq1_exit

irq1_extend_byte:
	mov	bl, [Keycode_status]
	or	bl, 0b00000001
	mov	[Keycode_status], bl
	jmp	irq1_exit

irq1_extended_byte:
	or	al, 0x80
	jmp	irq1_extended_byte_ret

	; pressed
irq1_modifierkey_crtl_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b0000_0001
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret1

irq1_modifierkey_alt_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b0000_0010
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret1

irq1_modifierkey_shift_p:
	mov	bl, [Keyboard_modifier_keys]
	or	bl, 0b0000_0100
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret1

	; released
irq1_modifierkey_crtl_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b1111_1110
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret1

irq1_modifierkey_alt_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b1111_1101
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret1

irq1_modifierkey_shift_r:
	mov	bl, [Keyboard_modifier_keys]
	and	bl, 0b1111_1011
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret1

	; toggle
irq1_modifierkey_majlock:
	mov	bl, [Keyboard_modifier_keys]
	xor	bl, 0b0001_0000
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret2

irq1_modifierkey_numlock:
	mov	bl, [Keyboard_modifier_keys]
	xor	bl, 0b0010_0000
	mov	[Keyboard_modifier_keys], bl
	jmp	irq1_modifierkeys_ret2

isr_stub_irq2:
	pushad

	mov	al, 0x32
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq3:
	pushad

	mov	al, 0x33
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq4:
	pushad

	mov	al, 0x34
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq5:
	pushad

	mov	al, 0x35
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq6:
	pushad

	mov	al, 0x36
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq7:
	pushad

	mov	al, 0x37
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq8:
	pushad

	mov	al, 0x38
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq9:
	pushad

	mov	al, 0x39
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq10:
	pushad

	mov	al, 0x3a
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq11:
	pushad

	mov	al, 0x3b
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq12:
	pushad

	mov	al, 0x3c
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq13:
	pushad

	mov	al, 0x3d
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq14:
	pushad

	mov	al, 0x3e
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

	popad
	sti
	iret

isr_stub_irq15:
	pushad

	mov	al, 0x3f
	call	exception_handler

	mov	al, 0x20
	out	0x20, al

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
 
%assign i 0
%rep	16
	dd	isr_stub_irq%+i
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
