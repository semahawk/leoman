.PHONY: all bootloader kernel run disk_image clean

DISK_IMAGE = nihilum.fs
DISK_IMAGE_SIZE = 64m

SUBDIRS = boot kernel

all: bootloader $(DISK_IMAGE)

bootloader:
	cd boot; $(MAKE)

kernel:
	cd kernel; $(MAKE)

run: $(DISK_IMAGE)
	qemu -hda $(DISK_IMAGE) -monitor stdio

$(DISK_IMAGE): bootloader kernel
	mkdir -p image/boot
	cp kernel/kernel image/boot
	@# the original makefs has a little bug, which creates the bootblock of
	@# size 8KiB, instead of 64KiB
	@#
	@# I should probably upload a patch..
	/usr/src/usr.sbin/makefs/makefs -tffs -oversion=2 -M$(DISK_IMAGE_SIZE) $(DISK_IMAGE) image/
	dd conv=notrunc if=boot/boot.bin of=$(DISK_IMAGE) bs=512 count=128

clean:
.for dir in $(SUBDIRS)
	$(MAKE) -C ${dir} clean
.endfor

distclean: clean
.for dir in $(SUBDIRS)
	$(MAKE) -C ${dir} distclean
.endfor

