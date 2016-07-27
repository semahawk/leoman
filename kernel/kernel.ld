OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

/* find a way to use the preprocessor to not hardcode those values
 * in our new build system (CMake) */
kernel_phys = 0x00100000;
kernel_off  = 0xe0000000;

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

