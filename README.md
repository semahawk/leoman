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
  - cmake
  - make
  - mkisofs

#### Run time

  - qemu-system-i386

Building
========

Thanks to CMake building is pretty easy. After you've cloned the repository:

    CC=clang cmake .
    make

(Building with GCC is not supported yet.)

This will create the `leoman.iso` file which contains the bootloadable ISO filesystem with the kernel and the initrd.

If you also wanted to try the system out (not that there is much to look at), there's the `run` target:

    make run

which pretty much just runs qemu with the image.
