/*
 *
 * idt.h
 *
 * Created at:  Tue  8 Apr 12:39:12 2014 12:39:12
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

struct idt_entry {
  uint16_t base_low;
  uint16_t segm;
  uint8_t  zero;
  uint8_t  flags;
  uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
  uint16_t length;
  uint32_t base;
} __attribute((packed));

static inline void lidt(void *base, uint16_t length)
{
  struct idt_ptr idtr;

  idtr.length = length;
  idtr.base = (uint32_t)base;

  asm("lidt (%0)" : : "p"(&idtr));
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

