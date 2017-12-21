/*
 *
 * atomic.h
 *
 * Created at:  21 Dec 2017 18:16:16 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_ATOMIC_H
#define KERNEL_ATOMIC_H

#include <stdint.h>

static inline void atomic_inc(volatile uint32_t *addr)
{
    __asm__ __volatile__("lock incl %0" : "=m"(*addr));
}

#endif /* !KERNEL_ATOMIC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
