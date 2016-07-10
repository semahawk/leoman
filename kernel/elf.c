/*
 *
 * elf.c
 *
 * Created at:  Thu 27 Nov 21:12:04 2014 21:12:04
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include "common.h"
#include "elf.h"
#include "vm.h"
#include "proc.h"

struct proc *elf_execute(const char *name, const void *file)
{
  struct elf_header *hdr = (struct elf_header *)file;
  struct elf_pheader *phdr = (struct elf_pheader *)(file + hdr->e_phoff);

  for (int i = 0; i < hdr->e_phnum; i++, phdr++){
    switch (phdr->p_type){
      case PT_NULL:
        /* nuthin' */
        break;

      case PT_LOAD:
        /* FIXME fix user access (PTE_U) */
        map_pages((void *)file + phdr->p_offset, (void *)phdr->p_vaddr, PTE_W | PTE_U, phdr->p_memsz);
        break;
    }
  }

  return proc_new(name, (void *)hdr->e_entry, false);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

