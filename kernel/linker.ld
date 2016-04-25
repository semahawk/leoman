OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

#include <vm.h>

kernel_phys = KERN_PHYS;
kernel_off  = KERN_VOFF;

SECTIONS
{
  . = kernel_phys;

  .preamble : AT(ADDR(.preamble))
  {
    preamble = .;
    *(.preamble)
    . = ALIGN(4K);
  }

  . += kernel_off;

  kernel_start = .;

  .text : AT(ADDR(.text) - kernel_off)
  {
    text = .;
    *(.multiboot)
    *(.text)
    *(.rodata*)
    . = ALIGN(4K);
  }

  .data : AT(ADDR(.data) - kernel_off)
  {
    data = .;
    *(.data)
    . = ALIGN(4K);
  }

  .bss : AT(ADDR(.bss) - kernel_off)
  {
    bss = .;
    *(COMMON)
    *(.bss)
    *(.stack)
    . = ALIGN(4K);
  }

  kernel_end = .;
  kernel_size = kernel_end - kernel_start;
}

