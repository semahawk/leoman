#
# Makefile
# Szymon Urbaś, 17 Oct 2016 23:06:22 +0200 (CEST) 23:06
#

.PHONY: build run

build:
	@if [ ! -d .tup ]; then tup init; fi
	@tup

run: build
	qemu-system-i386 -smp 16 -cdrom leoman.iso -monitor stdio -serial file:serial.log

# vim:ft=make
#
