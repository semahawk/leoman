global get_eip
get_eip:
  pop eax
  jmp eax

; void tss_flush(uint8_t selector);
global tss_flush
tss_flush:
  ; load the index of our TSS structure - passed as an argument
  mov ax, word [esp + 4]
  ltr ax
  ret

; vi: ft=nasm:ts=2:sw=2 expandtab

