	bits 32
	global	_start

	section .text
_start:
	mov	dword [0x2000], 0xb8280
	extern main
	call main
	jmp $
