!------------------------------- bs.s file ------------------
BOOTSEG = 0x9000
SSP     = 32*1024

!IMPORT
.globl _main, _printf, _color, _dap
!EXPORT
.globl _diskr, _getc, _putc, _setds, _error

! Master Boot Record loaded at segment 0x07C0. Load entire booter to segment 0x9000
mov ax,#BOOTSEG
mov es,ax
!clear BX = 0
xor bx,bx

!---------------------------------------------
! Read entire booter in sector#1-8KB to 0x9000
!---------------------------------------------
! dh=head=0, dL=0x80=HD or USB's MBR
mov dx,#0x0080
xor cx,cx
! cyl 0, sector 1
incb cl
! cyl 0, sector 2
incb cl
! READ 16 sectors (booter<8KB)
mov ax, #0x0220
int 0x13

! CodeSegment=BOOTSEG, InstructionPointer=next
jmpi next,BOOTSEG

next:
! establish segments again
mov ax,cs
! we know ES,CS=0x9000. Let DataSegment=CodeSegment
mov ds,ax
mov ss,ax
mov es,ax
! 32 KB stack size
mov sp,#SSP

! Call BIOS for 640x480 color mode
mov ax,#0x0012
int 0x10

! call main() in C
call _main

! check return value from main()
test ax, ax
! main() return 0 if error
je _error

! FAR jump to (segment, offset)=(0x1000, 0)
jmpi 0, 0x1000

!---------------------------------------------
! char getc() function: return a char from kb
!---------------------------------------------
_getc:
! clear ah
	xorb ah, ah
! call BIOS to get a char in AX
	int 0x16
! 7-bit ascii
	andb al,#0x7F
	ret
!---------------------------------------------
! int putc(char c) function: print a char to screen
!---------------------------------------------
_putc:
	push bp
	mov bp,sp

! get the char into aL
	movb al,4[bp]
! aH = 14
	movb ah,#14
! cyan
	mov bx,_color
! call BIOS to display the char
	int 0x10

	pop bp
	ret

!---------------------------------------------
!setds(segment) : set DS to segment to R/W memory outside BOOTSEG
!---------------------------------------------
_setds:
	push bp
	mov bp,sp
	mov ax,4[bp]
	mov ds,ax
	pop bp
	ret

!---------------------------------------------
! diskr(): read disk sectors specified by _dap in C code
!---------------------------------------------
_diskr:
	mov dx, #0x0080
	mov ax, #0x4200
	mov si, #_dap

! call BIOS to read the block
	int 0x13
! to error() if CarryBit is on [read failed]
	jb _error
	ret

!---------------------------------------------
! error() & reboot
!---------------------------------------------
_error:
	push #msg
	call _printf
! reboot
	int 0x19
msg: .asciz "\n\rError!\n\r"

