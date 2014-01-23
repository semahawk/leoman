; stage two of the bootloader

jmp near boot1

%include "utils.asm"

boot1:
  ; update the segment register
  mov ax, 0x0500
  mov ds, ax
  mov es, ax

  ; enable the A20 line
enable_a20:
  ; try using BIOS first
  mov ax, 0x2401
  int 0x15
  jnc .done
  ; BIOS doesn't support INT 15, 2401
  ; try using FAST A20
  in al, 0x92
  test al, 2
  jnz .done
  or al, 2
  and al, 0xFE
  out 0x92, al
  ; if this doesn't work we're gonna have a problem

.done: ; enable_a20

  cli
  hlt

; pad the remaining of the two sectors with 0s
times 1024-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

