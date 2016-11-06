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

#include <kernel/common.h>
#include <kernel/pm.h>
#include <kernel/vm.h>

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

  map_page(new_pdir, new_pdir, 0);
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
  /* basically, it's going to replace the kernel's page directory in the virtual
   * memory */
  new_pdir[1023] = (uint32_t)new_pdir | PDE_P | PDE_W;

  return new_pdir;
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
      pdir[vm_pdir_idx(vaddr)] |= flags;
      ptab[vm_ptab_idx(vaddr)]  = (uint32_t)paddr | PTE_P | PTE_W | flags;
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

/*
 * Given the virtual address <vaddr> see what physical address it is mapped to
 * in the current address space
 */
void *vm_get_phys_mapping(void *vaddr)
{
  uint32_t page_offset = (uint32_t)vaddr & 0xfff;

  vaddr = PALIGNDOWN(vaddr);

  uint32_t *pdir = KERN_PDIR_ADDR;
  uint32_t *ptab = ((uint32_t *)KERN_PTABS_ADDR) + (0x400 * vm_pdir_idx(vaddr));

  /* holy crap */
  /* the & discards the flags from the physical address */
  /* the + is because page mappings are 12-bit aligned, so the addresses the low
   * 12-bits unset - add the offset into the page (which are the last 12-bits
   * in the virtual address) to the physical address to get the final physical
   * address */
  return (void *)((uint32_t)(ptab[vm_ptab_idx(vaddr)] & (~0x3ff)) + page_offset);
}

/*
 * Allocate <sz> worth of physical pages, and map them continuously into the
 * current virtual address space, starting at address <vaddr>
 */
void vm_alloc_pages_at(void *vaddr, unsigned flags, unsigned sz)
{
  void *physical_page;

  if (sz == 0)
    return;

  vaddr = PALIGNDOWN(vaddr);

  /* the last bit is to map a sufficent number of pages */
  unsigned npages = sz / PAGE_SIZE + (sz % PAGE_SIZE > 0);

  for (int i = 0; i < npages; i++){
    physical_page = pm_alloc();

    map_page(physical_page, vaddr, flags);

    vaddr += PAGE_SIZE;
  }
}

void *vm_init(struct kern_bootinfo *bootinfo)
{
  /* TODO: map stuff */
  vga_printf("[vm] virtual memory manager was set up\n");

  return kernel_pdir;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

