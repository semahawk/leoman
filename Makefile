.PHONY: all run disk_image clean
.SUFFIXES: .asm .bin

DISK_IMAGE = nihilum.fs
DISK_IMAGE_SIZE = 64m

BOOT_OBJS = boot0.bin boot1.bin

all: boot $(DISK_IMAGE)

boot: $(BOOT_OBJS)
	cat $(BOOT_OBJS) > $@

.asm.bin: print.asm utils.asm
	nasm -dDEBUG -f bin $< -o $@

run: $(DISK_IMAGE)
	qemu -hda $(DISK_IMAGE) -monitor stdio

$(DISK_IMAGE): boot
	# the original makefs has a little bug, which creates the bootblock of
	# size 8KiB, instead of a 64KiB
	#
	# I should probably upload a patch..
	/usr/src/usr.sbin/makefs/makefs -tffs -oversion=2 -M$(DISK_IMAGE_SIZE) $(DISK_IMAGE) image/
	dd conv=notrunc if=boot of=$(DISK_IMAGE) bs=512 count=128

clean:
	rm -f *.bin
	rm -f *.img
	rm -f *.fs

distclean: clean
	rm -f boot
	rm -f $(DISK_IMAGE)

