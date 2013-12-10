BITS 16
ORG 0

start:
  jmp 0x07C0:bootloader

; The standard BIOS Parameter Block
; Taken from http://www.brokenthorn.com/Resources/OSDev6.html
times 0xB-$+start db 0x6E    ; the BPB should start at offset 0xB
BytesPerSector dw 512
SectorsPerCluster db 1
ReservedSectors dw 1
FatCopies db 2
RootDirEntries dw 224
NumSectors dw 2880
MediaType db 0xF0
SectorsPerFAT dw 9
SectorsPerTrack dw 18
NumberOfHeads dw 2
HiddenSectors dd 0
SectorsBig dd 0
DriveNumber db 0
Reserved db 0
ExtendedBootSignature db 0x29
VolumeSerialNumber dd 0xDEAFBEEF
VolubeLabel db "NIHILUM    "
FileSystemType db "FAT12   "

%include "utils.asm"

bootloader:
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

  print welcome_msg1
  print welcome_msg2
  print welcome_msg3
  print welcome_msg4

reset:
  ; reset the floppy drive
  mov ax, 0
  mov dl, [bootdrv]
  int 13h

  jc reset          ; error -> try again

read:
  mov ax, 0x0500
  mov es, ax
  mov bx, 0x0000    ; es:bx = 0500h:0000h

  ; load the second sector from the floppy
  mov ah, 0x02      ; the instruction
  mov al, 2         ; load two sectors
  mov ch, 0         ; cylinder no. 0
  mov cl, 2         ; sector no. 2
  mov dh, 0         ; head no. 0
  mov dl, [bootdrv] ; the drive from which we've booted
  int 13h           ; read!

  jc read           ; error -> try again

  ; save the number of sectors read
  mov [sectors_read], al

  ; jump to stage two
  jmp 0500h:0000h

; the strings
welcome_msg1 db 'Welcome to Nihilum  v0.01  http://github.com/semahawk/nihilum', 0dh, 0ah, 0dh, 0ah, 0
welcome_msg2 db '  Copyright (c) 2013 - Szymon Urbas', 0dh, 0ah, 0dh, 0ah, 0
welcome_msg3 db 'The source code is licensed under the 3 Clause BSD License.', 0dh, 0ah, 0
welcome_msg4 db 'For more information in that matter, please visit the LICENSE file.', 0dh, 0ah, 0dh, 0ah, 0
; some variables
sectors_read db 0
bootdrv db 0

; pad the remainder of the boot sector with n's
times 510-($-$$) db 0
; the standard PC boot signature
dw 0xAA55

; vi: ft=nasm:ts=2:sw=2 expandtab
