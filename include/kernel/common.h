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

#ifndef __ASSEMBLY__
#include <stddef.h>
#include <stdint.h>

/* few beautifiers */
#define __PACKED __attribute__((packed))
#define __NAKED  __attribute__((naked))

#define KiB(n) ((uint32_t)0x00000400 * (n))
#define MiB(n) ((uint32_t)0x00100000 * (n))
#define GiB(n) ((uint32_t)0x40000000 * (n))

#define BIT(n) (1 << (n))

/* these should later go to string.h, as soon as we have libc */
void *memset(void *, int, size_t);
void *memcpy(void *, void *, size_t);
size_t strlen(const char *);
int strcmp(const char *, const char *);

typedef enum { false = 0, true = 1 } bool;

bool streq(const char *, const char *);

struct kern_bootinfo {
  /* address of the initrd file loaded by boot1 */
  uint32_t *initrd_addr;
  /* size of the initrd file */
  uint32_t initrd_size;
  /* the total amount of available RAM memory */
  uint32_t mem_avail;
  /* the memory map entries */
  struct memory_map_entry {
    uint32_t *base_low;
    uint32_t *base_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
    uint32_t acpi_ext;
  } __PACKED memory_map[16];
} __PACKED;

/* preserved processor's state
 * passed from `isr_common_stub' to `isr_handler'
 * and    from `irq_common_stub' to `irq_handler' */
struct intregs {
  /* segment selectors */
  uint32_t gs, fs, es, ds;
  /* pushed by `pusha' */
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  /* interrupt number and error code */
  uint32_t num, err;
  /* pushed by the processor automatically */
  uint32_t eip, cs, eflags;
  /*   only when crossing rings (eg. user to kernel) */
  uint32_t useresp, ss;
};

/* provided by the linker */
extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern uint32_t kernel_size;
extern uint32_t kernel_phys;
extern uint32_t kernel_off;

extern uint32_t kernel_stack_top;
#endif /* !__ASSEMBLY__ */

#endif /* COMMON_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

