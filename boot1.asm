BITS 16
ORG 0

jmp near start

start:
  ; print '.'
  mov ah, 0xe
  mov al, '.'
  int 10h

halt:
  cli
  hlt

; pad the remainder of the code section with zeros
times 512-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

