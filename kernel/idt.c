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
#include "idt.h"
#include "vga.h"
#include "x86.h"

/* THE mighty IDT */
static struct idt_entry idt[256];

/* IRQ subroutine table */
/* it's actually an array of function pointers */
static void *irq_routines[16] = { 0 };

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

void isr_handler(struct intregs *regs)
{
  /* {{{ */
  /* paint the screen bloood */
  int x, y;
  /* change the colors */
  vga_color = vga_make_color(COLOR_WHITE, COLOR_RED);

  /* make sure it's an ISR, not an IRQ */
  if (regs->num > 31)
    return;

  for (y = 0; y < VGA_HEIGHT; y++)
    for (x = 0; x < VGA_WIDTH; x++)
      vga_putch(' ');

  vga_row = 1; vga_col = 1;
  vga_printf("%s!\n\n", messages[regs->num]);
  vga_printf(" eax: %x   ds: %x\n", regs->eax, regs->ds);
  vga_printf(" ebx: %x   es: %x\n", regs->ebx, regs->es);
  vga_printf(" ecx: %x   fs: %x\n", regs->ecx, regs->fs);
  vga_printf(" edx: %x   gs: %x\n", regs->edx, regs->gs);
  vga_printf(" esi: %x   cs: %x\n", regs->esi, regs->cs);
  vga_printf(" edi: %x   ss: %x\n", regs->edi, regs->ss);
  vga_printf(" eip: %x  err: %x\n", regs->eip, regs->err);
  vga_printf(" flg: %x  num: %x\n", regs->eflags, regs->num);
  vga_printf("\n");

  /* print additional informations about the exception */
  vga_printf(" Additional notes / possible causes:\n");
  if (regs->num == 14 /* page fault */){
    /* {{{ */
    uint8_t err = regs->err & 0x7;

    switch (err){
      case 00:
        vga_printf(" - supervisory process tried to read a non-present\n"
                   "   page entry\n");
        break;
      case 01:
        vga_printf(" - supervisory process tried to read a page\n"
                   "   and caused a protection fault\n");
        break;
      case 02:
        vga_printf(" - supervisory process tried to write to a\n"
                   "   non-present page entry\n");
        break;
      case 03:
        vga_printf(" - supervisory process tried to write to a page\n"
                   "   and caused a protection fault\n");
        break;
      case 04:
        vga_printf(" - user process tried to read a non-present\n"
                   "   page entry\n");
        break;
      case 05:
        vga_printf(" - user process tried to read a page\n"
                   "   and caused a protection fault\n");
        break;
      case 06:
        vga_printf(" - user process tried to write to a\n"
                   "   non-present page entry\n");
        break;
      case 07:
        vga_printf(" - user process tried to write to a page\n"
                   "   and caused a protection fault\n");
        break;
      default:
        vga_printf(" - unknown..\n");
        break;
    }


    uint32_t cr2;
    __asm volatile("mov %%cr2, %0" : "=b"(cr2));
    vga_printf(" - the cr2 register contains: 0x%x\n", cr2);
    /* }}} */
  }

  /* hm.. let's hang */
  /* I don't see a better option really */
  for (;;);
  /* }}} */
}

void irq_handler(struct intregs *regs)
{
  irq_handler_t handler = irq_routines[regs->num - 32];

  /* launch the handler if there is any associated with the IRQ being fired */
  if (handler)
    handler(regs);

  /* if the IDT entry that was invoked was greater than 40 (meaning IRQ8-15)
   * then we need to send an End of Interrupt to the slave controller */
  if (regs->num >= 40)
    outb(0xa0, 0x20);

  /* in either case, we need to send an EOI to the master interrupt controller
   * too */
  outb(0x20, 0x20);
}

/*
 * Sets a handler for an IRQ of a given <num>
 */
void irq_install_handler(int num, irq_handler_t handler)
{
  irq_routines[num] = handler;
}

/*
 * Unsets the handler for an IRQ of a given <num>
 */
void irq_uninstall_handler(int num)
{
  irq_routines[num] = 0;
}

/*
 * Maps IRQs 0-15 to ISRs 32-47, as they normally are mapped to ISRs 8-15
 */
static inline void irq_remap(void)
{
  outb(0x20, 0x11);
  outb(0xa0, 0x11);
  outb(0x21, 0x20);
  outb(0xa1, 0x28);
  outb(0x21, 0x04);
  outb(0xa1, 0x02);
  outb(0x21, 0x01);
  outb(0xa1, 0x01);
  outb(0x21, 0x00);
  outb(0xa1, 0x00);
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

  __asm volatile("lidt %0" : : "p"(idtr));
}

void idt_install(void)
{
  /* zero out the IDT entries (is it actually necessary?) */
  memset(idt, 0x0, sizeof(idt));
  /* set the IDT gates */
  /* ISRs */
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
  /* IRQs */
  irq_remap();
  idt_set_gate(32, irq0,  0x8, 0x8e);
  idt_set_gate(33, irq1,  0x8, 0x8e);
  idt_set_gate(34, irq2,  0x8, 0x8e);
  idt_set_gate(35, irq3,  0x8, 0x8e);
  idt_set_gate(36, irq4,  0x8, 0x8e);
  idt_set_gate(37, irq5,  0x8, 0x8e);
  idt_set_gate(38, irq6,  0x8, 0x8e);
  idt_set_gate(39, irq7,  0x8, 0x8e);
  idt_set_gate(40, irq8,  0x8, 0x8e);
  idt_set_gate(41, irq9,  0x8, 0x8e);
  idt_set_gate(42, irq10, 0x8, 0x8e);
  idt_set_gate(43, irq11, 0x8, 0x8e);
  idt_set_gate(44, irq12, 0x8, 0x8e);
  idt_set_gate(45, irq13, 0x8, 0x8e);
  idt_set_gate(46, irq14, 0x8, 0x8e);
  idt_set_gate(47, irq15, 0x8, 0x8e);

  /* load the IDT into the processor */
  idt_load(idt, sizeof(idt));
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

