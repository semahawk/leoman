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

#include <stdint.h>

#include "common.h"

struct idt_entry {
  uint16_t base_low;
  uint16_t segm;
  uint8_t  zero;
  uint8_t  flags;
  uint16_t base_high;
} __PACKED;

struct idt_ptr {
  uint16_t limit;
  uint32_t base;
} __PACKED;

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

void idt_install(void);

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

