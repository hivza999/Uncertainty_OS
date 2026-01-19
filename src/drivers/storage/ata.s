%define primary_ATA_IO_base	0x1F0
%define primary_ATA_Ctl_base	0x376

%define primary_ATA_Data		primary_ATA_IO_base + 0
%define primary_ATA_sector_count	primary_ATA_IO_base + 2
%define primary_ATA_LBA_lo		primary_ATA_IO_base + 3
%define primary_ATA_LBA_mid		primary_ATA_IO_base + 4
%define primary_ATA_LBA_hi		primary_ATA_IO_base + 5
%define primary_ATA_Drive_Select	primary_ATA_IO_base + 6
%define primary_ATA_Command		primary_ATA_IO_base + 7
%define primary_ATA_Status		primary_ATA_IO_base + 7

%define Identify_memory_location	0x90600

	section .text

extern print
extern hexprint8

global ATA_init
ATA_init:
	mov	al, 0xa0
	mov	dx, primary_ATA_Drive_Select
	out	dx, al

	xor	al, al
	mov	dx, primary_ATA_sector_count
	out	dx, al
	mov	dx, primary_ATA_LBA_lo
	out	dx, al
	mov	dx, primary_ATA_LBA_mid
	out	dx, al
	mov	dx, primary_ATA_LBA_hi
	out	dx, al

	mov	al, 0xec
	mov	dx, primary_ATA_Command
	out	dx, al

	; same register for command and status
	in	al, dx

	cmp	al, 0
	je	error

loop0:
	test	al, 0x80
	jnz short loop0

	mov	dx, primary_ATA_LBA_hi
	in	al, dx
	mov	ah, al

	mov	dx, primary_ATA_LBA_mid
	in	al, dx

	or	ax, ax
	jnz	error

loop1:
	mov	dx, primary_ATA_Status
	in	al, dx

	test	al, 0b0000_0001
	jnz	error

	test	al, 0b0000_1000
	jz short loop1

	mov	edi, Identify_memory_location
	mov	bx, 256	; counter

loop2:
	mov	dx, primary_ATA_Data
	in	ax, dx
	mov	[edi], ax
	add	edi, 2

	mov	dx, primary_ATA_Status
loop3:	in	al, dx
	test	al, 0b1000_0000
	jnz short loop3

	dec	bx
	cmp	bx, 0
	jne short loop2

	; exit
	mov	al, 0
	ret

error:	mov	al, 1
	ret