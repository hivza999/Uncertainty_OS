	%define	DISK_ID			0x12345678
	%define	kernel_location		0x0500
	%define	memory_map_location	0x8000

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

	; get memory map
	mov	di, (memory_map_location+4)

	clc
	xor	ebx, ebx
	mov	edx, 0x534d4150
	mov	eax, 0xe820
	mov	ecx, 24
	mov	es, bx
	mov	bp, bx		; count of entries

	mov	dword [es:di + 20], 1

	int	0x15
	jc	short mmap_Error		; if unsuported function
	mov	edx, 0x0534D4150
	cmp	eax, edx		; if success: eax = 0x0534D4150
	jne	short mmap_Error
	test	ebx, ebx		; ebx = 0 > list is 1 entry long, skip it
	je	short mmap_Error

	jmp	mmap_jmp

mmap_loop:
	mov eax, 0xe820
	mov [es:di + 20], dword 1
	mov ecx, 24
	int 0x15
	jc short mmap_finished	; carry flag > end of list
	mov edx, 0x0534D4150

mmap_jmp:
	jcxz	mmap_skip_entry
	cmp	al, 20			; 24 bytes > ACPI 3.X response
	jbe	mmap_not_extended

	; ACPI 3
	test	byte [es:di + 20], 1	; see if segment is present or not
	je	short mmap_skip_entry

mmap_not_extended:
	mov ecx, [es:di + 8]	; get lower uint32_t of memory region length
	or ecx, [es:di + 12]	; "or" it with upper uint32_t to test for zero
	jz mmap_skip_entry		; if length uint64_t is 0, skip entry

	add	di, 24		; if entry is valid, incread di to next entry position
	inc	bp		; increase counter

mmap_skip_entry:
	and	ebx, ebx	; if ebx = 0, list is complete
	jnz	mmap_loop

mmap_finished:
	mov	[es:memory_map_location], bp

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

mmap_Error:
	mov	si, mmap_Error_text
	mov	ah, 0x0e	; displlay character

mmap_Error_loop:
	mov	al, [si]
	inc	si
	int	0x10
	cmp	al, 0
	jne	mmap_Error_loop

	jmp	$

	; data for 16 bit part of the bootloader
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

mmap_Error_text:
	db	'Error detecting memory', 0

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
