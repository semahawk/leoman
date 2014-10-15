/*
 *
 * mm.h
 *
 * Created at:  Mon 14 Apr 19:42:53 2014 19:42:53
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef MM_H
#define MM_H

#include "common.h"

#define MM_USED   1
#define MM_UNUSED 0

struct memblock {
  struct memblock *prev, *next;
  uint32_t size;
  uint8_t used; /* MM_USED / MM_UNUSED (free) */
} __PACKED;

void mm_init(uint32_t starting_point);

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif /* MM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


