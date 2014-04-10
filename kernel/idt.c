/*
 *
 * idt.c
 *
 * Created at:  Wed  9 Apr 11:31:27 2014 11:31:27
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "isr.h"
#include "idt.h"
#include "vga.h"

/* THE mighty IDT */
static struct idt_entry idt[256];

static const char *const messages[] =
{
  /* {{{ exception messages */
  "Division by Zero",
  "Debug exception",
  "Non maskable interrupt",
  "Breakpoint exception",
  "Into detected overflow",
  "Out of bounds",
  "Invalid opcode",
  "No coprocessor",
  "Double fault",
  "Coprocessor segment overrun",
  "Bad TSS",
  "Segment not present",
  "Stack fault",
  "General protection fault",
  "Page fault",
  "Unknown interrupt",
  "Coprocessor fault",
  "Alignment check exception",
  "Machine check exception",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved"
  /* }}} */
};

void isr_handler(struct regs regs)
{
  /* paint the screen bloood */
  int x, y;
  /* change the colors */
  vga_color = vga_make_color(COLOR_WHITE, COLOR_RED);

  for (y = 0; y < VGA_HEIGHT; y++)
    for (x = 0; x < VGA_WIDTH; x++)
      vga_putch(' ');

  vga_row = 1;
  vga_col = 2;
  vga_puts(messages[regs.num]);

  /* TODO: dump the registers */

  /* hm.. let's hang */
  /* I don't see a better option really */
  for (;;);
}

static void idt_set_gate(uint8_t num, void *base, uint16_t segm, uint8_t flags)
{
  idt[num].base_low  = ((uint32_t)base & 0xffff);
  idt[num].base_high = ((uint32_t)base & 0xffff0000) >> 16;
  idt[num].zero = 0x0;
  idt[num].flags = flags;
  idt[num].segm = segm;
}

static inline void idt_load(void *base, uint16_t size)
{
  struct idt_ptr idtr;

  idtr.limit = size;
  idtr.base = (uint32_t)base;

  asm("lidt %0" : : "p"(idtr));
}

void idt_install(void)
{
  /* zero out the IDT entries (is it actually necessary?) */
  memset(idt, 0x0, sizeof(idt));
  /* set the IDT gates */
  idt_set_gate(0,  isr0,  0x8, 0x8e);
  idt_set_gate(1,  isr1,  0x8, 0x8e);
  idt_set_gate(2,  isr2,  0x8, 0x8e);
  idt_set_gate(3,  isr3,  0x8, 0x8e);
  idt_set_gate(4,  isr4,  0x8, 0x8e);
  idt_set_gate(5,  isr5,  0x8, 0x8e);
  idt_set_gate(6,  isr6,  0x8, 0x8e);
  idt_set_gate(7,  isr7,  0x8, 0x8e);
  idt_set_gate(8,  isr8,  0x8, 0x8e);
  idt_set_gate(9,  isr9,  0x8, 0x8e);
  idt_set_gate(10, isr10, 0x8, 0x8e);
  idt_set_gate(11, isr11, 0x8, 0x8e);
  idt_set_gate(12, isr12, 0x8, 0x8e);
  idt_set_gate(13, isr13, 0x8, 0x8e);
  idt_set_gate(14, isr14, 0x8, 0x8e);
  idt_set_gate(15, isr15, 0x8, 0x8e);
  idt_set_gate(16, isr16, 0x8, 0x8e);
  idt_set_gate(17, isr17, 0x8, 0x8e);
  idt_set_gate(18, isr18, 0x8, 0x8e);
  idt_set_gate(19, isr19, 0x8, 0x8e);
  idt_set_gate(20, isr20, 0x8, 0x8e);
  idt_set_gate(21, isr21, 0x8, 0x8e);
  idt_set_gate(22, isr22, 0x8, 0x8e);
  idt_set_gate(23, isr23, 0x8, 0x8e);
  idt_set_gate(24, isr24, 0x8, 0x8e);
  idt_set_gate(25, isr25, 0x8, 0x8e);
  idt_set_gate(26, isr26, 0x8, 0x8e);
  idt_set_gate(27, isr27, 0x8, 0x8e);
  idt_set_gate(28, isr28, 0x8, 0x8e);
  idt_set_gate(29, isr29, 0x8, 0x8e);
  idt_set_gate(30, isr30, 0x8, 0x8e);
  idt_set_gate(31, isr31, 0x8, 0x8e);

  /* load the IDT into the processor */
  idt_load(idt, sizeof(idt));
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

