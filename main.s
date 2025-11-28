	%define	DISK_ID			0x12345678
	%define	Sector_per_track	63
	%define	Track_per_cylinder	1024
	%define	Number_of_head		16


	org 0x7c00	; this code is loaded at 0x7c00 in memory
	bits 16		; targeting 16 bit

; start
test:
	; clear registers
	xor	ax, ax	; 0 AX
	mov	ds, ax	; Set Data Segment to 0
	mov	es, ax	; Set Extra Segment to 0
	mov	ss, ax	; Set Stack Segment to 0
	mov	sp, ax	; Set Stack Pointer to 0


exit:	jmp	$	; stop execution


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
