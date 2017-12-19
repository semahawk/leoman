global gdt_flush

gdt_flush:
  ; SEG_KCODE
  jmp 0x08:.flush
.flush:
  ; SEG_KDATA
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  ret

; vi: ft=nasm:ts=2:sw=2 expandtab

