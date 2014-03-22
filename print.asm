; prints a newline
putnl:
; {{{
  pusha
  mov ah, 0xe
  mov al, 0xd
  int 10h
  mov al, 0xa
  int 10h
  popa
  ret
; }}}

; prints characters from SI until a nul is found
putstr:
; {{{
  pusha
  mov ah, 0xe
  .putchar:
    lodsb
    cmp al, 0
    je .done
    int 10h
    jmp .putchar
  .done:
    popa
    ret
; }}}

; print the value in AL as a digit (0-9 as decimal, 10-15 as
; lowercase hexadecimal)
putdigit:
; {{{
  pusha
  cmp al, 10
  jl .ten_less
  .ten_more:
    add al, 87
    jmp .print
  .ten_less:
    add al, 48
  .print:
    mov ah, 0xE
    int 10h
    popa
    ret
; }}}

; print the value in EDX as a hexadecimal dword
puthex:
; {{{
%define COUNTER ecx
%define MASK ebx
%define VALUE edx

  pusha
  ; some initialization
  mov MASK, 0xf0000000
  mov COUNTER, 8
  ; print the leading '0x'
  mov ah, 0xE
  mov al, '0'
  int 10h
  mov al, 'x'
  int 10h

.nibbliwibbli:
  ; save the counter
  push COUNTER
  ; and the original value
  push VALUE

  mov eax, VALUE ; eax = VALUE
  and eax, MASK  ; eax = eax & MASK
  push eax
  push ebx
  ; calculate the number by which the mask should be shifted
  mov eax, ecx   ; eax = ecx
  mov ebx, 4     ; ebx = 4
  mul ebx        ; eax = ebx * 4
  sub eax, 4     ; eax = eax - 4
  mov ecx, eax   ; ecx = eax
  pop ebx
  pop eax
  shr eax, cl    ; eax = eax >> cl
  call putdigit

  ; shift the nibble four bits right
  shr MASK, 4
  ; restore the value
  pop VALUE
  ; restore the counter
  pop COUNTER
  loop .nibbliwibbli

  ; restore the registers
  popa
  ret

%undef COUNTER
%undef MASK
%undef VALUE
; }}}

; vi: ft=nasm:ts=2:sw=2 expandtab

