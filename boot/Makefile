.PHONY: all isoboot

CC = clang
CFLAGS := -target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib
AS = nasm
ASFLAGS = -dDEBUG -f bin

all: isoboot

isoboot: isoboot.bin
isoboot.bin: isoboot.asm isofs.asm elf.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f *.bin
	rm -f *.temp

distclean: clean

