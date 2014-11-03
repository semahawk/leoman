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
  __asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(uint16_t port, uint8_t data)
{
  __asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

struct kern_bootinfo {
  /* address at which the kernel begins */
  uint32_t kernel_addr;
  /* kernel's size, d'uh */
  uint32_t kernel_size;
  /* the total amount of available RAM memory */
  uint32_t mem_avail;
  /* the memory map entries */
  struct memory_map_entry {
    uint32_t base_low;
    uint32_t base_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
    uint32_t acpi_ext;
  } __PACKED memory_map[64];
} __PACKED;

/* preserved processor's state
 * passed from `isr_common_stub' to `isr_handler'
 * and    from `irq_common_stub' to `irq_handler' */
struct regs {
  /* data segment selector */
  uint32_t ds;
  /* pushed by `pusha' */
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  /* interrupt number and error code */
  uint32_t num, err;
  /* pushed by the processor automatically */
  uint32_t eip, cs, eflags, useresp, ss;
};

#endif /* COMMON_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

