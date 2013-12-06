.PHONY: all nihilum run iso clean

all: nihilum

nihilum: nihilum.bin

nihilum.bin: nihilum.asm
	nasm -f bin -o nihilum.bin nihilum.asm

nihilum.flp: nihilum.bin
	dd conv=notrunc if=nihilum.bin of=nihilum.flp

run: nihilum.flp
	qemu -fda nihilum.flp

iso: nihilum.flp
	mkisofs -no-emul-boot -boot-load-size 4 -quiet -V 'Nihilum' -input-charset iso8859-1 -o nihilum.iso -b nihilum.flp .

clean:
	rm -f *.flp
	rm -f *.iso
	rm -f *.bin

