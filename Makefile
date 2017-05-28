#
# Makefile
# Szymon Urbaś, 06 Jan 2017 22:19:39 +0100 (CET) 22:19
#

arch ?= i686
target = $(arch)-unknown-leoman
build_type ?= debug
topdir = $(shell readlink -f .)

mkisofs = mkisofs

ifeq ($(arch), i686)
qemu = qemu-system-i386
else
qemu = qemu-system-$(arch)
endif

.PHONY: all
all: leoman.iso

.PHONY: kernel
kernel:
	@echo Building the kernel...
	@(export RUST_TARGET_PATH="$(topdir)/targets" && cd kernel && xargo build --target=$(target) --quiet)

.PHONY: boot
boot:
	@echo Building the bootloader...
	@(cd boot && make)

iso_root/boot/loader/isoboot.bin: boot
	@mkdir -p $(shell dirname $@)
	@cp boot/isoboot.bin $@
iso_root/boot/kernel/kernel.bin: kernel
	@mkdir -p $(shell dirname $@)
	@cp target/$(target)/$(build_type)/kernel $@
.PHONY: iso_root/boot/kernel/initrd.bin
iso_root/boot/kernel/initrd.bin:
	@mkdir -p $(shell dirname $@)
	@echo "angle" > $@

.PHONY: iso_root
iso_root: \
	iso_root/boot/loader/isoboot.bin \
	iso_root/boot/kernel/initrd.bin \
	iso_root/boot/kernel/kernel.bin

.PHONY: leoman.iso
leoman.iso: iso_root
	@echo Generating leoman.iso...
	@$(mkisofs) -quiet -R -J -l -c boot/boot.cat \
		-b boot/loader/isoboot.bin -no-emul-boot -boot-load-size 4 \
		-o $@ iso_root

.PHONY: run
run: leoman.iso
	$(qemu) -cdrom leoman.iso -monitor stdio

.PHONY: clean
clean:
	@rm -rf target
	@rm -rf *.iso
	@rm -rf iso_root

# vim:ft=make
#
