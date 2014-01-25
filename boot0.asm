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
reset_mbr:
  ; reset the drive from which we've booted from
  mov ah, 00h
  mov dl, [bootdrv]
  int 13h
  ; error -> try again
  jc reset_mbr

read_mbr:
  ; set up the registers
  mov ax, 0x0090
  mov es, ax
  mov bx, 0x0000    ; es:bx = 0090h:0000h (= 0x0900)

  ; load the MBR from the drive from which we've booted from
  mov ah, 0x02      ; the instruction
  mov al, 1         ; load one sector
  mov ch, 0         ; cylinder no. 0
  mov cl, 1         ; sector no. 1
  mov dh, 0         ; head no. 0
  mov dl, [bootdrv] ; the boot device
  int 13h           ; read!
  ; error -> try again
  jc read_mbr

  ; print the partition's (types) located in the MBR
  mov cx, 5       ; starting from 5 because of the 'dec cx'
  mov ax, 0x0090
  mov es, ax
  mov si, 0x1C2   ; 1BEh + 4h
  mov dl, 1       ; the partition's menu id

  print_partitions:
  dec cx
  ; print the system's identification string
  .print_sys_id:
    cmp byte [es:si], 0x00    ; None
    je .print_sys_id_after_nolf
    ; print '['
    mov ah, 0xE
    mov al, '['
    int 10h
    ; print the partition's menu id
    print_digit dl
    ; print ']'
    mov ah, 0xE
    mov al, ']'
    int 10h
    ; print ' '
    mov ah, 0xE
    mov al, ' '
    int 10h

    cmp byte [es:si], 0xA5    ; FreeBSD
    je .print_sys_id_bsd
    cmp byte [es:si], 0xA6    ; OpenBSD
    je .print_sys_id_bsd
    cmp byte [es:si], 0x39    ; Plan 9
    je .print_sys_id_plan9
    cmp byte [es:si], 0x83    ; Linux (any)
    je .print_sys_id_linux
    cmp byte [es:si], 0x07    ; Windows
    je .print_sys_id_windoze
    ; none of the above matches the system's id
    print os_unknown
    jmp .print_sys_id_after

  .print_sys_id_bsd:
    print os_bsd
    ; increment the partition's menu id
    inc dl
    jmp .print_sys_id_after
  .print_sys_id_plan9:
    print os_plan9
    ; increment the partition's menu id
    inc dl
    jmp .print_sys_id_after
  .print_sys_id_linux:
    print os_linux
    ; increment the partition's menu id
    inc dl
    jmp .print_sys_id_after
  .print_sys_id_windoze:
    print os_windoze
    ; increment the partition's menu id
    inc dl
    jmp .print_sys_id_after

  .print_sys_id_after:
  call utils_print_newline
  .print_sys_id_after_nolf:
  ; 'go' to the next partition entry
  add si, 16
  ; it's kind of strange, but doing 'loop print_partitions' here gives me 'short
  ; jump out of range' error
  cmp cx, 0
  jg print_partitions
  ; print the additional menu items
  print boot_from_hd_msg

  ; save the number of systems
  mov [syscount], dl

;
; Stage Two load (into 0050h:0000h)
;
reset_boot1:
  ; reset the floppy drive
  mov ax, 0
  mov dl, [bootdrv]
  int 13h

  jc reset_boot1    ; error -> try again

read_boot1:
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

  jc read_boot1     ; error -> try again

  ; save the system partition count (so boot1 can use it)
  mov cl, [syscount]
  ; save the boot drive (so boot1 can use it)
  mov dl, [bootdrv]
  ; jump to stage two
  jmp 0050h:0000h

; the messages
welcome_msg db 'Quidquid Latine dictum, sit altum videtur.', 0xD, 0xA, 0xD, 0xA, 0
boot_from_hd_msg db '[a] boot from HD', 0xD, 0xA, 0xD, 0xA, 0
; the OS IDs
os_bsd db 'BSD', 0
os_plan9 db 'Plan 9', 0
os_linux db 'GNU/Linux', 0
os_windoze db 'Windows', 0
os_unknown db 'unknown', 0
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

