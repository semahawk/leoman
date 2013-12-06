BITS 16
ORG 0

jmp 07C0h:start

welcome_msg db 'Nihilum bootloader v0.0 awaiting for the command!', 0dh, 0ah, 0dh, 0ah, 0
menu_opt1   db '  [r] Reboot', 0dh, 0ah, 0
menu_opt2   db '  [b] Boot (not working, obviously)', 0dh, 0ah, 0dh, 0ah, 0
boot_not_working db "It's not working, ey!", 0
easter_egg db 'Easter egg!', 0
blank_line db '                                             ', 0
keypressed db 1
cursor_row db 1

; {{{ print a string from SI
print_string:
  mov ah, 0x0E

  .repeat:
    lodsb                      ; get character from string
    cmp al, 0
    je .done                   ; if char is zero, end of string
    int 10h                    ; otherwise print it
    jmp .repeat

  .done:
    ret
; }}}

; {{{ puts macro
%macro puts 1
  push si
  mov si, %1
  call print_string
  pop si
%endmacro
; }}}

; {{{ debug macro
%macro debug 1
  ; set cursor's position
  mov ah, 02h
  mov bh, 0             ; page number = 0
  mov dh, [cursor_row]  ; row = cursor_row
  mov dl, 0             ; column = 0
  int 10h

  puts blank_line

  ; set cursor's position (again)
  mov ah, 02h
  mov bh, 0             ; page number = 0
  mov dh, [cursor_row]  ; row = cursor_row
  mov dl, 0             ; column = 0
  int 10h

  puts %1
%endmacro
; }}}

start:
  ; update the segment registers
  mov ax, cs
  mov ds, ax
  mov es, ax

  puts welcome_msg           ; print the welcoming message
  puts menu_opt1             ;
  puts menu_opt2             ; print the menu

  ; get cursor's position (to print messages at a fixed position)
  ; (i'm not making the row value fixed so that it looks nice in QEMU)
  mov ah, 03h
  mov bh, 0    ; page number = 0
  int 10h
  mov [cursor_row], dh

keypress:
  ; wait for a key to be pressed
  mov ah, 0
  int 16h
  mov [keypressed], al
  ; see which key got pressed
  ; the user pressed 'r', going for reboot
  cmp [keypressed], byte 0x72
  je reboot
  ; he pressed 'b', booting (haha :-)
  cmp [keypressed], byte 0x62
  je boot
  ; he pressed 'e', easter egg! ;)
  cmp [keypressed], byte 0x65
  je easter

  jmp keypress

reboot:
  jmp 0xFFFF:0x0000

boot:
  debug boot_not_working
  jmp keypress

easter:
  debug easter_egg
  jmp keypress

; pad remainder of boot sector with 0s
times 510-($-$$) db 0
; the standard PC boot signature
dw 0xAA55

; vi: ft=nasm:ts=2:sw=2 expandtab

