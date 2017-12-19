org 0x8000
bits 16

start:
    jmp 0x0:trampoline

trampoline:
    ; update the segment register
    ; mov ax, 0x8000
    xor ax, ax
    mov ds, ax

.halt:
    cli
    hlt

    ; just for good measure
    jmp .halt

; vi: ft=nasm:ts=2:sw=2:expandtab
