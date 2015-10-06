; defined in idt.c
extern isr_handler
extern irq_handler

isr_common_stub:
  pusha ; pushes e[ds]i, e[bs]p, e[abcd]x
  push ds
  push es
  push fs
  push gs

  mov ax, 0x10 ; load the kernel data segment descriptor
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov eax, esp
  push eax
  mov eax, isr_handler
  call eax
  pop eax

  pop gs
  pop fs
  pop es
  pop ds
  popa ; pops e[ds]i, e[bs]p, e[abcd]x

  add esp, 8 ; cleans up the pushed error code and pushed ISR number
  iret ; pops cs, epi, eflags, ss and esp

irq_common_stub:
  pusha
  push ds
  push es
  push fs
  push gs

  mov ax, 0x10 ; load the kernel data segment
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  mov ecx, esp
  push ecx
  mov ecx, irq_handler
  call ecx
  pop ecx

  pop gs ; restore the data segments
  pop fs
  pop es
  pop ds
  popa

  add esp, 8
  iret

; DRY
; argument #1: the ISR number
%macro isr_noerr 1
  global isr%1
  isr%1:
    cli
    push byte 0 ; dummy error code
    push byte %1
    jmp isr_common_stub
%endmacro

; argument #1: the ISR number
%macro isr_err 1
  global isr%1
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

; argument #1: the IRQ number
; argument #2: the IDT gate it is mapped to
%macro irq 2
  global irq%1
  irq%1:
    cli
    push byte 0 ; dummy error code
    push byte %2
    jmp irq_common_stub
%endmacro

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err   8
isr_noerr 9
isr_err   10
isr_err   11
isr_err   12
isr_err   13
isr_err   14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31

irq    0, 32
irq    1, 33
irq    2, 34
irq    3, 35
irq    4, 36
irq    5, 37
irq    6, 38
irq    7, 39
irq    8, 40
irq    9, 41
irq   10, 42
irq   11, 43
irq   12, 44
irq   13, 45
irq   14, 46
irq   15, 47

; vi: ft=nasm:ts=2:sw=2 expandtab

