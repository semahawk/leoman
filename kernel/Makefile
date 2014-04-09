AS = nasm
CC = clang
CFLAGS := -save-temps -target i386-pc-none -ffreestanding \
	-fno-builtin -nostdlib

OBJS = entry.o kernel.o

all: kernel

entry.o: entry.asm
	$(AS) -felf32 entry.asm -o entry.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -c -o kernel.o

kernel: $(OBJS) linker.ld
	$(CC) $(CFLAGS) -T linker.ld -o $@ $(OBJS)

clean:
	rm -f *.o

distclean: clean
	rm -f kernel

