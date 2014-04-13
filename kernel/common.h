/*
 *
 * common.h
 *
 * Created at:  Wed  9 Apr 11:48:18 2014 11:48:18
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

/* few beautifiers */
#define __PACKED __attribute__((packed))
#define __NAKED  __attribute__((naked))

/* these should later go to string.h, as soon as we have libc */
void *memset(void *dst, int ch, size_t len);
void *memcpy(void *dst, void *src, size_t len);

static inline uint8_t inb(uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(uint16_t port, uint8_t data)
{
  asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

#endif /* COMMON_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

