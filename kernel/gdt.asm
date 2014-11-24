global reload_segments

reload_segments:
  jmp 0x08:.reload
.reload:
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  ret

; vi: ft=nasm:ts=2:sw=2 expandtab

