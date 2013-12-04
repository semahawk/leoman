.PHONY: all run iso clean

all: nihilum

nihilum: nihilum.bin

nihilum.bin: nihilum.asm
	nasm -f bin -o nihilum.bin nihilum.asm
	dd conv=notrunc if=nihilum.bin of=cdiso/nihilum.flp

run: nihilum
	qemu -fda cdiso/nihilum.flp

iso: nihilum.bin
	mkisofs -no-emul-boot -boot-load-size 4 -quiet -V 'Nihilum' -input-charset iso8859-1 -o cdiso/nihilum.iso -b nihilum.flp cdiso/

clean:
	rm -f cdiso/*.iso
	rm -f *.bin

