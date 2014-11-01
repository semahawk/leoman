OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)
phys = 0x100000;

SECTIONS
{
  .text phys : AT(phys)
  {
    text = .;
    *(.multiboot)
    *(.text)
    *(.rodata*)
    . = ALIGN(4K);
  }

  .data : AT(phys + (data - text))
  {
    data = .;
    *(.data)
    . = ALIGN(4K);
  }

  .bss : AT(phys + (bss - text))
  {
    bss = .;
    *(COMMON)
    *(.bss)
    *(.stack)
    . = ALIGN(4K);
  }
}

