%define primary_ATA_IO_base	0x1f0
%define primary_ATA_Ctl_base	0x3f6
%define secondary_ATA_IO_base	0x170
%define secondary_ATA_Ctl_base	0x376

%define primary_ATA_Data		primary_ATA_IO_base + 0
%define primary_ATA_sector_count	primary_ATA_IO_base + 2
%define primary_ATA_LBA_lo		primary_ATA_IO_base + 3
%define primary_ATA_LBA_mid		primary_ATA_IO_base + 4
%define primary_ATA_LBA_hi		primary_ATA_IO_base + 5
%define primary_ATA_Drive_Select	primary_ATA_IO_base + 6
%define primary_ATA_Command		primary_ATA_IO_base + 7
%define primary_ATA_Status		primary_ATA_IO_base + 7

%define secondary_ATA_Data		secondary_ATA_IO_base + 0
%define secondary_ATA_sector_count	secondary_ATA_IO_base + 2
%define secondary_ATA_LBA_lo		secondary_ATA_IO_base + 3
%define secondary_ATA_LBA_mid		secondary_ATA_IO_base + 4
%define secondary_ATA_LBA_hi		secondary_ATA_IO_base + 5
%define secondary_ATA_Drive_Select	secondary_ATA_IO_base + 6
%define secondary_ATA_Command		secondary_ATA_IO_base + 7
%define secondary_ATA_Status		secondary_ATA_IO_base + 7

%define	ATA_Master_drive	0xa0
%define	ATA_Slave_drive	0xb0

%define Identify_memory_location	0x90600

	section .text

extern print
extern hexprint8

global ATA_init
ATA_init:
	sub	esp, 4


	; primary ATA / Master
	; Select drive
	mov	al, ATA_Master_drive
	mov	dx, primary_ATA_Drive_Select
	out	dx, al

	mov	ebx, Identify_memory_location
	call	Identify_primary_disk
	mov	byte [ebp-4], al

	; primary ATA / Slave
	; Select drive
	mov	al, ATA_Slave_drive
	mov	dx, primary_ATA_Drive_Select
	out	dx, al

	mov	ebx, Identify_memory_location + 512
	call	Identify_primary_disk
	shl	al, 1
	or	byte [ebp-4], al


	; secondary ATA / Master
	; Select drive
	mov	al, ATA_Master_drive
	mov	dx, secondary_ATA_Drive_Select
	out	dx, al

	mov	ebx, Identify_memory_location + 1024
	call	Identify_secondary_disk
	shl	al, 2
	or	byte [ebp-4], al

	; secondary ATA / Slave
	; Select drive
	mov	al, ATA_Slave_drive
	mov	dx, secondary_ATA_Drive_Select
	out	dx, al

	mov	ebx, Identify_memory_location + 1536
	call	Identify_secondary_disk
	shl	al, 3
	or	byte [ebp-4], al

	; exit
	mov	al, [ebp-4]
	add	esp, 4
	ret

; primary ATA bus
Identify_primary_disk:
	mov	edi, ebx	; get destionation for the data

	; clear some register
	xor	al, al
	mov	dx, primary_ATA_sector_count
	out	dx, al
	mov	dx, primary_ATA_LBA_lo
	out	dx, al
	mov	dx, primary_ATA_LBA_mid
	out	dx, al
	mov	dx, primary_ATA_LBA_hi
	out	dx, al

	; send the Identify command
	mov	al, 0xec
	mov	dx, primary_ATA_Command
	out	dx, al

	; same register for command and status
	in	al, dx

	cmp	al, 0
	je	Identify_primary_error

Identify_primary_loop0:
	test	al, 0x80
	jnz short Identify_primary_loop0

	mov	dx, primary_ATA_LBA_hi
	in	al, dx
	mov	ah, al

	mov	dx, primary_ATA_LBA_mid
	in	al, dx

	or	ax, ax
	jnz	Identify_primary_error

Identify_primary_loop1:
	mov	dx, primary_ATA_Status
	in	al, dx

	test	al, 0b0000_0001
	jnz	Identify_primary_error

	test	al, 0b0000_1000
	jz short Identify_primary_loop1

	mov	bx, 256	; counter

Identify_primary_loop2:
	mov	dx, primary_ATA_Data
	in	ax, dx
	mov	[edi], ax
	add	edi, 2

	mov	dx, primary_ATA_Status
Identify_primary_loop3:
	in	al, dx
	test	al, 0b1000_0000
	jnz short Identify_primary_loop3

	dec	bx
	cmp	bx, 0
	jne short Identify_primary_loop2

	mov	al, 1	; Disk found
	ret

Identify_primary_error:
mov	al, 0	; 0 Disk not found
	ret

; Secondary ATA bus
Identify_secondary_disk:
	mov	edi, ebx	; get destionation for the data

	; clear some register
	xor	al, al
	mov	dx, secondary_ATA_sector_count
	out	dx, al
	mov	dx, secondary_ATA_LBA_lo
	out	dx, al
	mov	dx, secondary_ATA_LBA_mid
	out	dx, al
	mov	dx, secondary_ATA_LBA_hi
	out	dx, al

	; send the Identify command
	mov	al, 0xec
	mov	dx, secondary_ATA_Command
	out	dx, al

	; same register for command and status
	in	al, dx

	cmp	al, 0
	je	Identify_secondary_error

Identify_secondary_loop0:
	test	al, 0x80
	jnz short Identify_secondary_loop0

	mov	dx, secondary_ATA_LBA_hi
	in	al, dx
	mov	ah, al

	mov	dx, secondary_ATA_LBA_mid
	in	al, dx

	or	ax, ax
	jnz	Identify_secondary_error

Identify_secondary_loop1:
	mov	dx, secondary_ATA_Status
	in	al, dx

	test	al, 0b0000_0001
	jnz	Identify_secondary_error

	test	al, 0b0000_1000
	jz short Identify_secondary_loop1

	mov	bx, 256	; counter

Identify_secondary_loop2:
	mov	dx, secondary_ATA_Data
	in	ax, dx
	mov	[edi], ax
	add	edi, 2

	mov	dx, secondary_ATA_Status
Identify_secondary_loop3:
	in	al, dx
	test	al, 0b1000_0000
	jnz short Identify_secondary_loop3

	dec	bx
	cmp	bx, 0
	jne short Identify_secondary_loop2

	mov	al, 1	; Disk found
	ret

Identify_secondary_error:
mov	al, 0	; 0 Disk not found
	ret
