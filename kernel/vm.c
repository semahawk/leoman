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

uint32_t *vm_copy_kernel_pdir()
{
  uint32_t *new_pdir = pm_alloc();

  if (!new_pdir) return NULL;

  map_page_in_kernspace(new_pdir, new_pdir, 0);
  memset(new_pdir, 0x0, PAGE_SIZE);

  for (unsigned i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++){
    if (kernel_pdir[i] & PDE_P){
      if (!(kernel_pdir[i] & PDE_U)){
        /* page directory entry present and meant for kernel, let's copy */
        new_pdir[i] = kernel_pdir[i];
      }
    }
  }

  /* map the page directory into itself at the next-to-last page directory entry
   * so that when this page directory is loaded into the cr3 it will be mapped
   * and is going to be easily modifiable */
  new_pdir[1023] = (uint32_t)new_pdir | PDE_P | PDE_W;

  return new_pdir;
}

/*
 * Map a single page in kernel's address space
 */
void map_page_internal(uint32_t *pdir, uint32_t *ptab_base, void *paddr, void *vaddr, unsigned flags)
{
  paddr = PALIGNDOWN(paddr);
  vaddr = PALIGNDOWN(vaddr);

  uint32_t *ptab = ptab_base + (0x400 * vm_pdir_idx(vaddr));

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

void map_page_in_kernspace(void *paddr, void *vaddr, unsigned flags)
{
  return map_page_internal(KERN_PDIR_ADDR, KERN_PTABS_ADDR, paddr, vaddr, flags);
}

void map_page_in_userspace(void *paddr, void *vaddr, unsigned flags)
{
  return map_page_internal(USER_PDIR_ADDR, USER_PTABS_ADDR, paddr, vaddr, flags);
}

/*
 * Unmap a single page
 */
void unmap_page_internal(uint32_t *pdir, uint32_t *ptab_base, void *vaddr)
{
  vaddr = PALIGNDOWN(vaddr);

  uint32_t *ptab = ptab_base + (0x400 * vm_pdir_idx(vaddr));

  ptab[vm_ptab_idx(vaddr)] = 0x0;
}

void unmap_page_from_kernspace(void *vaddr)
{
  return unmap_page_internal(KERN_PDIR_ADDR, KERN_PTABS_ADDR, vaddr);
}

void unmap_page_from_userspace(void *vaddr)
{
  return unmap_page_internal(USER_PDIR_ADDR, USER_PTABS_ADDR, vaddr);
}

/*
 * Map <sz> memory of continuous pages
 */
void map_pages_internal(uint32_t *pdir, uint32_t *ptab_base, void *paddr, void *vaddr, unsigned flags, unsigned sz)
{
  if (sz == 0)
    return;

  paddr = PALIGNDOWN(paddr);
  vaddr = PALIGNDOWN(vaddr);

  /* the last bit is to map a sufficent number of pages */
  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    map_page_internal(pdir, ptab_base, paddr, vaddr, flags);

    paddr += PAGE_SIZE;
    vaddr += PAGE_SIZE;
  }
}

void map_pages_in_kernspace(void *paddr, void *vaddr, unsigned flags, unsigned sz)
{
  return map_pages_internal(KERN_PDIR_ADDR, KERN_PTABS_ADDR, paddr, vaddr, flags, sz);
}

void map_pages_in_userspace(void *paddr, void *vaddr, unsigned flags, unsigned sz)
{
  return map_pages_internal(USER_PDIR_ADDR, USER_PTABS_ADDR, paddr, vaddr, flags, sz);
}

/*
 * Unmap <sz> memory of continuous pages
 */
void unmap_pages_internal(uint32_t *pdir, uint32_t *ptab_base, void *vaddr, unsigned sz)
{
  if (sz == 0)
    return;

  vaddr = PALIGNDOWN(vaddr);

  /* the last bit is to map a sufficent number of pages */
  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    unmap_page_internal(pdir, ptab_base, vaddr);

    vaddr += PAGE_SIZE;
  }
}

void unmap_pages_from_kernspace(void *vaddr, unsigned sz)
{
  return unmap_pages_internal(KERN_PDIR_ADDR, KERN_PTABS_ADDR, vaddr, sz);
}

void unmap_pages_from_userspace(void *vaddr, unsigned sz)
{
  return unmap_pages_internal(USER_PDIR_ADDR, USER_PTABS_ADDR, vaddr, sz);
}

void *vm_init(struct kern_bootinfo *bootinfo)
{
  /* TODO: map stuff */

  return kernel_pdir;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

