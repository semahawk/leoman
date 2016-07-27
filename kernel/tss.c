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

#include <kernel/common.h>
#include <kernel/gdt.h>
#include <kernel/tss.h>

static struct tss_entry tss;

void tss_set_ss(uint32_t ss)
{
  tss.ss0 = ss;
}

void tss_set_esp(uint32_t esp)
{
  tss.esp0 = esp;
}

void tss_init(uint32_t ss, uint32_t esp)
{
  void *base = &tss;
  uint32_t limit = (uint32_t)base + sizeof(tss);

  gdt_set_segment(SEG_TSS_IDX, base, limit,
      GDTE_DPL_USER | GDTE_X | GDTE_A | GDTE_TSS,
      GDTE_BYTE_GRAN | GDTE_32_BIT);

  /* make sure the TSS descriptor is initially zero */
  memset(&tss, 0x0, sizeof(struct tss_entry));

  tss.ss0 = ss;
  tss.esp0 = esp;

  tss.cs = SEG_KCODE | 3;
  tss.ss =
  tss.es =
  tss.ds =
  tss.fs =
  tss.gs = SEG_KDATA | 3;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

