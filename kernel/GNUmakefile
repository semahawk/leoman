.PHONY: all run clean distclean

AS = nasm
ASFLAGS = -felf32
CC = clang
CFLAGS := -target i386-pc-none -std=c99 -ffreestanding -fno-builtin -nostdlib

C_SRC =     \
  elf.c     \
  gdt.c     \
  idt.c     \
  kbd.c     \
  kernel.c  \
  pm.c      \
  proc.c    \
  sar.c     \
  syscall.c \
  timer.c   \
  tss.c     \
  vga.c     \
  vm.c      \

S_SRC =     \
  entry.asm \
  gdt.asm   \
  idt.asm   \
  proc.asm  \
  x86.asm   \

OBJS  = ${S_SRC:.asm=.asm.o} ${C_SRC:.c=.o}

INITRD_SRCS = \
	initrd.c \
	idle.c

INITRD_OBJS = ${INITRD_SRCS:.c=.initrd}

all: kernel initrd

%.asm.o: %.asm
	$(CC) -Wno-invalid-pp-token -I. -D__ASSEMBLY__ -E - < $< > $<.cpped
	$(AS) $(ASFLAGS) $<.cpped -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

kernel: $(OBJS) linker.ld
	$(CC) -D__ASSEMBLY__ -I. -E - < linker.ld > linker.ld.cpped
	@# dear kittens... get rid of the preprocessor stuff
	sed -i 's/^#.*$$//' linker.ld.cpped
	$(CC) $(CFLAGS) -T linker.ld.cpped -o $@ $(OBJS)

# file.c -> file.o
# initrd stuff
%.initrd: %.c initrd.ld
	$(CC) $(CFLAGS) -T initrd.ld -o $@ $<

initrd: tools $(INITRD_OBJS) initrd.ld
	../tools/sar c $@ $(INITRD_OBJS)

run:
	cd ..; $(MAKE) run

tools:
	cd ..; $(MAKE) tools

clean:
	rm -f ${INITRD_OBJS}
	rm -f *.o
	rm -f *.i
	rm -f *.s
	rm -f *.initrd
	rm -f *.cpped

distclean: clean
	rm -f initrd
	rm -f kernel

