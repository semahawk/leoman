/*
 *
 * paging.h
 *
 * Created at:  Sat  8 Nov 14:29:01 2014 14:29:01
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#include "common.h"

#define KERN_PHYS (0x00100000)
/* the virtual offset */
#define KERN_VOFF (0xe0000000)

#define PAGE_SIZE KiB(4)
/* page-align the given address */
/* shockingly, this magic works.. */
#define PALIGNUP(addr) (void *)((((uint32_t)(addr)) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PALIGNDOWN(addr) (void *)(((uint32_t)(addr)) & ~(PAGE_SIZE - 1))

/* page directory entry attribute masks */
#define PDE_IGNORE   (1 << 8) /* ignoreeeed */
#define PDE_S        (1 << 7) /* size */
/* GAP               (1 << 6) */
#define PDE_A        (1 << 5) /* accessed */
#define PDE_C        (1 << 4) /* cache disabled */
#define PDE_T        (1 << 3) /* write through */
#define PDE_U        (1 << 2) /* user */
#define PDE_W        (1 << 1) /* writeable */
#define PDE_P        (1 << 0) /* present */

/* page table entry attribute masks */
#define PTE_G        (1 << 8) /* global */
/* GAP               (1 << 7) */
#define PTE_D        (1 << 6) /* dirty */
#define PTE_A        (1 << 5) /* accessed */
#define PTE_C        (1 << 4) /* cashe disabled */
#define PTE_T        (1 << 3) /* write through */
#define PTE_U        (1 << 2) /* user */
#define PTE_W        (1 << 1) /* writeable */
#define PTE_P        (1 << 0) /* present */

uint32_t *new_page_directory(void);
void *kvm_init(struct kern_bootinfo *);
void *kalloc(void);
void  kfree(void *);
void *paging_init(struct kern_bootinfo *);
void *paddr(void *vaddr);
void map_page(void *paddr, void *vaddr, unsigned flags);
void unmap_page(void *vaddr);
void map_pages(void *paddr, void *vaddr, unsigned flags, unsigned sz);
void unmap_pages(void *vaddr, unsigned sz);

/* convert between physical and virtual addresses */
static inline void *p2v(uint32_t addr)
{
  return (void *)addr + KERN_VOFF;
}

static inline uint32_t v2p(void *addr)
{
  return (uint32_t)addr - KERN_VOFF;
}

static inline void set_cr3(uint32_t pdir)
{
  __asm volatile("movl %0, %%cr3" : : "r"(pdir));
}

/* defined in paging.c */
extern uint32_t *page_directory;

#endif /* PAGING_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

