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

typedef void (*isr_handler_t)(struct intregs *);
typedef void (*irq_handler_t)(struct intregs *);
typedef void (*int_handler_t)(struct intregs *);

void idt_install(void);

void irq_install_handler(int, irq_handler_t);
void irq_uninstall_handler(int);

void int_install_handler(int, int_handler_t);
void int_uninstall_handler(int);

void idt_set_gate(uint8_t, void *, uint16_t, uint8_t);
uint32_t idt_get_gate(uint8_t);

/* {{{ IRS declarations, defined in idt.asm */
extern void isr0 (void);
extern void isr1 (void);
extern void isr2 (void);
extern void isr3 (void);
extern void isr4 (void);
extern void isr5 (void);
extern void isr6 (void);
extern void isr7 (void);
extern void isr8 (void);
extern void isr9 (void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
/* }}} */
/* {{{ IRQ declarations, defined in idt.asm */
extern void irq0 (void);
extern void irq1 (void);
extern void irq2 (void);
extern void irq3 (void);
extern void irq4 (void);
extern void irq5 (void);
extern void irq6 (void);
extern void irq7 (void);
extern void irq8 (void);
extern void irq9 (void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
/* }}} */
/* {{{ INT declarations, defined in idt.asm */
extern void int127(void); /* process scheduler */
extern void int128(void); /* syscall */
/* }}} */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

