OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
  . = 0x30000000;

  .text : 
  {
    text = .;
    *(.text)
    *(.rodata*)
    . = ALIGN(4K);
  }

  .data :
  {
    data = .;
    *(.data)
    . = ALIGN(4K);
  }

  .bss :
  {
    bss = .;
    *(COMMON)
    *(.bss)
    *(.stack)
    . = ALIGN(4K);
  }
}

