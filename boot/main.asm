org 0x8000
bits 16

jmp 0x0000:boot

boot:
  ; update the segment register
  xor ax, ax
  mov ds, ax

  cli
  mov ss, ax
  ; put the stack just below the bootsector
  mov sp, 0xffff
  sti

hello:
  mov ax, 0x0e0e
  int 10h

hang:
  jmp $

; vi: ft=nasm:ts=2:sw=2 expandtab

