; stage two of the bootloader

jmp near boot1

%include "utils.asm"

boot1:
  ; update the segment register
  mov ax, 0x0050
  mov ds, ax
  mov es, ax

  ; save the device's number from which we've booted
  mov [bootdrv], dl
  ; save the # of system partitions
  mov [syscount], cl

keypress:
  ; wait for a key to be pressed
  mov ah, 0
  int 16h
  ; switch (al):
  ; case  'a':
  cmp al, 'a'
  je boot_from_hd
  ; case  'm':
  cmp al, 'm'
  je reboot
  ; al = al - '0'
  sub al, 48
  ; case  1 <= al < [syscount]:
  cmp al, 1
  jge boot_preamble
  ; default:
  jmp keypress

reboot:
  jmp 0xFFFF:0x0000

boot_preamble:
  ; al should be less than '# of systems'
  cmp al, [syscount]
  jl  boot
  jmp keypress

boot:
  print booting_msg
  jmp keypress

boot_from_hd:
  .reset:
    ; reset the hard drive
    mov ah, 00h
    mov dl, 81h
    int 13h

    jc .reset          ; error -> try again

  .read:
    ; set up the registers
    mov ax, 0x0000
    mov es, ax
    mov bx, 0x7C00    ; es:bx = 0000h:7C00h (= 0x7C00)

    ; load the MBR from the hard drive
    mov ah, 0x02      ; the instruction
    mov al, 1         ; load one sector
    mov ch, 0         ; cylinder no. 0
    mov cl, 1         ; sector no. 1
    mov dh, 0         ; head no. 0
    mov dl, 0x81      ; the hard drive
    int 13h           ; read!

    jc .read          ; error -> try again

    mov ax, 0x0000
    mov es, ax
    mov si, 0x7DBE    ; 0x7C00 + 0x01BE (MBR offset)
    mov dl, [bootdrv]
    ; execute the MBR
    jmp 0x0000:0x7C00

halt:
  ; stop right there!
  jmp $

booting_msg db 'booting...', 0xD, 0xA, 0
; number of the drive we have booted from
bootdrv db 0
; number of system partitions
syscount db 0

; pad the remaining of the sector with zeros
times 512-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

