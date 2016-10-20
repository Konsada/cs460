VFD=mtximage

as86 -o ts.o ts.s
bcc  -c -ansi t.c
ld86 -d -o mtx ts.o t.o OBJ/*.o mtxlib /usr/lib/bcc/libc.a

mkdir ./mnt
sudo mount -o loop $VFD ./mnt
ls
cp mtx ./mnt/boot
umount ./mnt


(cd USER; ./mkallu)

echo done
