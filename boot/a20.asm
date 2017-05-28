; credit: http://wiki.osdev.org/A20_Line
;
; return: 0 in AX if the a20 line is disabled (memory wraps around)
;         1 in AX if the a20 line is enabled (memory does not wrap around)
check_a20:
; {{{
  pushf
  push ds
  push es
  push di
  push si

  cli

  xor ax, ax ; ax = 0
  mov es, ax

  not ax ; ax = 0xffff
  mov ds, ax

  mov di, 0x0500
  mov si, 0x0510

  mov al, byte [es:di]
  push ax

  mov al, byte [ds:si]
  push ax

  mov byte [es:di], 0x00
  mov byte [ds:si], 0xff

  cmp byte [es:di], 0Xff

  pop ax
  mov byte [ds:si], al

  pop ax
  mov byte [es:di], al

  mov ax, 0
  je .exit

  mov ax, 1

.exit:
  pop si
  pop di
  pop es
  pop ds
  popf

  ret
; }}}

; credit: http://wiki.osdev.org/A20_Line
;
; _attempts_ to enable the a20 line using the keyboard controller
enable_a20_via_kbd:
; {{{
  cli

  call .wait
  mov al,0xAD
  out 0x64,al

  call .wait
  mov al,0xD0
  out 0x64,al

  call .wait2
  in al,0x60
  push eax

  call .wait
  mov al,0xD1
  out 0x64,al

  call .wait
  pop eax
  or al,2
  out 0x60,al

  call .wait
  mov al,0xAE
  out 0x64,al

  call .wait
  sti
  ret

.wait:
  in al,0x64
  test al,2
  jnz .wait
  ret

.wait2:
  in al,0x64
  test al,1
  jz .wait2
  ret
; }}}

; attempts to enable the a20 line using three different methods
enable_a20_or_die:
; {{{
.1:
  ; try the BIOS
  mov ax, 0x2401
  int 0x15
  ; see if it worked
  call check_a20
  test ax, ax
  jne .end
  ; it didn't, carry on
.2:
  ; try using the keyboard controller
  call enable_a20_via_kbd
  ; see if it worked
  call check_a20
  test ax, ax
  jne .end
  ; it didn't, carry on
.3:
  ; try the Fast A20 Gate
  in al, 0x92
  test al, 2
  jne .end
  or al, 2
  and al, 0xfe
  out 0x92, al
  ; check if it worked
  call check_a20
  test ax, ax
  jne .end

  ; here, it seems it didn't work at all, which is a shame
  error
.end:
  ret
; }}}

; vi: ft=nasm:ts=2:sw=2 expandtab

