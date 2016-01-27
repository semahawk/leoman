.PHONY: all main isoboot

CC = clang
CFLAGS := -target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib
AS = nasm
ASFLAGS = -dDEBUG -f bin

all: main isoboot

main: main.bin
main.bin: main.c
	$(CC) $(CFLAGS) -T main.ld $< -o $@

isoboot: isoboot.bin
isoboot.bin: isoboot.asm isofs.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f *.bin
	rm -f *.temp

distclean: clean

