OUTPUT_FORMAT("elf32-i386")
ENTRY(main)

SECTIONS
{
  . = 0x100000;

  .text :
  {
    *(.text)
    *(.rodata*)
    . = ALIGN(4K);
  }

  .data :
  {
    *(.data)
    . = ALIGN(4K);
  }

  .bss :
  {
    *(COMMON)
    *(.bss)
    *(.stack)
    . = ALIGN(4K);
  }
}

