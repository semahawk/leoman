.PHONY: all isoboot

all: isoboot

isoboot: isoboot.bin
isoboot.bin: isoboot.asm
	nasm -dDEBUG -f bin $< -o $@

clean:
	rm -f *.bin

distclean: clean
	rm -f isoboot.bin

