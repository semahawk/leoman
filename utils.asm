; a few handy macros

%macro print 0
  mov ah, 0xE
  mov al, 0xD
  int 10h
  mov al, 0xA
  int 10h
%endmacro

%macro print 1
  mov si, word %1
  mov ah, 0xE

  %%print_char:
    lodsb
    cmp al, 0
    je %%done
    int 10h
    jmp %%print_char

  %%done:
%endmacro

; vi: ft=nasm:ts=2:sw=2 expandtab

