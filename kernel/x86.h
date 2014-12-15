/*
 *
 * x86.h
 *
 * Created at:  Mon  8 Dec 15:43:48 2014 15:43:48
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef X86_H
#define X86_H

uint32_t get_eip(void);

static inline uint8_t inb(uint16_t port)
{
  uint8_t ret;
  __asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(uint16_t port, uint8_t data)
{
  __asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline __NAKED void cli(void)
{
  __asm volatile("cli");
}

static inline __NAKED void sti(void)
{
  __asm volatile("sti");
}

#endif /* X86_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

