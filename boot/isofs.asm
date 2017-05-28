; the DAP used when issuing int 13h, ah=42h
disk_address_packet:
  .size: db 0x10
  .zero: db 0x00
  .sector_num: dw 0x0001
  .membuf: dd 0x00000000
  .sector: dq 0x10

; load a number of sectors from the CD
; param: cx - the sector's number
;        dx - number of sectors to read (one sector is 2KiB)
;        edi - destination where to load them
load_sectors:
  pusha
.continue:
  mov word [disk_address_packet.sector_num], 1
  mov word [disk_address_packet.sector], cx
  mov dword [disk_address_packet.membuf], 0x2000
  push edx
  push ecx
  push edi
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
  jz .relocate
  error
.relocate:
  mov ecx, 512 ; 512 * sizeof dword = 2KiB
  mov esi, 0x2000
  pop edi
.1:
  mov edx, dword [esi]
  mov dword [edi], edx

  add edi, 4
  add esi, 4
  loop .1

  pop ecx
  pop edx

  ; increase the ID of the sector we're about to load next
  inc cx
  ; decrease the number of sectors to load
  dec dx
  jnz .continue
.done:
  popa
  ret

; param: cx - the LBA at which the directory might be found
load_directory:
  ; FIXME see if loading 1 sector (2KiB) is actually sufficient
  mov dx, 0x1
  ; the location where to load the directory is fixed
  mov edi, 0x8c00
  call load_sectors

  ret

; param: cx - the LBA at which the file's contents might be found
;        dx - number of sectors to load
load_file:
  mov edi, 0x10000
  call load_sectors

  ret

rootdir_extent_size: dd 0

initialize_isofs_utilities:
  ; load the Primary Volume Descriptor into 0x8400-0x8c00 (just above the ISO
  ; bootsector)
  mov cx, 0x10
  mov dx, 0x1
  mov edi, 0x8400
  call load_sectors

  ; spade
  putchar 0x06

load_root_directory:
  ; load the root directory '/'
  ; at offset 156(into the PVD)+2(into the directory structure) is the LBA of
  ; the root's directory extent - load it into 0x8c00-0xf000
  mov cx, word [0x8400+156+2]
  mov eax, dword [0x8400+156+10]
  mov [rootdir_extent_size], eax
  call load_directory

  ret

; used by 'find_and_load_file'
; points to a string - the file's name to be found and loaded
fname_ptr: dd 0

find_and_load_file:
  call load_root_directory

  mov [fname_ptr], si
  cmp byte [si], '/'
  jne .dont_skip_leading_slash
  inc dword [fname_ptr]
.dont_skip_leading_slash:

  ; di points to the base of the directory's entry
  mov di, 0x8c00
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
  jbe .file_not_found

  ; offset 0 contains the directory record's length
  xor eax, eax
  mov al, byte [di+0]
  ; skip that many bytes
  add di, ax
  ; subtract from the extent's length
  sub dx, ax

  jmp .checkout_file_in_directory

.found_the_file:
  ; peseta
  putchar 0x9e
  ; fetch the file's length and then divide by sector's length to get the number
  ; of sectors to load
  xor dx, dx
  mov ax, [di+10]
  mov cx, 512 ; 512 or 2048 goddammit
  div cx ; is the sector size fixed?
  mov dx, ax
  mov cx, [di+2]
  mov ax, [di+10]
  call load_file
  ret

.file_not_found:
  error

; vi: ft=nasm:ts=2:sw=2 expandtab

