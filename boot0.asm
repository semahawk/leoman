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

  mov dl, [bootdrv] ; save the boot drive (so boot1 can use it)
  ; jump to stage two
  jmp 0050h:0000h

; number of the drive we have booted from
bootdrv db 0

; pad the remainder of the boot sector with zeros
times 446-($-$$) db 0

; partition entry #1 {{{
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
db 0xA6        ; (OpenBSD's id)
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

; the standard PC boot signature
dw 0xAA55

; vi: ft=nasm:ts=2:sw=2 expandtab

