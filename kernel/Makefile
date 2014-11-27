.PHONY: all run clean distclean

AS = nasm
CC = clang
CFLAGS := -target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib

C_SRC = kernel.c gdt.c paging.c mm.c idt.c kbd.c vga.c tar.c timer.c
S_SRC = entry.asm gdt.asm idt.asm
OBJS  = ${S_SRC:.asm=.asm.o} ${C_SRC:.c=.o}

INITRD_SRCS = initrdtestfile.c
INITRD_OBJS = ${INITRD_SRCS:.c=}

all: kernel initrd

.SUFFIXES: .asm.o .initrd

.asm.asm.o:
	$(AS) -felf32 $< -o $@

.c.o:
	$(CC) -c $(CFLAGS) $<

kernel: $(OBJS) linker.ld
	$(CC) $(CFLAGS) -T linker.ld -o $@ $(OBJS)

.c:
	$(CC) $(CFLAGS) -fPIC $< -o $@

initrd: $(INITRD_OBJS)
	tar -cf $@ $(INITRD_OBJS)

run:
	cd ..; $(MAKE) run

clean:
	rm -f *.o
	rm -f *.i
	rm -f *.s

distclean: clean
	rm -f initrd
	rm -f kernel

