OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

kernel_phys = 0x100000;
off  = 0xe0000000;
virt = off - kernel_phys;

SECTIONS
{
  . = kernel_phys;

  .preamble : AT(ADDR(.preamble))
  {
    preamble = .;
    *(.preamble)
    . = ALIGN(4K);
  }

  . += virt;

  kernel_start = .;

  .text : AT(ADDR(.text) - virt)
  {
    text = .;
    *(.multiboot)
    *(.text)
    *(.rodata*)
    . = ALIGN(4K);
  }

  .data : AT(ADDR(.data) - virt)
  {
    data = .;
    *(.data)
    . = ALIGN(4K);
  }

  .bss : AT(ADDR(.bss) - virt)
  {
    bss = .;
    *(COMMON)
    *(.bss)
    *(.stack)
    . = ALIGN(4K);
  }

  kernel_end = .;
}

