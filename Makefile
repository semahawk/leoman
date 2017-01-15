#
# Makefile
# Szymon Urbaś, 06 Jan 2017 22:19:39 +0100 (CET) 22:19
#

arch ?= x86_64
target = $(arch)-unknown-leoman
build_type ?= debug

cargo:
	@xargo build --target=$(target)

# vim:ft=make
#
