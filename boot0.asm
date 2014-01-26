BITS 16
ORG 0

jmp 0x07C0:boot0

%include "utils.asm"

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

welcome:
  ; print the welcomming message (d'oh!)
  print welcome_msg

;
; MBR load (into 0090h:0000h)
;
  ; reset the drive from which we've booted from
  mov dl, [bootdrv]
  call reset_drive

read_mbr:
  ; set up the registers
  mov ax, 0x0090
  mov es, ax
  xor bx, bx        ; es:bx = 0090h:0000h (= 0x0900)

  ; load the MBR from the drive from which we've booted from
  mov ah, 0x02      ; the instruction
  mov al, 1         ; load one sector
  xor ch, ch        ; cylinder no. 0
  mov cl, 1         ; sector no. 1
  xor dh, dh        ; head no. 0
  mov dl, [bootdrv] ; the boot device
  int 13h           ; read!
  ; error -> try again
  jc read_mbr

;
; Stage Two load (into 0050h:0000h)
;
  ; reset the disk we've booted from
  mov dl, [bootdrv]
  call reset_drive

read_boot1:
  ; read the stage two of the bootloader
  mov ax, 0x0050
  mov es, ax
  xor bx, bx        ; es:bx = 0050h:0000h (= 0x0500)

  ; load the second sector from the floppy
  mov ah, 0x02      ; the instruction
  mov al, 1         ; load one sector
  xor ch, dh        ; cylinder no. 0
  mov cl, 2         ; sector no. 2
  xor dh, dh        ; head no. 0
  mov dl, [bootdrv] ; the drive from which we've booted
  int 13h           ; read!

  jc read_boot1     ; error -> try again

  ; save the system partition count (so boot1 can use it)
  mov cl, [syscount]
  ; save the boot drive (so boot1 can use it)
  mov dl, [bootdrv]
  ; jump to stage two
  jmp 0050h:0000h

; the messages
welcome_msg db 'Quidquid Latine dictum, sit altum videtur.', 0xD, 0xA, 0xD, 0xA, 0
; number of the drive we have booted from
bootdrv db 0
; number of system partitions
syscount db 0

; pad the remainder of the boot sector with zeros
times 446-($-$$) db 0

; partition entry #1 {{{
; status (bit 7 set: active / bootable; old MBRs only accept 80h)
;
; 0x00: inactive
; 0x01-0x7F: invalid
db 0x80
; CHS address of FIRST absolute sector in the partition
db 00000000b   ; head (bits 0-7)
db 00000000b   ; sector (bits 0-5); cylinder (bits 6-7 are bits 8-9 of
               ; the cylinder)
db 00000000b   ; cylinder (bits 0-7)
; partition type
db 0xA5        ; (FreeBSD's id)
; CHS address of LAST absolute sector in the partition
db 00000000b   ; head (bits 0-7)
db 00000000b   ; sector (bits 0-5); cylinder (bits 6-7 are bits 8-9 of
               ; the cylinder)
db 00000000b   ; cylinder (bits 0-7)
; LBA of first absolute sector in the partition
dd 0x0
; # of sectors in the partition
dd 0x0
; }}}
; partition entry #2 {{{
; status (bit 7 set: active / bootable; old MBRs only accept 80h)
;
; 0x00: inactive
; 0x01-0x7F: invalid
db 0x00
; CHS address of FIRST absolute sector in the partition
db 00000000b   ; head (bits 0-7)
db 00000000b   ; sector (bits 0-5); cylinder (bits 6-7 are bits 8-9 of
               ; the cylinder)
db 00000000b   ; cylinder (bits 0-7)
; partition type
db 0x39        ; (Plan 9's id)
; CHS address of LAST absolute sector in the partition
db 00000000b   ; head (bits 0-7)
db 00000000b   ; sector (bits 0-5); cylinder (bits 6-7 are bits 8-9 of
               ; the cylinder)
db 00000000b   ; cylinder (bits 0-7)
; LBA of first absolute sector in the partition
dd 0x0
; # of sectors in the partition
dd 0x0
; }}}
; partition entry #4 {{{
dq 0, 0
; }}}
; partition entry #3 {{{
; status (bit 7 set: active / bootable; old MBRs only accept 80h)
;
; 0x00: inactive
; 0x01-0x7F: invalid
db 0x00
; CHS address of FIRST absolute sector in the partition
db 00000000b   ; head (bits 0-7)
db 00000000b   ; sector (bits 0-5); cylinder (bits 6-7 are bits 8-9 of
               ; the cylinder)
db 00000000b   ; cylinder (bits 0-7)
; partition type
db 0x07        ; (OpenBSD's id)
; CHS address of LAST absolute sector in the partition
db 00000000b   ; head (bits 0-7)
db 00000000b   ; sector (bits 0-5); cylinder (bits 6-7 are bits 8-9 of
               ; the cylinder)
db 00000000b   ; cylinder (bits 0-7)
; LBA of first absolute sector in the partition
dd 0x0
; # of sectors in the partition
dd 0x0
; }}}

; the standard PC boot signature
dw 0xAA55

; vi: ft=nasm:ts=2:sw=2 expandtab

