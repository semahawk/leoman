#
# Makefile
# Szymon Urbaś, 06 Jan 2017 22:19:39 +0100 (CET) 22:19
#

arch ?= i686
target = $(arch)-unknown-leoman
build_type ?= debug
topdir = $(shell readlink -f .)

mkisofs = mkisofs

.PHONY: all
all: leoman.iso

.PHONY: kernel
kernel:
	@echo Building the kernel...
	@(export RUST_TARGET_PATH="$(topdir)/targets" && cd kernel && xargo build --target=$(target) --quiet)

.PHONY: boot
boot:
	@(cd boot && make)

iso_root/boot/loader/isoboot.bin: boot
	@mkdir -p $(shell dirname $@)
	@cp boot/isoboot.bin $@
iso_root/boot/kernel/kernel.bin: kernel
	@mkdir -p $(shell dirname $@)
	@cp target/$(target)/$(build_type)/kernel $@

.PHONY: iso_root
iso_root: \
	iso_root/boot/loader/isoboot.bin \
	iso_root/boot/kernel/kernel.bin

.PHONY: leoman.iso
leoman.iso: iso_root
	@echo Generating leoman.iso...
	@$(mkisofs) -quiet -R -J -l -c boot/boot.cat \
		-b boot/loader/isoboot.bin -no-emul-boot -boot-load-size 4 \
		-o $@ iso_root

.PHONY: clean
clean:
	@rm -rf target
	@rm -rf *.iso
	@rm -rf iso_root

# vim:ft=make
#
