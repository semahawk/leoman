#include <kernel/smp.h>

org KERNEL_TRAMPOLINE_LOAD_ADDR
bits 16

start:
    jmp 0x0:trampoline

trampoline:
    ; update the segment register
    xor ax, ax
    mov ds, ax

.halt:
    cli
    hlt

    ; just for good measure
    jmp .halt

; vi: ft=nasm:ts=2:sw=2:expandtab
