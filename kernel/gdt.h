/*
 *
 * gdt.h
 *
 * Created at:  Mon 24 Nov 17:40:51 2014 17:40:51
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#include "common.h"

/* number of segments */
#define SEGNUM     0x10

#define SEG_KCODE  0x1  /* kernel code */
#define SEG_KDATA  0x2  /* kernel data */
#define SEG_UCODE  0x3  /* user code */
#define SEG_UDATA  0x4  /* user data */
#define SEG_TSS    0x5  /* task state segment */

#define GDTE_X     0x8 /* executable segment */
#define GDTE_E     0x4 /* expand down (non-executable segments) */
#define GDTE_C     0x4 /* conforming code segment (executable segments) */
#define GDTE_W     0x2 /* writeable (non-executable segments) */
#define GDTE_R     0x2 /* readable (executable segments) */
#define GDTE_A     0x1 /* accessed */

/* privilege levels */
#define DPL_KERNEL 0x0
#define DPL_USER   0x3

struct gdt_entry {
  unsigned limit_low: 16;
  unsigned base_low: 16;
  unsigned base_mid: 8;
  unsigned type: 4;
  unsigned sys: 1; /* 0 - system, 1 - application */
  unsigned dpl: 2; /* descriptor privilege level */
  unsigned present: 1;
  unsigned limit_high: 4;
  unsigned unused: 1;
  unsigned reserved: 1;
  unsigned bits: 1; /* 0 - 16 bit segment, 1 - 32 bit segment */
  unsigned granularity: 1;
  unsigned base_high: 8;
} __PACKED;

struct gdt_ptr {
  uint16_t size;
  uint32_t address;
} __PACKED;

static inline void gdt_load(void *base, uint16_t size)
{
  struct gdt_ptr gdtr;

  gdtr.size = size;
  gdtr.address = (uint32_t)base;

  __asm volatile("lgdt %0" : : "p"(gdtr));
}

void gdt_set_segment(uint32_t idx, void *base, uint32_t limit, unsigned type, unsigned dpl);
void gdt_init(void);
void gdt_flush(void);

#endif /* GDT_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

