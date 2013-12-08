; stage two of the bootloader

jmp near loadsector

%include "utils.asm"

; it's actually two sectors wide :)
loadsector:
  ; update the segment register
  mov ax, 0x0500
  mov ds, ax
  mov es, ax

  ; finish the ok! thing
  print ok
  print

  ; print the menu
  print menu_boot
  print menu_reboot
  print

keypress:
  ; wait for a pressed key
  mov ah, 0
  int 16h
  ; user pressed 'b'
  cmp al, 98
  je boot
  ; user pressed Enter (carriage return)
  cmp al, 0dh
  je boot
  ; user pressed 'r'
  cmp al, 114
  je reboot
  jmp keypress

boot:
  print not_working
  jmp keypress

reboot:
  jmp 0xFFFF:0x0000

ok db ' ok!', 0dh, 0ah, 0
menu_boot   db '  [b] Boot [Enter]', 0dh, 0ah, 0
menu_reboot db '  [r] Reboot', 0dh, 0ah, 0
not_working db "Meh, booting's not working yet, come on! :P", 0dh, 0

; pad the remaining of the two sectors with 0s
times 1024-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

