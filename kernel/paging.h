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

#define PAGE_SIZE 0x1000

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

uint32_t *paging_init(struct kern_bootinfo *);

#endif /* PAGING_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
