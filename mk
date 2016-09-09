clear
clear

sudo cp ../disk/FDimage ./FDimage

VFD=./FDimage

as86 -o bs.o bs.s
bcc  -c -ansi bc.c util.h
ld86 -d -o keonbooter bs.o bc.o /usr/lib/bcc/libc.a

dd if=keonbooter of=$VFD bs=1024 count=1 conv=notrunc

qemu-system-i386 -fda $VFD -no-fd-bootchk
