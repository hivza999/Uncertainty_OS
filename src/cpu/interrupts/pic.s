	bits 32

	%define	PIC1_new_int	0x20
	%define	PIC2_new_int	0x28

extern remap_pic
remap_pic:

	; start initialization sequence in cascade mode
	mov	al, 0x11
	out	0x20, al
	out	0xa0, al

	; send new int value
	mov	al, PIC1_new_int
	out	0x21, al
	mov	al, PIC2_new_int
	out	0xa1, al

	; lin PICs together
	mov	al, 0000_0100
	out	0x21, al
	mov	al, 2
	out	0xa1, al

	; go into 8086 mode
	mov	al, 0x01
	out	0x21, al
	mov	al, 0x01
	out	0xa1, al

	ret