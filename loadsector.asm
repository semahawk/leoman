; stage two of the bootloader

jmp loadsector
; it's actually two sectors wide :)
loadsector:
  ; update the segment register
  mov ax, 0x0500
  mov ds, ax

  mov ah, 0xE
  mov al, 'N'
  int 10h
  mov al, '!'
  int 10h

  jmp $  ; infinity!

db 'LDSEC_END'
; pad the remaining of the two sectors with 0s
times 1024-($-($$+512)) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

