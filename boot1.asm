; stage two of the bootloader

jmp near boot1

%include "utils.asm"

boot1:
  ; update the segment register
  mov ax, 0x0050
  mov ds, ax
  mov es, ax

  ; enable the A20 line
enable_a20:
  ; try using BIOS first
  mov ax, 0x2401
  int 0x15
  jnc load_gdt  ; jump straight away to load the GDT
  ; BIOS doesn't support INT 15, 2401
  ; try using FAST A20
  in al, 0x92
  test al, 2
  jnz load_gdt  ; jump straight... yes
  or al, 2
  and al, 0xFE
  out 0x92, al
  ; if this doesn't work we're gonna have a problem

  ;
  ; GDT
  ;
gdt_data:
  ; the null descriptor
  dq 0          ; nothing!
  ; the code descriptor (rw, ring 0)
  dw 0xFFFF     ; limit low
  dw 0x0        ; base low
  db 0x0        ; base middle
  db 10011010b  ; access
  db 11001111b  ; granularity
  db 0x0        ; base high
  ; the data descriptor
  dw 0xFFFF     ; limit low
  dw 0x0        ; base low
  db 0x0        ; base middle
  db 10010010b  ; access
  db 11001111b  ; granularity
  db 0x0        ; base high
  ; other descriptors? (if so, here's the place)
gdt_end:
gdt_ptr:
  dw gdt_end - gdt_data - 1 ; limit (sizeof GDT)
  dd gdt_data               ; base of GDT

; enable_a20 jumps here
load_gdt:
  cli
  lgdt [gdt_ptr]
  sti

reset:
  ; reset the first hard drive
  mov ah, 00h
  mov dl, 80h
  int 13h

  jc reset          ; error -> try again

read:
  ; set up the registers
  mov ax, 0x0090
  mov es, ax
  mov bx, 0x0000    ; es:bx = 0090h:0000h (= 0x0900)

  ; load the MBR from the first hard drive
  mov ah, 0x02      ; the instruction
  mov al, 1         ; load one sector
  mov ch, 0         ; cylinder no. 0
  mov cl, 1         ; sector no. 1
  mov dh, 0         ; head no. 0
  mov dl, 0x80      ; the first hard drive
  int 13h           ; read!

  jc read           ; error -> try again

welcome:
  ; print the welcomming message (d'oh!)
  print welcome_msg

halt:
  ; stop right there!
  jmp $

welcome_msg db 'Quidquid Latine dictum, sit altum videtur.', 0xD, 0xA, 0xD, 0xA, 0

; pad the remaining of the two sectors with 0s
times 1024-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

