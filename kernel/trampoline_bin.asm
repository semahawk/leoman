#include <kernel/smp.h>

org KERNEL_TRAMPOLINE_LOAD_ADDR
bits 16

start:
    jmp 0x0:trampoline

align KERNEL_TRAMPOLINE_VARS_OFFSET
magic: dd 0x0
stack_id: dd 0x0

trampoline:
    ; update the segment register
    xor ax, ax
    mov ds, ax

    mov bx, word [magic]
    mov cx, word [magic + 2]

.halt:
    cli
    hlt

    ; just for good measure
    jmp .halt

; vi: ft=nasm:ts=2:sw=2:expandtab
