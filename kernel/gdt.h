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
#define SEGNUM     0x6

#define SEG_NULL_IDX   0x0
#define SEG_KCODE_IDX  0x1  /* kernel code */
#define SEG_KDATA_IDX  0x2  /* kernel data */
#define SEG_UCODE_IDX  0x3  /* user code */
#define SEG_UDATA_IDX  0x4  /* user data */
#define SEG_TSS_IDX    0x5  /* task state segment */

#define SEG_NULL  ((SEG_NULL_IDX)  * sizeof(struct gdt_entry))
#define SEG_KCODE ((SEG_KCODE_IDX) * sizeof(struct gdt_entry))
#define SEG_KDATA ((SEG_KDATA_IDX) * sizeof(struct gdt_entry))
#define SEG_UCODE ((SEG_UCODE_IDX) * sizeof(struct gdt_entry) | 3 /* RPL */)
#define SEG_UDATA ((SEG_UDATA_IDX) * sizeof(struct gdt_entry) | 3 /* RPL */)
#define SEG_TSS   ((SEG_TSS_IDX)   * sizeof(struct gdt_entry) | 3 /* RPL */)

/*
 * values to put into the access byte
 */
/* the first four bits of the access byte */
#define GDTE_X 0x8 /* executable segment */
#define GDTE_E 0x4 /* expand down (non-executable segments) */
#define GDTE_C 0x4 /* conforming code segment (executable segments) */
#define GDTE_W 0x2 /* writeable (non-executable segments) */
#define GDTE_R 0x2 /* readable (executable segments) */
#define GDTE_A 0x1 /* accessed */

/* this bit is always set to one (unless it's a TSS) */
#define GDTE_TSS 0x0
#define GDTE_NOT_TSS 0x10

/* privilege levels */
#define GDTE_DPL_KERNEL 0x00
#define GDTE_DPL_USER   0x60 /* 3 left-shifted by 5 */

/* macro for verbosity */
#define GDTE_PRESENT 0x80

/*
 * values to put into the granularity byte (well, the 4 bits)
 */
/* the first 2 bits */
/* selector's mode (either 16- or 32-bit) */
#define GDTE_16_BIT 0x0
#define GDTE_32_BIT 0x4

/* the second 2 bits */
/* byte vs page granularity */
#define GDTE_BYTE_GRAN 0x0
#define GDTE_PAGE_GRAN 0x8

struct gdt_entry {
  unsigned limit_low: 16;
  unsigned base_low: 16;
  unsigned base_mid: 8;
  /* access byte start (little endian, remember) */
  unsigned access: 8;
  /* access byte end */
  /* granularity byte start (little endian, remember) */
  unsigned limit_high: 4;
  unsigned flags: 4;
  /* granularity byte end */
  unsigned base_high: 8;
} __PACKED;

struct gdt_ptr {
  uint16_t size;
  uint32_t address;
} __PACKED;

static inline void gdt_load(void *base, uint16_t size)
{
  struct gdt_ptr gdtr;

  gdtr.size = size - 1;
  gdtr.address = (uint32_t)base;

  __asm volatile("lgdt %0" : : "p"(gdtr));
}

#if 0
void gdt_set_segment(uint32_t, void *, uint32_t, unsigned, unsigned, unsigned);
#else
void gdt_set_segment(uint32_t, void *, uint32_t, uint8_t, uint8_t);
#endif
void gdt_init(void);
void gdt_flush(void);

#endif /* GDT_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

