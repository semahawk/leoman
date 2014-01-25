BITS 16
ORG 0

jmp 0x07C0:boot0

boot0:
  ; update the segment register
  mov ax, 0x07C0
  mov ds, ax

  ; set up the stack
  cli
  mov ax, 0x9000
  mov ss, ax
  mov sp, 0xFFFF
  sti

  ; save the device's number from which we've booted
  mov [bootdrv], dl

reset:
  ; reset the floppy drive
  mov ax, 0
  mov dl, [bootdrv]
  int 13h

  jc reset          ; error -> try again

read:
  ; read the stage two of the bootloader
  mov ax, 0x0050
  mov es, ax
  mov bx, 0x0000    ; es:bx = 0050h:0000h (= 0x0500)

  ; load the second sector from the floppy
  mov ah, 0x02      ; the instruction
  mov al, 1         ; load one sector
  mov ch, 0         ; cylinder no. 0
  mov cl, 2         ; sector no. 2
  mov dh, 0         ; head no. 0
  mov dl, [bootdrv] ; the drive from which we've booted
  int 13h           ; read!

  jc read           ; error -> try again

  ; jump to stage two
  jmp 0050h:0000h

; number of the drive we have booted from
bootdrv db 0

; pad the remainder of the boot sector with n's
times 510-($-$$) db 0
; the standard PC boot signature
dw 0xAA55

; vi: ft=nasm:ts=2:sw=2 expandtab

