.PHONY: all run clean distclean

AS = nasm
CC = clang
CFLAGS := -target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib

C_SRC = kernel.c mm.c idt.c kbd.c vga.c timer.c
S_SRC = entry.asm isr.asm
OBJS  = ${S_SRC:.asm=.o} ${C_SRC:.c=.o}

all: kernel

.asm.o:
	$(AS) -felf32 $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $<

kernel: $(OBJS) linker.ld
	$(CC) $(CFLAGS) -T linker.ld -o $@ $(OBJS)

run:
	cd ..; $(MAKE) run

clean:
	rm -f *.o
	rm -f *.i
	rm -f *.s

distclean: clean
	rm -f kernel

