clear
clear

VFD=FDimage

as86 -o ts.o ts.s
#bcc  -c -ansi t.c util.h
bcc -c -ansi t.c
ld86 -d -o mtx ts.o t.o mtxlib /usr/lib/bcc/libc.a

sudo mount -o loop $VFD /mnt

sudo rm /mnt/boot/*

sudo cp mtx /mnt/boot/
sudo umount /mnt

echo ready?

qemu-system-i386 -fda FDimage -no-fd-bootchk

