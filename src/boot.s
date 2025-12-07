	%define	DISK_ID			0x12345678
	%define	kernel_location		0x0500

	org 0x7c00	; this code is loaded at 0x7c00 in memory
	bits 16		; targeting 16 bit

; start

	; load kernel
	mov	ah, 0x02
	mov	al, 10			; sector count to read
	mov	ch, 0			; cylinder
	mov	cl, 2			; sector
	mov	dh, 0			; head
	mov	bx, kernel_location	; buffer
					; drive select (already in dl)
	int	0x13			; interupt read disk (ah = 0x02)

	; switch video mode
	mov	ax, 0x0003	; ah = 0, al = video mode
	int	0x10

	; disable cursor
	mov	cx, 0x2607
	mov	ah, 0x01
	int	0x10

	cli	; disable interupts

	; set data segment to 0
	mov	ax, 0
	mov	ds, ax

	lgdt	[gdt_descriptor]

	; switch to protected mode
	mov	eax, cr0
	or	eax, 0x1
	mov	cr0, eax
	
	; jump to 32 bit code (clear pipeline)
	jmp CODE_SEG:start32


	; data for 16 bit part of the bootloaders
	; Global Descriptor Table
gdt_descriptor:
	dw	gdt_start - gdt_end - 1	; size
	dd	gdt_start		; start of gdt entries

gdt_start:

gdt_null:	; entry 0 (Null)
	dq	0		; filled with 0

gdt_code:	; entry 1 (Code segment)
	dw	0xffff		; limit (0-15)
	dw	0x0000		; base (0-15)
	db	0x00		; base (16-23)
	db	0b10011010	; access byte
	db	0b11001111	; b7-4: flags | b3-0: limit (16-19)
	db	0x00		; base (24-31)

gdt_data:	; entry 2 (Data segment)
	dw	0xffff		; limit (0-15)
	dw	0x0000		; base (0-15)
	db	0x00		; base (16-23)
	db	0b10010010	; access byte
	db	0b11001111	; limit (16-19) | flags
	db	0x00		; base (24-31)
gdt_end:

boot_disk:
	db	0

	; get offset for segments
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start


	; protected mode section
	bits 32

start32:
	; setup segmentation
	mov	ax, DATA_SEG
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	jmp	kernel_location

; fill the file with 0
times 440-($-$$) db 0

	; MBR descrpition table

	dd	DISK_ID		; disk ID
	dw	0x0000		; Optional / reserved?

	; 1st partition entry
part1:	db	0x80		; partition attributes (Bit 7 = bootable)

	db	0x00		; Starting head
	db	0x20		; Starting sector (Bits 6-7 are the upper two bits for the Starting Cylinder field.)
	db	0x20		; Starting Cylinder

	db	0x0c		; Filesystem

	db	0x20		; Ending head
	db	0xa7		; Ending sector (Bits 6-7 are the upper two bits for the Ending Cylinder field.)
	db	0x28		; Ending Cylinder
	
	dw	0x0800, 0x0000	; Relative sector (Words are inverted)
	dw	0x07ff, 0x0020	; Total sectors

	; partition 1 is created
	; 1GiB, FAT32

; other partitions are left empty
; 2nd partition entry
times 16 db 0

; 3rd partition entry
times 16 db 0

; 4th partition entry
times 16 db 0

	; bootable signature
dw	0xaa55
