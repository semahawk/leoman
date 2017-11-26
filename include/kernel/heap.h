/*
 *
 * heap.h
 *
 * Created at:  07 Nov 2017 20:20:18 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <kernel/common.h>

#define CONFIG_KERNEL_HEAP_SIZE KiB(16)

void *kalloc(size_t bytes);
void kfree(void *addr);
void heap_init(void);

#endif /* !KERNEL_HEAP_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

