; stage two of the bootloader

jmp near loadsector
; it's actually two sectors wide :)
loadsector:
  ; update the segment register
  mov ax, 0x0500
  mov ds, ax
  mov es, ax

  call print_m

  jmp $  ; infinity!

print_m:
  mov ah, 0xE
  mov al, 'm'
  int 10h
  ret

menu_boot   db '  [b] Boot [Enter]', 0dh, 0ah, 0
menu_reboot db '  [r] Reboot', 0dh, 0ah, 0

; pad the remaining of the two sectors with 0s
times 1024-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

