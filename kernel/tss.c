/*
 *
 * tss.c
 *
 * Created at:  Sat 29 Nov 15:36:22 2014 15:36:22
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include "gdt.h"
#include "tss.h"

static struct tss_entry tss;

void tss_set_esp(uint32_t esp)
{
  tss.esp0 = esp;
}

void tss_init(uint32_t ss, uint32_t esp)
{
  uint32_t base = (uint32_t)&tss;

  memset(&tss, 0, sizeof(struct tss_entry));

  gdt_set_segment(SEG_TSS_IDX, (void *)base, base + sizeof(struct tss_entry), GDTE_X | GDTE_A, DPL_USER, GDTE_NOSYS);

  tss.ss0 = ss;
  tss.esp0 = esp;

  tss.cs = SEG_KCODE;
  tss.ss = SEG_KDATA;
  tss.es = SEG_KDATA;
  tss.ds = SEG_KDATA;
  tss.fs = SEG_KDATA;
  tss.gs = SEG_KDATA;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

