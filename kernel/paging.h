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

/* page directory attribute masks */
#define PDIR_ATTR_IGNORE   (1 << 8)
#define PDIR_ATTR_SIZE     (1 << 7)
/* GAP                     (1 << 6) */
#define PDIR_ATTR_ACCESSED (1 << 5)
#define PDIR_ATTR_CSHDISB  (1 << 4)
#define PDIR_ATTR_WRTHRU   (1 << 3)
#define PDIR_ATTR_USER     (1 << 2)
#define PDIR_ATTR_RDWR     (1 << 1)
#define PDIR_ATTR_PRESENT  (1 << 0)

/* page table attribute masks */
#define PTAB_ATTR_GLOBAL   (1 << 8)
/* GAP                     (1 << 7) */
#define PTAB_ATTR_DIRTY    (1 << 6)
#define PTAB_ATTR_ACCESSED (1 << 5)
#define PTAB_ATTR_CSHDISB  (1 << 4)
#define PTAB_ATTR_WRTHRU   (1 << 3)
#define PTAB_ATTR_USER     (1 << 2)
#define PTAB_ATTR_RDWR     (1 << 1)
#define PTAB_ATTR_PRESENT  (1 << 0)

void *kvm_init(struct kern_bootinfo *);
void *kalloc(void);
void  kfree(void *);
void *paging_init(struct kern_bootinfo *);
void *paddr(void *vaddr);
void map_page(void *paddr, void *vaddr, unsigned int flags);
void unmap_page(void *vaddr);

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

#endif /* PAGING_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

