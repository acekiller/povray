;   File To Fix DJGCC V1.09 bug that doesn't allow IOCTL read/writes
;
;   Written 2/9/93 by Aaron A. Collins
;   This file is hereby placed in the public domain.
;
;   Version 1.1 - minor entry/exit fixes 6/1/93 - AAC
;
;   Things that need IOCTL read/write operations simply need to be
;   recoded to use 0x440A and 0x440B (which DJGCC's go32 passes).
;   This converts them to the proper 0x4402 or 0x4403 as required,
;   then executes the DOS interrupt.
;		
	.RADIX	16
; 
CSEG	segment para public 'CODE'
	assume cs:CSEG, ds:CSEG, ss:CSEG, es:CSEG
;
main	proc	near
;
	org	100
;
begin:	jmp	start			; entry point according to 'end' statement
;
olddv:	dw	0000
oldds:	dw	0000
;
;
start:	push	es
	push	ds
	push	cs			; litter seg registers with our CS
	pop	ds
	cli
	mov	ah,35			; get vector, save and effect mine.
	mov	al,21			; get vector of DOS fn. interrupt
	int	21
	mov	word ptr [olddv],bx	; store vector offset of old...
	mov	ax,es
	mov	word ptr [oldds],ax	; store vector segment of old...
	lea	dx,dosint		; get effective address of int srvr.
	mov	ah,25			; set new interrupt vector
	mov	al,21			; set vector of DOS fn. interrupt
	int	21
	sti
	pop	ds
	pop	es
	mov	dx,((finish-begin+115) SHR 4) ; add file length to PSP >> para
	mov	ax,3100			; DOS TSR with retcd = 00.
	int	21
	mov	ax,4c01			; DOS Program Terminate, retcd = 01
	int	21			; we should never get here...
	ret
;
main	endp
;
	even
;
;dosint	proc	far			; DOS function int. service routine
;
;	pushf				; simulate building Int stack structure
;	pushf				; save flags we are about to trash
;	cmp	cx,1E			; make sure this is a Targa read/write?
;	jnz	around
;	cmp	ax,440A			; bogus IOCTL read call?
;	jz	yup			; Yes - modify it, else do DOS call
;	cmp	ax,440BH		; bogus IOCTL write call?
;	jnz	around			; No - do DOS call straight away...
;yup:	sub	al,8
;	jmp	around
;wierd:	iret				; this is really strange but it works!
;around: popf				; restore saved flags
;	push	cs			; simulate building Int stack structure
;	call	wierd
;	jmp	dword ptr cs:[olddv] 	; simulated far call to prev. INT 21 vector
;
;dosint	endp
;
dosint	proc	far			; DOS function int. service routine
;
	pushf				; save flags we are about to trash
	cmp	cx,1E			; make sure this is a Targa read/write?
	jnz	around
	cmp	ax,440A			; bogus IOCTL read call?
	jz	yup			; Yes - modify it, else do DOS call
	cmp	ax,440BH		; bogus IOCTL write call?
	jnz	around			; No - do DOS call straight away...
yup:	sub	al,8
around: popf				; restore saved flags
	jmp	dword ptr cs:[olddv] 	; simulated far call to prev. INT 21 vector
;
dosint	endp
;
finish:	db	0
;
CSEG	ends
;
	end	begin
