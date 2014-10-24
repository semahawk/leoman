; param:  ECX - inode's size
;         EBX - lbn (logical block number (ie. how many blocks is it needed to
;               contain the file's contents))
; return: EAX - the result (the inode's block's size)
blksize:
  cmp ebx, NDADDR
  jge .return_fs_bsize

  inc ebx   ; lbn++
  mov eax, ebx
  call smalllblktosize
  cmp eax, ecx
  jge .return_fs_bsize

  jmp .return_fragroundup

  .return_fs_bsize:
    mov eax, [fs_bsize]
    jmp .end

  .return_fragroundup:
    mov eax, ecx
    call blkoff
    call fragroundup
    jmp .end

  .end:
  ret

; param:  EAX - loc
; return: EAX - the result of loc % fs_bsize
blkoff:
  push ebx
  push edx

  xor edx, edx
  mov ebx, [fs_bsize]
  div ebx
  mov eax, edx

  pop edx
  pop ebx
  ret

; param:  EAX - the file's length
; return: EAX - the result of loc / fs_bsize
lblkno:
  push ebx
  push edx

  xor edx, edx
  mov ebx, [fs_bsize]
  div ebx

  pop edx
  pop ebx
  ret

; param:  EAX - blk
; return: EAX - the result of blk * fs_bsize
; note:   the high part of the result is ignored
smalllblktosize:
  push ebx
  push edx
  mov ebx, [fs_bsize]
  mul ebx
  ; eax = the low part of the result
  pop edx
  pop ebx
  ret

; param:  EAX - size
; return: EAX - the result of roundup(size, fs_fsize)
fragroundup:
  push ebx
  mov ebx, [fs_fsize]
  call roundup
  pop ebx

  ret

; vi: ft=nasm:ts=2:sw=2 expandtab

