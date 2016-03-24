/*
 *
 * pm.h
 *
 * Created at:  Tue 16 Dec 16:28:44 2014 16:28:44
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef PM_H
#define PM_H

#include "common.h"

/* 32K of uint32_t's because 1 bit stands for one page; 32 bits in a uint32_t
 * times the page size of 4096, times 32K gives 4GiB */
#define PM_BITMAP_NMEMB 32 * 1024

void *pm_alloc(void);
void  pm_free(void *);
void *pm_init(struct kern_bootinfo *);

#endif /* PM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

