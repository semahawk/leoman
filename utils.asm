; a few handy macros

; saves the registers
%macro regsave 0
  push ax
  push bx
  push cx
  push dx
%endmacro
; restores the registers
%macro regrest 0
  pop dx
  pop cx
  pop bx
  pop ax
%endmacro

%macro print 0
  regsave
  mov ah, 0xE
  mov al, 0xD
  int 10h
  mov al, 0xA
  int 10h
  regrest
%endmacro

%macro print 1
  regsave
  ; set the registers
  mov si, word %1
  mov ah, 0xE

  %%print_char:
    lodsb
    cmp al, 0
    je %%done
    int 10h
    jmp %%print_char

  %%done:
    regrest
%endmacro

; vi: ft=nasm:ts=2:sw=2 expandtab

