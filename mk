VFD=mtximage

as86 -o ts.o ts.s
bcc  -c -ansi t.c
bcc  -c -ansi kernel.c
#bcc  -c -ansi wait.c
#bcc  -c -ansi int.c
#ld86 -d -o mtx ts.o t.o kernel.o wait.o int.o mtxlib /usr/lib/bcc/libc.a
ld86  -d -o mtx ts.o t.o mtxlib /usr/lib/bcc/libc.a

sudo mount -o loop $VFD /mnt
sudo cp mtx /mnt/boot
sudo umount /mnt
sudo rm *.o mtx

(cd USER;echo ./mku u1)
qemu-system-i386 -fda $VFD -no-fd-bootchk
echo done
