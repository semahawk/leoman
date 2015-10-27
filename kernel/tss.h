/*
 *
 * tss.h
 *
 * Created at:  Mon 24 Nov 19:46:46 2014 19:46:46
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef TSS_H
#define TSS_H

#include "common.h"
#include "gdt.h"

struct tss_entry {
  uint32_t prev_tss;
  uint32_t esp0;
  uint32_t ss0;
  /* everything below is unused, meh.. */
  uint32_t esp1;
  uint32_t ss1;
  uint32_t esp2;
  uint32_t ss2;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t es;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t fs;
  uint32_t gs;
  uint32_t ldt;
  uint16_t trap;
  uint16_t iomap_base;
} __PACKED;

void tss_set_esp(uint32_t);
void tss_init(uint32_t, uint32_t);
/* defined in x86.asm */
void tss_flush(uint8_t);

#endif /* TSS_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

