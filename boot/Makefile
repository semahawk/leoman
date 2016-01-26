.PHONY: all isoboot

all: main isoboot

main: main.bin
main.bin: main.asm isofs.asm
	nasm -dDEBUG -f bin $< -o $@

isoboot: isoboot.bin
isoboot.bin: isoboot.asm isofs.asm
	nasm -dDEBUG -f bin $< -o $@

clean:
	rm -f *.bin

distclean: clean
	rm -f isoboot.bin

