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
uint32_t *kernel_pdir  = KERN_PDIR_ADDR;
uint32_t *kernel_ptabs = KERN_PTABS_ADDR;

uint32_t *new_pdir(void)
{
  uint32_t *pdir = pm_alloc();

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

  uint32_t *pdir = KERN_PDIR_ADDR;
  uint32_t *ptab = ((uint32_t *)KERN_PTABS_ADDR) + (0x400 * vm_pdir_idx(vaddr));

  if (pdir[vm_pdir_idx(vaddr)] & PDE_P){
    /* the corresponding page table exists */
    if (ptab[vm_ptab_idx(vaddr)] & PTE_P){
      /* page is already mapped */
      return;
    } else {
      /* page isn't mapped */
      ptab[vm_ptab_idx(vaddr)] = (uint32_t)paddr | PTE_P | PTE_W | flags;
    }
  } else {
    /* the page table doesn't exist */
    uint32_t *new_ptab = pm_alloc();

    pdir[vm_pdir_idx(vaddr)] = (uint32_t)new_ptab | PDE_P | PDE_W | flags;
    vm_flush_page(&pdir[vm_pdir_idx(vaddr)]);
    ptab[vm_ptab_idx(vaddr)] = (uint32_t)paddr | PTE_P | PTE_W | flags;
  }
}

/*
 * Unmap a single page
 */
void unmap_page(void *vaddr)
{
  vaddr = PALIGNDOWN(vaddr);

  uint32_t *pdir = KERN_PDIR_ADDR;
  uint32_t *ptab = ((uint32_t *)KERN_PTABS_ADDR) + (0x400 * vm_pdir_idx(vaddr));

  ptab[vm_ptab_idx(vaddr)] = 0x0;
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

  /* the last bit is to map a sufficent number of pages */
  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    map_page(paddr, vaddr, flags);

    paddr += PAGE_SIZE;
    vaddr += PAGE_SIZE;
  }
}

/*
 * Unmap <sz> memory of continuous pages
 */
void unmap_pages(void *vaddr, unsigned sz)
{
  if (sz == 0)
    return;

  vaddr = PALIGNDOWN(vaddr);

  /* the last bit is to map a sufficent number of pages */
  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    unmap_page(vaddr);

    vaddr += PAGE_SIZE;
  }
}

void *vm_init(struct kern_bootinfo *bootinfo)
{
  /* TODO: map stuff */

  return kernel_pdir;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

