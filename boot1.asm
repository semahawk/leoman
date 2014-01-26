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

  ; print the partition's (types) located in the MBR
  mov cx, 5       ; starting from 5 because of the 'dec cx'
  mov ax, 0x0090
  mov es, ax
  mov si, 0x1BE
  mov dl, 1       ; the partition's menu id

  print_partitions:
  dec cx
  ; print the system's identification string
  .print_sys_id:
    cmp byte [es:si], 0x80    ; see if the partition is bootable
    jne .print_sys_id_after_nolf
    ; print ' '
    mov al, ' '
    call print_char
    ; print '['
    mov al, '['
    call print_char
    ; print the partition's menu id
    print_digit dl
    ; print ']'
    mov al, ']'
    call print_char
    ; print ' '
    mov al, ' '
    call print_char
    ; set si to point to the partition's type
    add si, 0x4
    ;  see what the partition type is
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
    jmp .print_sys_id_after
  .print_sys_id_plan9:
    print os_plan9
    ; increment the partition's menu id
    jmp .print_sys_id_after
  .print_sys_id_linux:
    print os_linux
    ; increment the partition's menu id
    jmp .print_sys_id_after
  .print_sys_id_windoze:
    print os_windoze
    ; increment the partition's menu id
    jmp .print_sys_id_after

  .print_sys_id_after:
  inc dl
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
  print reboot_msg

  ; save the number of systems
  mov [syscount], dl

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

; the messages
booting_msg db 'booting...', 0xD, 0xA, 0
boot_from_hd_msg db ' [a] boot from HD', 0xD, 0xA, 0
reboot_msg db ' [m] reboot', 0xD, 0xA, 0xD, 0xA, 0
; the OS IDs
os_bsd db 'BSD', 0
os_plan9 db 'Plan 9', 0
os_linux db 'Linux', 0
os_windoze db 'Windows', 0
os_unknown db 'unknown', 0
; number of the drive we have booted from
bootdrv db 0
; number of system partitions
syscount db 0

; pad the remaining of the sector with zeros
times 512-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

