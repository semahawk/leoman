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

#ifndef VM_H
#define VM_H

#include <stdint.h>

#include "common.h"

typedef uint32_t pde_t;
typedef uint32_t pte_t;

/* the physical start of kernel's guts */
/* note that .text might not begin exactly right here */
#define KERN_PHYS (0x00100000)
/* the virtual offset */
#define KERN_VOFF (0xe0000000)

#define PAGE_SIZE KiB(4)
/* page-align the given address */
/* shockingly, this magic works.. */
#define PALIGNUP(addr) (void *)((((uint32_t)(addr)) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PALIGNDOWN(addr) (void *)(((uint32_t)(addr)) & ~(PAGE_SIZE - 1))

#define KERN_PDIR_ADDR  ((void *)0xfffff000)
#define KERN_PTABS_ADDR ((void *)0xffc00000)

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

#define vm_pdir_idx(vaddr) ((((uint32_t)vaddr) >> 22))
#define vm_ptab_idx(vaddr) ((((uint32_t)vaddr) >> 12) & 0x3ff)

uint32_t *new_pdir(void);
void *vm_init(struct kern_bootinfo *);

void map_page(void *, void *, unsigned);
void unmap_page(void *);
void map_pages(void *, void *, unsigned, unsigned);
void unmap_pages(void *, unsigned);

/* convert between physical and virtual addresses */
static inline void *p2v(uint32_t addr)
{
  return (void *)addr + KERN_VOFF;
}

static inline uint32_t v2p(void *addr)
{
  return (uint32_t)addr - KERN_VOFF;
}

static inline void vm_flush_page(void *vaddr)
{
  __asm volatile("invlpg (%0)" :: "a" ((uint32_t)vaddr & 0xfffff000));
}

static inline void set_cr3(uint32_t pdir)
{
  __asm volatile("movl %0, %%cr3" : : "r"(pdir));
}

/* defined in paging.c */
extern uint32_t *page_directory;

#endif /* VM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

