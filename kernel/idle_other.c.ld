OUTPUT_FORMAT("elf32-i386")
ENTRY(main)

SECTIONS
{
  . = 0x30000000;

  .text : AT(ADDR(.text))
  {
    text = .;
    *(.text)
    *(.rodata*)
    . = ALIGN(4K);
  }

  .data : AT(ADDR(.data))
  {
    data = .;
    *(.data)
    . = ALIGN(4K);
  }

  .bss : AT(ADDR(.bss))
  {
    bss = .;
    *(COMMON)
    *(.bss)
    *(.stack)
    . = ALIGN(4K);
  }
}

