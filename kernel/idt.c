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

#include <kernel/common.h>
#include <kernel/idt.h>
#include <kernel/proc.h>
#include <kernel/print.h>
#include <kernel/vga.h>
#include <kernel/x86.h>

/* THE mighty IDT */
static struct idt_entry idt[256];

/*
 * It's not beautiful, but it works.
 */

/* ISR routines */
/* an array of function pointers, which get executed if an exception / trap
 * is raised (IDT entries below index 32) to handle it */
static void *isr_handlers[32] = { 0 };

/* IRQ subroutine table */
/* an array of function pointers, which get executed if a hardware interrupt is
 * issued (IDT entries between 32 and 47, inclusively) to handle it */
static struct irq_handler_entry irq_handlers[16] = { { .handler = NULL, .pdir = 0 } };

/* INT subroutine table */
/* an array of function pointers, which get executed if a software interrupt is
 * issued (IDT entries above 47) to handle it */
static void *int_handlers[256 - (32 + 16)] = { 0 };

/* the sizes of `{isr,irq,int}_handlers` should total in 256 4-byte entries */
/* if you were to lay down the three arrays contiguously you'd get 1:1 mapping
 * between IDT entries and the higher-level C functions that would handle the
 * corresponding interrupt */

/* note: IDT entries actually contain pointers to functions defined in
 * `idt.asm` which call a common function which in turn calls the corresponding
 * function defined in one of these arrays */

static const char *const isr_names[] =
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

static void screen_of_death(struct intregs *regs)
{
  /* {{{ */
  /* paint the screen bloood */
  int x, y;
  /* change the colors */
  vga_color = vga_make_color(COLOR_WHITE, COLOR_RED);

  for (y = 0; y < VGA_HEIGHT; y++)
    for (x = 0; x < VGA_WIDTH; x++)
      vga_putch(' ');

  vga_row = 1; vga_col = 1;

  kprintf("     \xdc\xdf\xdf\xdc \xdb  \xdc\xdf\xdf\xdb\xdc\xdc\xdc\xdc  \xdc\xdf\xdf\xdc\xdf\xdf\xdf\xdc  \xdc\xdf\xdf\xdc \xdf\xdc  \xdc\xdf\xdf\xdb\xdc\xdc\xdc\xdc  \xdc\xdf\xdf\xdf\xdf\xdc     \n");
  kprintf("    \xdb  \xdb \xdc\xdf \xde  \xdc\xdf   \xde \xdb   \xdb   \xdb \xdb  \xdb \xdb \xdb \xde  \xdc\xdf   \xde \xdb    \xdb      \n");
  kprintf("    \xde  \xdb\xdf\xdc    \xdb\xdc\xdc\xdc\xdc\xdc  \xde  \xdb\xdf\xdf\xdb\xdf  \xde  \xdb  \xdf\xdb   \xdb\xdc\xdc\xdc\xdc\xdc  \xde    \xdb      \n");
  kprintf("      \xdb   \xdb   \xdb    \xdd   \xdc\xdf    \xdb    \xdb   \xdb    \xdb    \xdd      \xdb       \n");
  kprintf("    \xdc\xdf   \xdb   \xdc\xdf\xdc\xdc\xdc\xdc   \xdb     \xdb   \xdc\xdf   \xdb    \xdc\xdf\xdc\xdc\xdc\xdc     \xdc\xdf\xdc\xdc\xdc\xdc\xdc\xdc\xdf \n");
  kprintf("    \xdb    \xde   \xdb    \xde   \xde     \xde   \xdb    \xde    \xdb    \xde     \xdb         \n");
  kprintf("    \xde    \xdc\xdf\xdf\xdc\xdf\xdf\xdf\xdc  \xdc\xdf\xdf\xdb\xdc   \xdc\xdf\xdf\xdc \xdf\xdc  \xdc\xdf\xdf\xdb\xdf\xdc\xde   \xdc\xdf\xdc\xdc\xdc\xdc \xde         \n");
  kprintf("        \xdb   \xdb   \xdb \xde \xdc\xdf \xdf\xdc \xdb  \xdb \xdb \xdb \xdb   \xdb  \xdb  \xdb \xdb    \xdd          \n");
  kprintf("        \xde  \xdb\xdf\xdf\xdf\xdf    \xdb\xdc\xdc\xdc\xdb \xde  \xdb  \xdf\xdb \xde   \xdb  \xde  \xde \xdb               \n");
  kprintf("           \xdb       \xdc\xdf   \xdb   \xdb   \xdb      \xdb       \xdb               \n");
  kprintf("         \xdc\xdf       \xdb   \xdc\xdf  \xdc\xdf   \xdb    \xdc\xdf\xdf\xdf\xdf\xdf\xdc   \xdc\xdf\xdc\xdc\xdc\xdc\xdf          \n");
  kprintf("        \xdb         \xde   \xde   \xdb    \xde   \xdb       \xdb \xdb     \xde           \n");
  kprintf("        \xde                 \xde        \xde       \xde \xde                 \n");

  kprintf("\n\n");
  kprintf("  eip:%x err:%x num:%x %s\n", regs->eip, regs->err, regs->num, isr_names[regs->num]);
  kprintf("  eax:%x ebx:%x ecx:%x edx:%x flg:%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx, regs->err);
  kprintf("  esp:%x ebp:%x esi:%x edi:%x  cs:%x\n", regs->esp, regs->ebp, regs->esi, regs->edi, regs->cs);
  kprintf("   ds:%x  es:%x  fs:%x  gs:%x  ss:%x\n", regs->ds, regs->es, regs->fs, regs->gs, regs->ss);
  kprintf("\n");

  kprintf("  Process which was running: %s\n", current_proc->name);

  /* print additional informations about the exception */
  if (regs->num == 14 /* page fault */){
    /* {{{ */
    uint8_t err = regs->err & 0x7;

    switch (err){
      case 00:
        kprintf("  supervisory process tried to read a non-present page entry\n");
        break;
      case 01:
        kprintf("  supervisory process tried to read a page and caused a protection fault\n");
        break;
      case 02:
        kprintf("  supervisory process tried to write to a non-present page entry\n");
        break;
      case 03:
        kprintf("  supervisory process tried to write to a page and caused a protection fault\n");
        break;
      case 04:
        kprintf("  user process tried to read a non-present page entry\n");
        break;
      case 05:
        kprintf("  user process tried to read a page and caused a protection fault\n");
        break;
      case 06:
        kprintf("  user process tried to write to a non-present page entry\n");
        break;
      case 07:
        kprintf("  user process tried to write to a page and caused a protection fault\n");
        break;
      default:
        kprintf("  unknown..\n");
        break;
    }


    uint32_t cr2;
    __asm volatile("mov %%cr2, %0" : "=b"(cr2));
    kprintf("  memory address where fault occured: 0x%x\n", cr2);
    /* }}} */
  }

  /* hm.. let's hang */
  /* I don't see a better option really */
  for (;;);
  /* }}} */
}

/*
 * Every function `isr[0-31]` defined in idt.asm and has a slot in the IDT calls
 * this function which dispatches the interrupt to an actual function which
 * deals with it. Pretty much the exact same follows for `irq_handler` and
 * `int_handler`
 */
void isr_handler(struct intregs *regs)
{
  if (regs->num < 32){
    isr_handler_t handler = isr_handlers[regs->num];

    if (handler){
      handler(regs);
    } else {
      /* if not found, display a upper case S on red background */
      *((uint16_t *)0xb8000) = 0xc053;
    }
  }
}

struct intregs *irq_handler(struct intregs *regs)
{
  struct intregs *ret = regs;

  if (regs->num >= 32 && regs->num < 48){
    irq_handler_t handler = irq_handlers[regs->num - 32].handler;
    pde_t pdir = irq_handlers[regs->num - 32].pdir;

    /* launch the handler if there is any associated with the IRQ being fired */
    if (handler){
      pde_t saved_pdir = get_cr3();

      /* switch to the interrupt handlers address space */
      /* it's in order to have userspace interrupt handlers possible */
      if (0 != pdir)
        set_cr3(pdir);

      ret = handler(regs);

      /* restore the original address space */
      if (0 != pdir)
        set_cr3(saved_pdir);
    } else {
      /* if not found, display an upper case Q on red background */
      *((uint16_t *)0xb8002) = 0xc051;
    }
  }

  /* if the IDT entry that was invoked was greater than 40 (meaning IRQ8-15)
   * then we need to send an End of Interrupt to the slave controller */
  if (regs->num >= 40)
    outb(0xa0, 0x20);

  /* in either case, we need to send an EOI to the master interrupt controller
   * too */
  outb(0x20, 0x20);

  return ret;
}

/*
 * Sets a handler for an IRQ of a given <num>
 */
void irq_install_handler(int num, irq_handler_t handler, pde_t pdir)
{
  if (num < 16){
    irq_handlers[num].handler = handler;
    irq_handlers[num].pdir    = pdir;
  }
  /* else error? */
}

/*
 * Unsets the handler for an IRQ of a given <num>
 */
void irq_uninstall_handler(int num)
{
  if (num < 16)
    irq_handlers[num].handler = NULL;
  /* else error? */
}

void int_handler(struct intregs *regs)
{
  if (regs->num >= 48){
    int_handler_t handler = int_handlers[regs->num - (32 + 16)];

    if (handler){
      handler(regs);
    } else {
      /* if not found, display a upper case N on red background */
      *((uint16_t *)0xb8004) = 0xc04e;
    }
  }
}

void int_install_handler(int num, int_handler_t handler)
{
  if (num >= 48)
    int_handlers[num - (32 + 16)] = handler;
}

void int_uninstall_handler(int num)
{
  if (num >= 48)
    int_handlers[num - (32 + 16)] = 0;
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

  kprintf("[idt] remapping irq 0-15 to int 32-47\n");
}

void idt_set_gate(uint8_t num, void *base, uint16_t segm, uint8_t flags)
{
  idt[num].base_low  = ((uint32_t)base & 0xffff);
  idt[num].base_high = ((uint32_t)base & 0xffff0000) >> 16;
  idt[num].zero = 0x0;
  idt[num].flags = flags;
  idt[num].segm = segm;
}

uint32_t idt_get_gate(uint8_t num)
{
  return (idt[num].base_high << 16) | idt[num].base_low;
}

static inline void idt_load(void *base, uint16_t size)
{
  struct idt_ptr idtr;

  idtr.limit = size - 1;
  idtr.base = (uint32_t)base;

  __asm volatile("lidt %0" : : "p"(idtr));
}

void idt_init(void)
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

  for (int i = 0; i < 32; i++)
    if (isr_handlers[i] == 0)
      isr_handlers[i] = screen_of_death;

  kprintf("[idt] interrupt vectors were configured\n");
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

