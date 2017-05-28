Leòman
======

My "operating" system.

Why?
====

To have fun, and learn a bunch.

Features
========

#### Current

  - Targets the i386 Intel architecture
  - Complete rewrite in Rust - so most features are gone now

#### End goals

  - Simple, stable and reliable microkernel
  - Little to no POSIX compliance
  - Keep away from anything GNU
  - At least support for the ARM architecture

Dependencies
============

#### Build time

  - cargo
  - rust
  - nasm
  - mkisofs
  - make (mooore or less optional)

#### Run time

  - qemu-system-i386

Building
========

Leoman uses Cargo as the main build system. It's used to build the kernel and
all the other components written in Rust (which hopefully will ultimately mean
'everything'), plus a mixture of plain old makefiles. A general Makefile is
provided which will build everything needed to create the image:

    make

At this point you should have the `leoman.iso` file which contains the
bootloadable ISO filesystem with the kernel and the initrd.

If you also wanted to try the system out (not that there is much to look at),
there's the `run` target:

    make run

which pretty much just runs qemu with the image.
