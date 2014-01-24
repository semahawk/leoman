; a phony MBR sector; used for tests running under QEMU

;
; Phony Bootstrap code area
;

  ; hello, infinity
  jmp $

; fill out the empty space with zeros
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

