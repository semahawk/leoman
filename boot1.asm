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

welcome:
  ; print the welcomming message (d'oh!)
  print welcome_msg

reset:
  ; reset the drive from which we've booted from
  mov ah, 00h
  mov dl, [bootdrv]
  int 13h

  jc reset          ; error -> try again

read:
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

  jc read           ; error -> try again

  ; print the partition's (types) located in the MBR
  mov cx, 5       ; starting from 5 because of the 'dec cx'
  mov ax, 0x0090
  mov es, ax
  mov si, 0x1C2   ; 1BEh + 4h
  mov dl, 0       ; the partition's menu id

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

keypress:
  ; wait for a key to be pressed
  mov ah, 0
  int 16h
  ; switch (al):
  ; case  'a':
  cmp al, 'a'
  je boot_from_hd
  ; al = al - '0'
  sub al, 48
  ; case  0 <= al < [syscount]:
  cmp al, 0
  jge boot_preamble
  ; default:
  jmp keypress

boot_preamble:
  ; al should be less than '# of systems'
  cmp al, [syscount]
  jl  boot
  jmp keypress

boot:
  print booting_msg
  jmp keypress

boot_from_hd:
  print booting_from_hd_msg

  .reset:
    ; reset the drive from which we've booted from
    mov ah, 00h
    mov dl, 80h
    int 13h

    jc .reset          ; error -> try again

  .read:
    ; set up the registers
    mov ax, 0x0000
    mov es, ax
    mov bx, 0x07C0    ; es:bx = 0000h:07C0h (= 0x7C00)

    ; load the MBR from the first hard drive
    mov ah, 0x02      ; the instruction
    mov al, 1         ; load one sector
    mov ch, 0         ; cylinder no. 0
    mov cl, 1         ; sector no. 1
    mov dh, 0         ; head no. 0
    mov dl, 0x80      ; the first hard drive
    int 13h           ; read!

    jc .read           ; error -> try again

    ; execute the MBR
    jmp 0x0000:0x07C0

halt:
  ; stop right there!
  jmp $

welcome_msg db 'Quidquid Latine dictum, sit altum videtur.', 0xD, 0xA, 0xD, 0xA, 0
boot_from_hd_msg db '[a] boot from HD', 0xD, 0xA, 0xD, 0xA, 0
booting_msg db 'booting...', 0xD, 0xA, 0
booting_from_hd_msg db 'booting from HD...', 0xD, 0xA, 0

os_bsd db 'BSD', 0
os_plan9 db 'Plan 9', 0
os_linux db 'GNU/Linux', 0
os_windoze db 'Windows', 0
os_unknown db 'unknown', 0

; number of the drive we have booted from
bootdrv db 0
; number of system partitions
syscount db 0

; pad the remaining of the sector with zeros
times 512-($-$$) db 0

; vi: ft=nasm:ts=2:sw=2 expandtab

