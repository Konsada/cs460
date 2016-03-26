        .globl _main,_syscall,_exit,_getcs
        .globl _getc, _putc, _color

	
        call _main
	
! if main() ever return, exit(0)
	push  #0
        call  _exit

_syscall:
        int    80
        ret
_getcs:
        mov   ax, cs
        ret
