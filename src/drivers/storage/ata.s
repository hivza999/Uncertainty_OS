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
%define primary_ATA_alternate_status	primary_ATA_Ctl_base + 0

%define secondary_ATA_Data		secondary_ATA_IO_base + 0
%define secondary_ATA_sector_count	secondary_ATA_IO_base + 2
%define secondary_ATA_LBA_lo		secondary_ATA_IO_base + 3
%define secondary_ATA_LBA_mid		secondary_ATA_IO_base + 4
%define secondary_ATA_LBA_hi		secondary_ATA_IO_base + 5
%define secondary_ATA_Drive_Select	secondary_ATA_IO_base + 6
%define secondary_ATA_Command		secondary_ATA_IO_base + 7
%define secondary_ATA_Status		secondary_ATA_IO_base + 7

%define	ATA_Master_drive	0xa0
%define	ATA_Slave_drive		0xb0

%define ATA_IDENTIFY		0xec
%define	ATA_PIO_READ		0x20

%define	Device_ATA	1
%define	Device_PATAPI	2
%define	Device_SATAPI	3
%define	Device_PATA	4
%define	Device_SATA	5
%define Device_Unknowed	255

%define Identify_memory_location	0x90600

	section .text

extern print
extern hexprint8

global ATA_init
ATA_init:
	sub	esp, 4
	mov	dword [ebp-4], 0


	; primary ATA / Master
	; Select drive
	mov	al, ATA_Master_drive
	mov	dx, primary_ATA_Drive_Select
	out	dx, al
	call	wait_40ns

	mov	ebx, Identify_memory_location
	call	Identify_primary_disk
	or	dword [ebp-4], eax

	; primary ATA / Slave
	; Select drive
	mov	al, ATA_Slave_drive
	mov	dx, primary_ATA_Drive_Select
	out	dx, al
	call	wait_40ns

	mov	ebx, Identify_memory_location + 512
	call	Identify_primary_disk
	shl	eax, 8
	or	dword [ebp-4], eax


	; secondary ATA / Master
	; Select drive
	mov	al, ATA_Master_drive
	mov	dx, secondary_ATA_Drive_Select
	out	dx, al
	call	wait_40ns

	mov	ebx, Identify_memory_location + 1024
	call	Identify_secondary_disk
	shl	eax, 16
	or	dword [ebp-4], eax

	; secondary ATA / Slave
	; Select drive
	mov	al, ATA_Slave_drive
	mov	dx, secondary_ATA_Drive_Select
	out	dx, al
	call	wait_40ns

	mov	ebx, Identify_memory_location + 1536
	call	Identify_secondary_disk
	shl	eax, 24
	or	dword [ebp-4], eax

	; exit
	mov	eax, [ebp-4]
	add	esp, 4
	ret


wait_40ns:
	mov	dx, primary_ATA_Status
	mov	ah, 15

wait_40ns_loop:
	in	al, dx
	dec	ah
	jnz	wait_40ns_loop

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
	mov	al, ATA_IDENTIFY
	mov	dx, primary_ATA_Command
	out	dx, al


Identify_primary_loop4:
	; same register for command and status
	in	al, dx

	or	al, al
	jz	Identify_primary_error

	test	al, 1	; test error bit
	jnz	Identify_primary_aborted

	test	al, 8	; test DRQ bit
	jz	Identify_primary_loop4

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

	mov	eax, Device_ATA	; Disk found
	ret

Identify_primary_aborted:
	; get the drive signature
	mov	dx, primary_ATA_IO_base + 5
	in	al, dx
	mov	ah, al

	mov	dx, primary_ATA_IO_base + 4
	in	al, dx


	mov	bl, Device_Unknowed

	cmp	ax, 0xeb14
	jne	Identify_primary_skip0
	mov	ebx, Device_PATAPI
Identify_primary_skip0:

	cmp	ax, 0x9669
	jne	Identify_primary_skip1
	mov	ebx, Device_SATAPI
Identify_primary_skip1:

	cmp	ax, 0x0000
	jne	Identify_primary_skip2
	mov	ebx, Device_PATA
Identify_primary_skip2:

	cmp	ax, 0xc33c
	jne	Identify_primary_skip3
	mov	ebx, Device_SATA
Identify_primary_skip3:

	mov	eax, ebx
	ret	


Identify_primary_error:
mov	eax, 0	; 0 Disk not found
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
	mov	al, ATA_IDENTIFY
	mov	dx, secondary_ATA_Command
	out	dx, al

Identify_secondary_loop4:
	; same register for command and status
	in	al, dx

	or	al, al
	jz	Identify_primary_error

	test	al, 1	; test error bit
	jnz	Identify_primary_aborted

	test	al, 8	; test DRQ bit
	jz	Identify_secondary_loop4

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


	mov	eax, Device_ATA	; Disk found
	ret

Identify_secondary_aborted:
	; get the drive signature
	mov	dx, secondary_ATA_IO_base + 5
	in	al, dx
	mov	ah, al

	mov	dx, secondary_ATA_IO_base + 4
	in	al, dx

	mov	bl, Device_Unknowed

	cmp	ax, 0xeb14
	jne	Identify_secondary_skip0
	mov	ebx, Device_PATAPI
Identify_secondary_skip0:

	cmp	ax, 0x9669
	jne	Identify_secondary_skip1
	mov	ebx, Device_SATAPI
Identify_secondary_skip1:

	cmp	ax, 0x0000
	jne	Identify_secondary_skip2
	mov	ebx, Device_PATA
Identify_secondary_skip2:

	cmp	ax, 0xc33c
	jne	Identify_secondary_skip3
	mov	ebx, Device_SATA
Identify_secondary_skip3:

	mov	eax, ebx
	ret	

Identify_secondary_error:
	mov	eax, 0	; 0 Disk not found
	ret

global ATA_PIO_read
ATA_PIO_read:
	mov	esi, eax

	; select drive (low 4 bits are high bits of the LBA)
	mov	dx, primary_ATA_Drive_Select
	mov	al, [esi+3]
	out	dx, al

	call	wait_40ns	; wait after a drive select

	; bits 0~23 of the LBA
	mov	dx, primary_ATA_LBA_lo
	mov	al, [esi]
	out	dx, al

	mov	dx, primary_ATA_LBA_mid
	mov	al, [esi+1]
	out	dx, al

	mov	dx, primary_ATA_LBA_hi
	mov	al, [esi+2]
	out	dx, al

	;3	sector count
	mov	dx, primary_ATA_sector_count
	mov	al, [esi+8]
	out	dx, al

	;7	read command
	mov	dx, primary_ATA_Command
	mov	al, ATA_PIO_READ
	out	dx, al


	mov	bx, 256		; counter
	mov	edi, [esi+4]	; destination

ATA_PIO_read_loop:
	mov	dx, primary_ATA_Status
	in	al, dx

	; test for error
	mov	ah, al
	test	ah, 0x01
	jnz	Error

	; wait until drive is ready
	mov	ah, al
	and	al, 0x80
	and	ah, 0x08
	xor	ah, 0x08 ; wait until it set

	or	al, ah
	jnz	ATA_PIO_read_loop

	mov	dx, primary_ATA_Data
	in	ax, dx
	mov	[edi], ax

	; increment to next loop
	add	edi, 2
	dec	bx
	jnz	ATA_PIO_read_loop

	mov	eax, 0
	ret

Error:	; default error handler
	mov	eax, 1
	ret