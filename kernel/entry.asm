bits 32

global kernel_stack_top

extern kmain
global _start
global _higherhalf

extern kernel_phys
extern kernel_start
extern kernel_end
extern kernel_size

%define KERNEL_VIRT_OFFSET 0xe0000000

; the stack is 16KiB
section .stack
align 4
kernel_stack_bottom:
  ; that's 16KiB
  times 2048 dq 0
kernel_stack_top:

section .preamble
; the page directory is mostly computable at compile time
page_directory:
  ; bitwise "or" doesn't work on labels :c
  dd page_table_0 + 3
  ; fill the void between PDE #0 and PDE #1023
  ; the kernel's page table will be plugged somewhere here
  times (1024 - 2) dd 0
  ; map the page directory to itself as the last page in virtual memory
  dd page_directory + 3

; the first page table is completely computable at compile time
; it will identity-map the first 4MiB
page_table_0:
  %assign addr 0x0
  %rep 1024
    ; attributes: supervisor level, read + write, present
    dd addr | 3
  %assign addr addr + 4096
  %endrep

; this also can be computed at compile time :)
; it will map 4MiB for the kernel
kernel_page_table:
  %assign addr 0x0
  %rep 1024
    ; attributes: supervisor level, read + write, present
    dd addr | 3
  %assign addr addr + 4096
  %endrep

_start:
  ; insert kernel's page table into the page directory
  ; eax will hold the PDE's index
  mov eax, KERNEL_VIRT_OFFSET
  shr eax, 22
  ; ebx will hold the page table's address
  mov ebx, kernel_page_table
  or  ebx, 3

  ; insert the kernel page table's PDE into the page directory
  mov [page_directory + eax * 4], ebx

  ; enable paging
  mov eax, page_directory
  mov cr3, eax

  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax

  jmp _higherhalf

section .text
_higherhalf:
  ; boot1 creates the `bootinfo' structure, populates it, and pushes a pointer
  ; to it to the stack (the pointer that's to be passed to `kmain')
  ; we have to remember the pointer's value because the kernel sets up it's
  ; own stack
  pop eax

  ; yup, right here
  mov esp, kernel_stack_top

  ; we are now ready to actually execute C code
  ; calling kmain(eax)
  push eax
  call kmain

  ; in case the function returns
  cli
.halt:
  hlt
  jmp .halt

; vi: ft=nasm:ts=2:sw=2 expandtab

