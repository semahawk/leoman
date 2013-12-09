; stage two of the bootloader

jmp near loadsector

%include "utils.asm"

; it's actually two sectors wide :)
loadsector:
  ; update the segment register
  mov ax, 0x0500
  mov ds, ax
  mov es, ax

  ; detect the 'low' and 'upper' memory
  print low_memory
  xor ax, ax
  int 12h
  ; if en error ocurred - print unknown
  jc .low_memory_unknown
  ; if it returned zero - print unknown
  test ax, ax
  jz .low_memory_unknown
  ; it went all ok, returned valid value etc.
  ; ax now holds the amount of continuous memory in kB starting from 0
  mov dx, ax
  print_dec ax
  mov ah, 0xE
  mov al, 'k'
  int 10h
  mov al, 'B'
  int 10h

  mov ah, 0xE
  mov al, ' '
  int 10h

  jmp print_menu

.low_memory_unknown:
  print unknown

print_menu:
  ; print the menu
  print
  print
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
  print boot_msg
  jmp keypress

reboot:
  jmp 0xFFFF:0x0000

low_memory  db 'Low memory:    ', 0
up_memory   db 'Upper memory:  ', 0
unknown     db 'unknown :c', 0
menu_boot   db '  [b] Boot [Enter]', 0dh, 0ah, 0
menu_reboot db '  [r] Reboot', 0dh, 0ah, 0
boot_msg    db 'Nope.', 0dh, 0

; pad the remaining of the two sectors with 0s
times 1024-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

