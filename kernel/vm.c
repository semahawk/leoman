/*
 *
 * vm.c
 *
 * Created at:  Sat  8 Nov 14:09:21 2014 14:09:21
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include "common.h"
#include "pm.h"
#include "vm.h"

/* kernel page directory */
       uint32_t *kernel_pdir;
static uint32_t *kernel_pdir_end;
static uint32_t *kernel_ptables_end;

uint32_t *new_pdir(void)
{
  uint32_t *pdir = palloc();

  if (!pdir)
    return NULL;

  memset(pdir, 0x0, PAGE_SIZE);

  return pdir;
}

/*
 * Map a single page
 */
void map_page(void *paddr, void *vaddr, unsigned flags)
{
  paddr = PALIGNDOWN(paddr);
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx = (uint32_t)vaddr >> 22;
  uint32_t ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

  uint32_t *pdir = kernel_pdir;
  uint32_t *ptab = kernel_pdir_end + pdir_idx * KiB(1);

  pdir[pdir_idx] |= PDE_P;
  ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTE_P
    /* TEMPORARY MEASURE */ | PTE_U;
}

/*
 * Unmap a single page
 */
void unmap_page(void *vaddr)
{
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx = (uint32_t)vaddr >> 22;
  uint32_t ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

  uint32_t *ptab = kernel_pdir_end + pdir_idx * KiB(1);

  ptab[ptab_idx] = 0x0;
}

/*
 * Map <sz> memory of continuous pages
 */
void map_pages(void *paddr, void *vaddr, unsigned flags, unsigned sz)
{
  if (sz == 0)
    return;

  paddr = PALIGNDOWN(paddr);
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx;
  uint32_t ptab_idx;

  uint32_t *pdir = kernel_pdir;
  uint32_t *ptab;

  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    pdir_idx = (uint32_t)vaddr >> 22;
    ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

    ptab = kernel_pdir_end + pdir_idx * KiB(1);

    pdir[pdir_idx] |= PDE_P;
    ptab[ptab_idx] = ((uint32_t)paddr) | (flags & 0xfff) | PTE_P;

    paddr += PAGE_SIZE;
    vaddr += PAGE_SIZE;
  }
}

/*
 * Unmap <sz> memory of continuous pages
 */
void unmap_pages(void *vaddr, unsigned sz)
{
  vaddr = PALIGNDOWN(vaddr);

  uint32_t pdir_idx;
  uint32_t ptab_idx;

  uint32_t *pdir = kernel_pdir;
  uint32_t *ptab;

  unsigned npages = sz / PAGE_SIZE;

  for (int i = 0; i < npages; i++){
    pdir_idx = (uint32_t)vaddr >> 22;
    ptab_idx = (uint32_t)vaddr >> 12 & 0x03ff;

    ptab = kernel_pdir_end + pdir_idx * KiB(1);
    ptab[ptab_idx] = 0x0;
    vaddr += PAGE_SIZE;
  }
}

void *vm_init(struct kern_bootinfo *bootinfo)
{
  /* the page directory is right after the kernel */
  kernel_pdir = PALIGNUP((uint32_t)&kernel_start + (uint32_t)&kernel_size);
  /* the page tables are right after the page directory */
  /* so yeah, whole 4MiB+4KiB are reserved for kernel's paging stuff */
  kernel_pdir_end = kernel_pdir + KiB(4);
  kernel_ptables_end = kernel_pdir_end + MiB(4);

  /* zero-out the page directory and the page tables */
  memset(kernel_pdir, 0x0, KiB(4) + MiB(4));

  for (int i = 0; i < 1024; i++)
    kernel_pdir[i] = v2p(kernel_pdir_end + i * KiB(1)) | PDE_W
      /* TODO FIXME that's just temporary */ | PDE_U;

  /* TODO FIXME fix the user's access to the pages */
  /* identity map the first 1 MiB of memory */
  map_pages(0x0, 0x0, PTE_W | PTE_U, MiB(1));
  /* map the kernel intestines to the higher half */
  map_pages(0x0, &kernel_off, PTE_W | PTE_U, ((uint32_t)&kernel_start - (uint32_t)&kernel_off) + ((uint32_t)&kernel_size) + MiB(4) + KiB(4));

  set_cr3(v2p(kernel_pdir));

  return kernel_pdir;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

