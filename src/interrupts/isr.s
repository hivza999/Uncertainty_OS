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

isr_stub_9: ; keyboard input
	pushad

	mov	al, 9

	in	al, PS_2_data ; get scan code
	or	al, al

	js	skip_echo

	mov	edx, [0x2000]
	call	echo_hexb
	mov	[0x2000], edx

skip_echo:
	mov	al, 0x20
	out	0x20, al
	
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