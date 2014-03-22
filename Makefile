.PHONY: all img run floppy_disk iso clean
.SUFFIXES: .asm .bin

OBJS = boot0.bin \
			 boot1.bin

all: nihilum

nihilum: $(OBJS)
	cat boot0.bin boot1.bin > nihilum

.asm.bin: print.asm
	nasm -f bin $< -o $@

img: nihilum.img
nihilum.img: nihilum
	dd conv=notrunc if=nihilum of=nihilum.img

run: floppy_disk
	qemu -hda disk -monitor stdio

floppy_disk: nihilum disk
	dd conv=notrunc if=nihilum of=disk bs=512 count=128

disk: disk.backup
	cp disk.backup disk

iso: nihilum.img
	mkisofs -no-emul-boot -boot-load-size 4 -quiet -V 'Nihilum' -input-charset iso8859-1 -o nihilum.iso -b nihilum.img .

clean:
	rm -f *.bin
	rm -f *.img
	rm -f *.iso

distclean: clean
	rm -f nihilum

