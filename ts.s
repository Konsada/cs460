        OSSEG  = 0x1000
	
       .globl _main,_running,_scheduler,_proc,_procSize  ! IMPORT
       .globl _tswitch,_inces,_diskr,_setes,_printf, _int80h, _goUmode, _kcinth
	
        jmpi   start,OSSEG

start:	mov  ax,cs
	mov  ds,ax
	mov  ss,ax

        mov  sp,#_proc
	add  sp,_procSize

	call _main
idle:	 jmp  idle
	
_tswitch:
SAVE:	
	push ax
	push bx
	push cx
	push dx
	push bp
	push si
	push di
	pushf
	mov  bx,_running
	mov  2[bx],sp

FIND:	call _scheduler

RESUME:
	mov  bx,_running
	mov  sp,2[bx]
	popf
	pop  di
	pop  si
	pop  bp
	pop  dx
	pop  cx
	pop  bx
	pop  ax

	ret

_diskr:
	push bp
	mov bp,sp

	movb dl, #0x00		! drive 0=fd0 in DL
	movb dh, 6[bp]		! head	      in DH
	movb cl, 8[bp]		! sector      in CL
	incb cl			! inc sector by 1 to suit BIOS
	movb ch, 4[bp]		! cyl         in CH
	mov  ax, #0x0202	! (AH)=0x02=READ, (AL)=02 sectors
	mov  bx, 10[bp]		! put buf value in BX ==> addr = [ES,BX]

!---------- call BIOS INT 0x13 ------------------------------
	int  0x13		! call BIOS to read the block
!------------------------------------------------------------
	jb  error		! to error if CarryBit is on [read failed]

	mov  sp,bp
	pop  bp
	ret

_setes:	push bp			! setes(segment): set ES to a segment
	mov  bp,sp

	mov  ax,4[bp]
	mov  es,ax

	mov  sp,bp
	pop  bp
	ret

_inces:				! inces() inc ES by 0x40, or 1K
	mov  ax,es
	add  ax,#0x40
	mov  es,ax
	ret

error:
	push #msg
	call _printf
	int  0x19		! reboot
msg:	.asciz "Loading Error!"

_int80h:
	push ax			! save CPU registers in ustack
	push bx
	push cx
	push dx
	push bp
	push si
	push di
	push es
	push ds
	push cs
	pop  ds

	mov  si, _running	! switch CPU from Umode to Kmode
	mov  4[si], ss
	mov  6[si], sp

	mov  di, ds
	mov  es, di
	mov  ss, di

	mov  sp, _running
	add  sp, _procSize

	call _kcinth		! call handler function 
	jmp  _goUmode

_goUmode:
	cli
	mov  bx, _running	! restore Umode uSS and uSP from running PROC
	mov  cx, 4[bx]
	mov  ss, cx
	mov  sp, 6[bx]

	pop  ds			! restore saved CPU registers from ustack
	pop  es
	pop  di
	pop  si
	pop  bp
	pop  dx
	pop  cx
	pop  bx
	pop  ax

	iret			! iret back to Umode
