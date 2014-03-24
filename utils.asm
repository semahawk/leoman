; converts a given LBA address into the CHS equivalent
;
; param: ECX - the LBA
; return: CH - cylinder
;         DH - head
;         CL - sector
;         DL - the drive we've booted from
lba_to_chs:
; {{{
  jmp $ + 3    ; skip over the variables
    c: db 0
    h: db 0
    s: db 0

  mov eax, ecx
  xor edx, edx
  xor ecx, ecx
  mov cl, [sectors_per_track]
  div dword ecx
  ; eax = LBA / sectors_per_track
  ; edx = LBA % sectors_per_track
  inc dl
  and dl, 0x3f
  mov [s], dl
  xor edx, edx
  xor ecx, ecx
  mov cl, [number_of_heads]
  div dword ecx
  ; eax = eax / number of heads
  ; edx = eax % number of heads
  mov [h], dl
  mov [c], al
  and ax, 0x300
  shr ax, 2
  or al, byte [s]
  mov [s], al

  mov ch, [c]
  mov dh, [h]
  mov cl, [s]
  mov dl, [bootdrv]

  ret
; }}}

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
cg_addr:
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
cg_inodes_addr:
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
cg_data_addr:
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

; calculate the physical address from a given inode's number
;
; param:  ECX - the inode's number
; return: EDX:EAX - the address
inode_addr:
; {{{
  ; address = cginoloc(inode / fs_ipg) + (inode % fs_ipg) * inode size
  push ecx

  xor edx, edx
  mov eax, ecx
  div dword [fs_ipg]
  ; eax = inode / fs_ipg
  ; edx = inode % fs_ipg
  push edx
  mov ecx, eax
  call cg_inodes_addr
  pop edx
  ; eax = cginoloc
  ; edx = inode % fs_ipg
  push eax
  mov eax, edx
  xor edx, edx
  mov ecx, dword INODE_SIZE
  mul dword ecx
  ; edx:eax = (inode % fs_ipg) * inode size
  pop ecx
  ; ecx = cginoloc
  add eax, ecx

  pop ecx
  ret
; }}}

; vi: ft=nasm:ts=2:sw=2 expandtab

