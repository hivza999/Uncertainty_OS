	bits 16

	; UncertaintyOS bootstrap
	%define	Identifier	'UncertFS'
	
	%define	partition_ID	0x0000abcd
	
	; must be 11 char (pad with spaces)
	%define partition_name	'Uncertainty'

	%define	nb_reserved_sec	0x0020

; BIOS parameter block

	jmp short $
	nop

	db	Identifier
	dw	0x0200		; number of bytes per sector
	db	0x08		; number of sector per cluster
	dw	nb_reserved_sec	; number of reserved sectors
	db	2		; number of file allocation table
	dw	0		; number of root entries
	dw	0		; sector count (0 > more tham 65535 sectors)
	db	0xf8		; media descriptor type
	dw	0x0000		; unused for FAT32
	dw	0x003e		; number of sector per track
	dw	0x00f7		; number of head
	dd	0x00000800	; number of hidden sectors
	dd	0x001ffffe	; large sector count

	; Extended boot record
	dd	2048		; Size of the FAT (In sector)
	dw	0x0000		; flags
	dw	0x0000		; version of FAT
	dd	2		; Cluster number of root dir
	dw	0x0001		; Sector number of FSinfo
	dw	0x0006		; Sector of backup boot sector
	times 12 db 0		; reserved
	db	0x80		; drive number
	db	0		; reserved
	db	0x29		; signature
	dd	partition_ID	; Volume ID
	db	partition_name	; partition name
	db	'FAT32   '	; System identifer

	times 510-($-$$) db 0
	dw	0xaa55		; make it 0xaa55 for bootable partition 

; FS info
	dd	0x41615252	; signature
	times 480 db 0		; reserved
	dd	0x61417272	; signature again
	dd	0xffffffff	; last free cluster (0xffffffff = unknowed)
	dd	0xffffffff	; where to start looking for free clusters (0xffffffff = unknowed)
	times 12 db 0		; reserved
	dd	0xaa550000	; trail signature


	times 0xc00-($-$$) db 0
; recovery

	; BIOS parameter block

	jmp short $
	nop

	db	 Identifier
	dw	0x0200		; number of bytes per sector
	db	0x08		; number of sector per cluster
	dw	nb_reserved_sec	; number of reserved sectors
	db	2		; number of file allocation table
	dw	0		; number of root entries
	dw	0		; sector count (0 > more tham 65535 sectors)
	db	0xf8		; media descriptor type
	dw	0x0000		; unused for FAT32
	dw	0x003e		; number of sector per track
	dw	0x00f7		; number of head
	dd	0x00000800	; number of hidden sectors
	dd	0x001ffffe	; large sector count

	; Extended boot record
	dd	2048		; Size of the FAT (In sector)
	dw	0x0000		; flags
	dw	0x0000		; version of FAT
	dd	2		; Cluster number of root dir
	dw	0x0001		; Sector number of FSinfo
	dw	0x0006		; Sector of backup boot sector
	times 12 db 0		; reserved
	db	0x80		; drive number
	db	0		; reserved
	db	0x29		; signature
	dd	partition_ID	; Volume ID
	db	partition_name	; partition name
	db	'FAT32   '	; System identifer

	times 0xdfe-($-$$) db 0
	dw	0xaa55		; make it 0xaa55 for bootable partition 

; FS info
	dd	0x41615252	; signature
	times 480 db 0		; reserved
	dd	0x61417272	; signature again
	dd	0xffffffff	; last free cluster (0xffffffff = unknowed)
	dd	0xffffffff	; where to start looking for free clusters (0xffffffff = unknowed)
	times 12 db 0		; reserved
	dd	0xaa550000	; trail signature

	times (nb_reserved_sec*512)-($-$$) db 0
