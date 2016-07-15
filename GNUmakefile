.PHONY: all bootloader kernel tools run disk_image clean distclean

DISK_IMAGE = leoman.iso

GDB = cgdb --

SUBDIRS = boot kernel tools

all: bootloader $(DISK_IMAGE)

bootloader:
	cd boot; $(MAKE)

kernel:
	cd kernel; $(MAKE)

run: $(DISK_IMAGE)
	qemu-system-i386 -cdrom $(DISK_IMAGE) -monitor stdio

gdb: .gdbcmds
	$(GDB) -x $<

# dependency on GNUmakefile so that the file gets updated on the offside
# that this target also is changed
.gdbcmds: GNUmakefile $(DISK_IMAGE)
	@echo -n "" > $@
	@# TODO meh those damn hardcodes
	@echo "symbol-file kernel/kernel" >> $@
	@echo "add-symbol-file kernel/idle.initrd 0x30000000" >> $@
	@echo "add-symbol-file kernel/idle_other.initrd 0x40000000" >> $@
	@echo "target remote | qemu-system-i386 -S -gdb stdio -cdrom $(DISK_IMAGE)" >> $@

tools:
	cd tools; $(MAKE)

$(DISK_IMAGE): bootloader kernel
	mkdir -p iso_root/boot/{kernel,loader}
# install the files into the image
	cp kernel/kernel iso_root/boot/kernel/kernel.bin
	cp kernel/initrd iso_root/boot/kernel/initrd.bin
	cp boot/isoboot.bin iso_root/boot/loader
# create the ISO
	mkisofs -quiet -R -J -l -c boot/boot.cat -b boot/loader/isoboot.bin -no-emul-boot -boot-load-size 4 -o $(DISK_IMAGE) iso_root

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

