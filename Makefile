.PHONY: all img run iso clean
.SUFFIXES: .asm .bin

OBJS = boot0.bin \
			 boot1.bin \
			 phony-mbr.bin

all: nihilum

nihilum: $(OBJS)
	cat $(OBJS) > $@

.asm.bin: utils.asm
	nasm -f bin $< -o $@

nihilum.img: nihilum
	dd conv=notrunc if=nihilum of=nihilum.img

img: nihilum.img

run: nihilum phony-mbr.bin
	qemu -boot order=a -hda phony-mbr.bin -fda nihilum -monitor stdio

iso: nihilum.img
	mkisofs -no-emul-boot -boot-load-size 4 -quiet -V 'Nihilum' -input-charset iso8859-1 -o nihilum.iso -b nihilum.img .

clean:
	rm -f *.bin
	rm -f *.img
	rm -f *.iso

distclean: clean
	rm nihilum

