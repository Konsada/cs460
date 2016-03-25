echo -------------------- mk --------------------

VFD=mtximage

as86 -o ts.o ts.s
bcc  -c -ansi t.c
#bcc  -c -ansi kernel.c
#bcc  -c -ansi wait.c
#bcc  -c -ansi int.c
#bcc  -c -ansi fork_exec.c
#ld86 -d -o mtx ts.o t.o kernel.o wait.o int.o fork_exec.o mtxlib /usr/lib/bcc/libc.a
ld86  -d -o mtx ts.o t.o mtxlib /usr/lib/bcc/libc.a
#echo ld86
cp ../temp/$VFD ./

sudo mount -o loop $VFD /mnt
cp mtx /mnt/boot
umount /mnt
rm *.o mtx

(cd USER; ./mku)

qemu-system-i386 -fda $VFD -no-fd-bootchk

echo done
