echo -------------------- mk --------------------

VFD=mtximage

#rm ../temp/mtximage
#rm mtximage*
#wget http://eecs.wsu.edu/~cs460/samples/LAB5/mtximage

as86 -o ts.o ts.s
bcc  -c -ansi t.c
ld86  -d -o mtx ts.o t.o mtxlib /usr/lib/bcc/libc.a

#rm $VFD
#cp ../temp/$VFD $VFD

mount -o loop $VFD /mnt
cp mtx /mnt/boot
umount /mnt
rm *.o mtx

(cd USER; ./mku)

qemu-system-i386 -fda $VFD -no-fd-bootchk

echo done
