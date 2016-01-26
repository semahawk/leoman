.PHONY: all bootloader kernel tools run disk_image clean distclean

DISK_IMAGE = leoman.iso

SUBDIRS = boot kernel tools

all: bootloader $(DISK_IMAGE)

bootloader:
	cd boot; $(MAKE)

kernel:
	cd kernel; $(MAKE)

run: $(DISK_IMAGE)
	qemu-system-i386 -cdrom $(DISK_IMAGE) -monitor stdio

tools:
	cd tools; $(MAKE)

$(DISK_IMAGE): bootloader kernel
	mkdir -p iso_root/boot/{kernel,loader}
# install the files into the image
	cp kernel/kernel iso_root/boot/kernel
	cp boot/isoboot.bin iso_root/boot/loader
	cp boot/main.bin iso_root/boot/loader
# create the ISO
	mkisofs -R -J -c boot/boot.cat -b boot/loader/isoboot.bin -no-emul-boot -boot-load-size 4 -o $(DISK_IMAGE) iso_root

clean:
	rm -f *.iso
	rm -rf iso_root
	
	for DIR in $(SUBDIRS); do \
		$(MAKE) -C $$DIR clean; \
	done

distclean: clean
	for DIR in $(SUBDIRS); do \
		$(MAKE) -C $$DIR distclean; \
	done

