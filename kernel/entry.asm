BITS 32

global kernel_stack_top

extern kmain
global _start
global _higherhalf

extern kernel_phys
extern kernel_start
extern kernel_end
extern kernel_size

; the stack is 16KiB
section .stack
align 4
kernel_stack_bottom:
  ; that's 16KiB
  times 2048 dq 0
kernel_stack_top:

section .preamble
; the page directory is actually computable at compile time
page_directory:
  ; bitwise "or" doesn't work on labels :c
  dd page_table_0 + 3
  ; fill the void between PDE #0 and PDE #896
  times (896 - 1) dd 0
  dd page_table_896 + 3
  dd page_table_897 + 3
  ; fill the remainder of PDEs
  times (1024 - 896 - 3) dd 0
  ; map the page directory to itself as the last page in virtual memory
  dd page_directory + 3

; the first page table is also computable at compile time
; it will identity-map the first 4MiB
; (BIOS stuff plus the .preamble section)
page_table_0:
%assign addr 0x0
%rep 1024
  ; attributes: supervisor level, read + write, present
  dd addr | 3
  %assign addr addr + 4096
%endrep
  ; fill the remainder of the PTEs
  ;times (1024 - 262) dd 0

; this, sadly, can't be computed at compile time (or can it?)
page_table_896:
  ; that's 4KiB
  times 512 dq 0

page_table_897:
  ; that's also 4KiB
  times 512 dq 0

_start:
  ; set up the #896 page table
  mov eax, 0x0 ; counter
  mov ebx, 0x0 ; address

  ; map 4MiB from the physical location to 0xe0000000
  .fill_table_896:
    mov ecx, ebx
    or  ecx, 3
    mov [page_table_896 + eax * 4], ecx
    add ebx, 0x1000
    inc eax
    cmp eax, 1024
    je .end_896
    jmp .fill_table_896
  .end_896:

  ; reset the counter
  mov eax, 0x0

  ; map next 4MiB from the physical location to 0xe0400000
  .fill_table_897:
    mov ecx, ebx
    or  ecx, 3
    mov [page_table_897 + eax * 4], ecx
    add ebx, 0x1000
    inc eax
    cmp eax, 1024
    je .end_897
    jmp .fill_table_897
  .end_897:

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

