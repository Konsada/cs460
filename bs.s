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


