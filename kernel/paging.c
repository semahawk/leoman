/*
 *
 * paging.c
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
#include "paging.h"

uint32_t *paging_init(struct kern_bootinfo *bootinfo)
{
  return &kernel_size;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

