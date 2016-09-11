clear
clear

VFD=vdisk

as86 -o ts.o ts.s
bcc  -c -ansi t.c util.h
ld86 -d -o mtx ts.o t.o /usr/lib/bcc/libc.a

sudo mount -o loop $VFD /mnt

qemu-system-i386 -fda $VFD -no-fd-bootchk

