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
  - Booting off of an ISO disk
  - Basic keyboard input
  - Kernel running in the higher half
  - Basic setup for system calls
  - Simplest paging
  - Basic ring 3 processes support with preemptive, round-robin multitasking
  - Processes run in their own address spaces
  - Basic support for ELF executables

#### End goals

  - Simple, stable and reliable microkernel
  - Little to no POSIX compliance
  - Keep away from anything GNU
  - At least support for the ARM architecture

Dependencies
============

#### Build time

  - clang
  - nasm
  - tup
  - mkisofs
  - make (optional)

#### Run time

  - qemu-system-i386

Building
========

Leoman uses Tup as the build system. Please refer to http://gittup.org/tup/
for instructions on how to build the build system.

After making sure you have it in your $PATH, perform the following:

    tup init
    tup

Alternatively, a small Makefile is provided which does that for you. The make
target for building the image is `build` (which is also the default):

    make [build]

At this point you should have the `leoman.iso` file which contains the
bootloadable ISO filesystem with the kernel and the initrd.

If you also wanted to try the system out (not that there is much to look at),
there's the `run` target:

    make run

which pretty much just runs qemu with the image.
