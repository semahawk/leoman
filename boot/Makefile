.PHONY: all
.SUFFIXES: .asm .bin

BOOT_OBJS = boot0.bin boot1.bin

all: boot.bin

boot.bin: $(BOOT_OBJS)
	cat $(BOOT_OBJS) > $@

.asm.bin: print.asm utils.asm
	nasm -dDEBUG -f bin $< -o $@

clean:
	rm -f *.bin

distclean: clean

