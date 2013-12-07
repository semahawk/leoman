.PHONY: all img run iso clean
.SUFFIXES: .asm .bin

OBJS = bootsector.bin \
			 loadsector.bin

all: nihilum

nihilum: $(OBJS)
	cat $(OBJS) > $@

.asm.bin:
	nasm -f bin $< -o $@

nihilum.img: nihilum
	dd conv=notrunc if=nihilum of=nihilum.img

img: nihilum.img

run: nihilum
	qemu -fda nihilum -monitor stdio

iso: nihilum.img
	mkisofs -no-emul-boot -boot-load-size 4 -quiet -V 'Nihilum' -input-charset iso8859-1 -o nihilum.iso -b nihilum.img .

clean:
	rm -f *.bin
	rm -f *.img
	rm -f *.iso

distclean: clean
	rm nihilum

