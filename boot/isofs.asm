; load a number of sectors from the CD
; param: cx - the sector's number
;        dx - number of sectors to read
;        di - destination where to load them
load_sectors:
  pusha
  mov word [disk_address_packet.sector_num], dx
  mov word [disk_address_packet.sector], cx
  mov word [disk_address_packet.membuf], di
  xor ax, ax
  xor dx, dx
  xor bx, bx
.loop:
  mov dl, byte [bootdrv]
  mov ah, 42h
  mov si, disk_address_packet
  int 13h
  jnc .ok
  error
.ok:
  test ah, ah
  jz .done
  error
.done:
  popa

  ret

; param: cx - the LBA at which the directory might be found
load_directory:
  ; FIXME see if loading 4 sectors (2KiB) is actually sufficient
  mov dx, 0x4
  ; the location where to load the directory is fixed
  mov di, 0xe800
  call load_sectors

  ret

; param: cx - the LBA at which the file's contents might be found
;        dx - number of sectors to load
load_file:
  mov di, 0x8000
  call load_sectors

  ret

rootdir_extent_size: dd 0

initialize_isofs_utilities:
  ; load the Primary Volume Descriptor into 0x8000-0x8800
  mov cx, 0x10
  mov dx, 0x4
  mov di, 0x8000
  call load_sectors

  ; load the root directory '/'
  ; at offset 156(into the PVD)+2(into the directory structure) is the LBA of
  ; the root's directory extent - load it into 0x8800-0x9000
  mov cx, word [0x8000+156+2]
  mov eax, dword [0x8000+156+10]
  mov [rootdir_extent_size], eax
  call load_directory

  ret

; used by 'find_and_load_file'
; points to a string - the file's name to be found and loaded
fname_ptr: dd 0

find_and_load_file:
  mov [fname_ptr], si
  cmp byte [si], '/'
  jne .dont_skip_leading_slash
  inc dword [fname_ptr]
.dont_skip_leading_slash:

  ; di points to the base of the directory's entry
  mov di, 0xe800
.checkout_file_in_directory:
  ; store the directory's extent length
  mov dx, [di+10]
  xor ecx, ecx
  ; cl contains the filename's length
  mov cl, byte [di+32]
  ; si points to the currently checked-out file/dir in the current directory
  ; bx points to the directory/file's name we want to find
  lea si, [di+33]
  mov bx, [fname_ptr]
.compare_filenames:
  mov al, [bx]
  mov ah, [si]

  ; files on the ISO fs are actually named like WHATEVER;1
  cmp ah, ';'
  je .found_the_file

  ; set the fifth bit in both characters (be case insensitive :/)
  ;or ax, 0x2020
  ; if they are not equal - break early
  cmp ah, al
  jne .checkout_next_file

  inc si
  inc bx

  loop .compare_filenames

  ; if we got here it means the names matched
  cmp byte [bx], '/'
  ; assume that if next character after the name is not a forward slash then
  ; it's the final name that we're looking for
  jne .found_the_file
  ; skip the '/'
  inc bx
  ; point fname_ptr to the next segment in the path
  mov [fname_ptr], bx
  ; load the directory into the memory and traverse it for the next dir/file
  mov cx, [di+2]
  call load_directory
  jmp .checkout_file_in_directory

.checkout_next_file:
  ; see if we're above the extent's length limit
  cmp dx, 0
  jle .file_not_found

  ; offset 0 contains the directory record's length
  xor eax, eax
  mov al, byte [di+0]
  ; skip that many bytes
  add di, ax
  ; subtract from the extent's length
  sub dx, ax

  jmp .checkout_file_in_directory

.found_the_file:
  ; fetch the file's length and then divide by sector's length to get the number
  ; of sectors to load
  xor dx, dx
  mov ax, [di+10]
  mov cx, 512
  div cx ; is the sector size fixed?
  mov dx, ax
  inc dx
  mov cx, [di+2]
  call load_file
  ret

.file_not_found:
  error

; vi: ft=nasm:ts=2:sw=2 expandtab
