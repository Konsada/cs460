        MTXSEG  = 0x1000
	
       .globl _main,_running,_scheduler
       .globl _proc, _procSize, _color
       .globl _tswitch, _getc, _putc,_getbp
	
        jmpi   start,MTXSEG

start:	mov  ax,cs
	mov  ds,ax
	mov  ss,ax
        mov  es,ax
	mov  sp,#_proc
	add  sp,_procSize
		
	call _main

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

	
! added functions for KUMODE
	.globl _int80h,_goUmode,_kcinth
	.globl _diskr,_setes,_inces
	.globl _printf
!These offsets are defined in struct proc
USS =   4
USP =   6

_int80h:
        push ax                 ! save all Umode registers in ustack
        push bx
        push cx
        push dx
        push bp
        push si
        push di
        push es
        push ds

! ustack contains : flag,uCS,uPC, ax,bx,cx,dx,bp,si,di,ues,uds
        push cs
        pop  ds                 ! KDS now

	mov si,_running  	! ready to access proc
        mov USS[si],ss          ! save uSS  in proc.USS
        mov USP[si],sp          ! save uSP  in proc.USP

! Change ES,SS to kernel segment
        mov  di,ds              ! stupid !!        
        mov  es,di              ! CS=DS=SS=ES in Kmode
        mov  ss,di

! set sp to HI end of running's kstack[]
	mov  sp,_running        ! proc's kstack [2 KB]
        add  sp,_procSize       ! HI end of PROC

        call  _kcinth
        jmp   _goUmode
  
_goUmode:
	cli
	mov bx,_running 	! bx -> proc
        mov cx,USS[bx]
        mov ss,cx               ! restore uSS
        mov sp,USP[bx]          ! restore uSP
  
	pop ds
	pop es
	pop di
        pop si
        pop bp
        pop dx
        pop cx
        pop bx
        pop ax                  ! NOTE: contains return value to Umode     
	
        iret
	
        .globl _diskr,_setes,_inces,_printf

                               !        4    6      8     10
_diskr:                        ! diskr(cyl, head, sector, buf)                 
        push  bp
	mov   bp,sp
	
        movb  dl, #0x00        ! drive 0=fd0 in DL
        movb  dh, 6[bp]        ! head        in DH
        movb  cl, 8[bp]        ! sector      in CL
        incb  cl               ! inc sector by 1 to suit BIOS
        movb  ch, 4[bp]        ! cyl         in CH
        mov   ax, #0x0202      ! (AH)=0x02=READ, (AL)=02 sectors
        mov   bx, 10[bp]       ! put buf value in BX ==> addr = [ES,BX]

!---------- call BIOS INT 0x13 ------------------------------------
        int  0x13              ! call BIOS to read the block 
!-----------------------------------------------------------------
        jb   error             ! to error if CarryBit is on [read failed]

	mov   sp,bp
	pop   bp
	ret

_setes:  push  bp              ! setes(segment): set ES to a segment        
	 mov   bp,sp
	
         mov   ax,4[bp]        
         mov   es,ax

	 mov   sp,bp
	 pop   bp
	 ret

_inces:                        ! inces() inc ES by 0x40, or 1K
         mov   ax,es
         add   ax,#0x40
         mov   es,ax
         ret

error:
        push #msg
        call _printf
        int  0x19                       ! reboot
msg:    .asciz  "Loading Error!"

_getbp:
	mov  ax, bp
	ret
	
_getc:
	xorb ah,ah
	int  0x16
	ret

_putc:
	push bp
	mov  bp,sp
	movb al,4[bp]
	movb ah,#14
	mov  bx,_color
	int  0x10
	pop  bp
	ret
