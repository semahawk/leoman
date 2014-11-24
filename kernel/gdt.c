/*
 *
 * gdt.c
 *
 * Created at:  Mon 24 Nov 17:37:49 2014 17:37:49
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "gdt.h"

/* THE mighty GDT */
static struct gdt_entry gdt[SEGNUM];

void gdt_set_segment(uint32_t idx, void *base, uint32_t limit, unsigned type, unsigned dpl)
{
  gdt[idx].limit_low = limit & 0xffff;
  gdt[idx].base_low = (uint32_t)base & 0xffff;
  gdt[idx].base_mid = (uint32_t)base & 0xff0000 >> 16;
  gdt[idx].type = type;
  gdt[idx].sys = 1;
  gdt[idx].dpl = dpl;
  gdt[idx].present = 1;
  gdt[idx].limit_high = limit >> 16;
  gdt[idx].unused = 1;
  gdt[idx].reserved = 1;
  gdt[idx].bits = 1; /* 32 bits */
  gdt[idx].granularity = 0; /* byte granularity */
  gdt[idx].base_high = (uint32_t)base >> 24;
}

void gdt_init(void)
{
  gdt_set_segment(SEG_KCODE, 0x0, 0xffffffff, GDTE_X | GDTE_R, DPL_KERNEL);
  gdt_set_segment(SEG_KDATA, 0x0, 0xffffffff, GDTE_W, DPL_KERNEL);
  gdt_set_segment(SEG_UCODE, 0x0, 0xffffffff, GDTE_X | GDTE_R, DPL_USER);
  gdt_set_segment(SEG_UDATA, 0x0, 0xffffffff, GDTE_W, DPL_USER);

  gdt_load(gdt, sizeof(gdt));

  reload_segments();
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

