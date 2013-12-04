BITS 16

start:
  mov ax, 07C0h              ; set up 4k stack space after this bootloader
  add ax, 288                ; (4096 + 512) / 16 bytes per paragraph
  mov ss, ax
  mov sp, 4096

  mov ax, 07C0h              ; set data segment to where we're loaded
  mov ds, ax

  mov cx, 10                 ; set the loop's counter to 10
loop_string:
  mov si, text_string        ; put string position into SI
  call print_string          ; call our string-printing routine
  loop loop_string           ; jump to loop_string if cx != 0

  jmp $                      ; infinity!

  text_string db 'Nihilum says Salve ;)', 0dh, 0ah, 0

print_string:
  mov ah, 0Eh                ; routine: output string in SI to screen

.repeat:
  lodsb                      ; get character from string
  cmp al, 0
  je .done                   ; if char is zero, end of string
  int 10h                    ; otherwise print it
  jmp .repeat

.done:
  ret

  times 510-($-$$) db 0      ; pad remainder of boot sector with 0s
  dw 0xAA55                  ; the standard PC boot signature

; vi: ft=nasm:ts=2:sw=2 expandtab

