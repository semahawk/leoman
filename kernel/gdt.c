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

#include <kernel/common.h>
#include <kernel/gdt.h>
#include <kernel/tss.h>
#include <kernel/vga.h>
#include <kernel/x86.h>

/* THE mighty GDT */
static struct gdt_entry gdt[SEGNUM];

void gdt_set_segment(uint32_t idx, void *base, uint32_t limit, uint8_t access, uint8_t flags)
{
  gdt[idx].limit_low = limit & 0xffff;
  gdt[idx].limit_high = (limit >> 16) & 0xf;

  gdt[idx].base_low = (uint32_t)base & 0xffff;
  gdt[idx].base_mid = ((uint32_t)base >> 16) & 0xff;
  gdt[idx].base_high = ((uint32_t)base >> 24) & 0xff;

  /* the access byte */
  gdt[idx].access = GDTE_PRESENT | access;
  /* granularity byte */
  gdt[idx].flags = flags;
}

void gdt_init(void)
{
  /* that's kind of ugly (setting the null segment) */
  memset(&gdt[SEG_NULL_IDX], 0x0, sizeof(struct gdt_entry));

  /* 0x08 */
  gdt_set_segment(SEG_KCODE_IDX, 0x0, 0xffffffff,
      GDTE_DPL_KERNEL | GDTE_X | GDTE_W | GDTE_NOT_TSS,
      GDTE_PAGE_GRAN | GDTE_32_BIT);

  /* 0x10 */
  gdt_set_segment(SEG_KDATA_IDX, 0x0, 0xffffffff,
      GDTE_DPL_KERNEL | GDTE_R | GDTE_NOT_TSS,
      GDTE_PAGE_GRAN | GDTE_32_BIT);

  /* 0x18 */
  gdt_set_segment(SEG_UCODE_IDX, 0x0, 0xffffffff,
      GDTE_DPL_USER | GDTE_X | GDTE_W | GDTE_NOT_TSS,
      GDTE_PAGE_GRAN | GDTE_32_BIT);

  /* 0x20 */
  gdt_set_segment(SEG_UDATA_IDX, 0x0, 0xffffffff,
      GDTE_DPL_USER | GDTE_R | GDTE_NOT_TSS,
      GDTE_PAGE_GRAN | GDTE_32_BIT);

  /* task switching structure */
  tss_init(SEG_KDATA, (uint32_t)&kernel_stack_top);

  gdt_load(gdt, sizeof(gdt));
  gdt_flush();

  vga_printf("[gdt] global descriptors were set up\n");

  tss_flush(SEG_TSS);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

