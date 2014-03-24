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

  not ax ; ax = 0xFFFF
  mov ds, ax

  mov di, 0x0500
  mov si, 0x0510

  mov al, byte [es:di]
  push ax

  mov al, byte [ds:si]
  push ax

  mov byte [es:di], 0x00
  mov byte [ds:si], 0xFF

  cmp byte [es:di], 0xFF

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

; calculate the physical address of a cylinder group
;
; param:  ECX - # of the cylinder group
; return: EDX:EAX - the address
cgloc:
; {{{
  ; cgbase(N) = fs_fpg * N
  ; cgtod(N) = cgbase(N) + fs_cblkno
  ; tell = fsbtodb(cgtod(ECX)) * d_bsize
  push ecx
  push ebx

  xor edx, edx
  mov eax, [fs_fpg]
  mul ecx

  add eax, [fs_cblkno]

  mov ecx, [fs_fsbtodb]
  shl eax, cl

  mul dword [d_bsize]

  pop ebx
  pop ecx
  ret
; }}}

; calculate the physical address of a CG's inode table
;
; param:  ECX - # of the cylinder group
; return: EDX:EAX - the address
cginoloc:
; {{{
  ; cgbase(N) = fs_fpg * N
  ; cgimin(N) = cgbase(N) + fs_iblkno
  ; phcgimin = fsbtodb(cgimin(ECX)) * d_bsize
  push ecx
  push ebx

  xor edx, edx
  mov eax, [fs_fpg]
  mul ecx

  add eax, [fs_iblkno]

  mov ecx, [fs_fsbtodb]
  shl eax, cl

  mul dword [d_bsize]

  pop ebx
  pop ecx
  ret
; }}}

; calculate the physical address of a CG's data blocks start
;
; param:  ECX - # of the cylinder group
; return: EDX:EAX - the address
cgdataloc:
; {{{
  ; cgbase(N) = fs_fpg * N
  ; cgdmin(N) = cgbase(N) + fs_dblkno
  ; phcgdmin = fsbtodb(cgdmin(ECX)) * d_bsize
  push ecx
  push ebx

  xor edx, edx
  mov eax, [fs_fpg]
  mul ecx

  add eax, [fs_dblkno]

  mov ecx, [fs_fsbtodb]
  shl eax, cl

  mul dword [d_bsize]

  pop ebx
  pop ecx
  ret
; }}}

; vi: ft=nasm:ts=2:sw=2 expandtab

