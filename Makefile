.PHONY: all nihilum img run iso clean

all: nihilum

nihilum: nihilum.bin

nihilum.bin: nihilum.asm bootsector.asm loadsector.asm
	nasm -f bin -o nihilum.bin nihilum.asm

nihilum.img: nihilum.bin
	dd conv=notrunc if=nihilum.bin of=nihilum.img

img: nihilum.img

run: nihilum.img
	qemu -fda nihilum.img

iso: nihilum.img
	mkisofs -no-emul-boot -boot-load-size 4 -quiet -V 'Nihilum' -input-charset iso8859-1 -o nihilum.iso -b nihilum.img .

clean:
	rm -f *.img
	rm -f *.iso
	rm -f *.bin

