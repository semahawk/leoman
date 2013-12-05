BITS 16
[ORG 0]

jmp 07C0h:start

welcome_msg db 'Nihilum says Salve ;)', 0
pressany_msg db 'Press any key to reboot..', 0

; {{{ print a newline
printnl:
  mov ah, 0Eh
  mov al, 0Dh
  int 10h       ; print carriage return
  mov al, 0Ah
  int 10h       ; print new line feed
  ret
; }}}

; {{{ print a string from SI with a newline appended
puts:
  mov ah, 0Eh

  .repeat:
    lodsb                      ; get character from string
    cmp al, 0
    je .done                   ; if char is zero, end of string
    int 10h                    ; otherwise print it
    jmp .repeat

  .done:
    call printnl
    ret
; }}}

start:
  ; update the segment registers
  mov ax, cs
  mov ds, ax
  mov es, ax

  mov si, welcome_msg        ;
  call puts                  ; print the welcoming message
  call printnl               ; print a new line

  mov si, pressany_msg       ;
  call puts                  ; print the second message

  ; wait for any key to be pressed
  mov ah, 0
  int 16h

  ; reboot
  jmp 0xFFFF:0x0000

  jmp $                      ; infinity!

  times 510-($-$$) db 0      ; pad remainder of boot sector with 0s
  dw 0xAA55                  ; the standard PC boot signature

; vi: ft=nasm:ts=2:sw=2 expandtab

