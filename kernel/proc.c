/*
 *
 * proc.c
 *
 * Created at:  Sat 29 Nov 16:51:41 2014 16:51:41
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include "config.h"
#include "common.h"
#include "gdt.h"
#include "idt.h"
#include "pm.h"
#include "vm.h"
#include "proc.h"
#include "vga.h"
#include "timer.h"
#include "tss.h"
#include "x86.h"

static struct proc procs[NPROCS];
static uint32_t next_pid = 0;
static struct proc *idle = NULL;
volatile struct proc *current_proc = NULL;

static struct proc *find_next_proc(enum proc_state state)
{
  static int i = 0;
  int n = 0;

  for (i %= NPROCS; /* no guard */; n++, i = (i + 1) % NPROCS){
    if (n >= NPROCS)
      /* we've visited every slot */
      break;

    if (procs[i].state == state)
      return &procs[i++];
  }

  /* default to the idle process */
  return idle;
}

void proc_idle(void)
{
  while (1){
    vga_puts("i");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 2000000; i++)
      ;
  }
}

/*
 * This function pretty much just calls `proc_schedule_after_irq` (this is the
 * handler for software interrupt no. 127)
 * Well, it issues an interrupts so `proc_schedule_after_irq` has an interrupt
 * (actually, an INT not a IRQ but there's pretty much no difference) to work on
 * (the CPU state is needed to do a switch a task)
 */
void proc_schedule_without_irq(void)
{
  /*vga_printf("w/o: current_proc is %s (ss %x)\n", current_proc->name, current_proc->trapframe.ss);*/

  /* well that was simple */
  /*halt();*/
  __asm volatile("int $0x7f");
}

void proc_schedule_after_irq(struct intregs *cpu_state)
{
  struct proc *next_proc;

  /*cli();*/
  /* find a new process that could be run */
  /*next_proc = find_next_proc(PROC_SLEEPING);*/
  /*vga_printf("current: %x, next: %s\n", current_proc, next_proc->name);*/
  /*vga_printf("cpu_state->ss: %x, eax: %x\n", cpu_state->meh, cpu_state->eax);*/

  /*if (current_proc){*/
    /*current_proc->trapframe = *cpu_state;*/
  /*}*/
  /*halt();*/

#if 0

  vga_printf("aft: current_proc is %s (ss %x)\n", current_proc->name, current_proc->trapframe.ss);

  /*halt();*/

  /*vga_printf("current %s, next %s\n", current_proc->name, next_proc->name);*/

  if (current_proc == next_proc)
    return;

  /* save the current processor's state */
  /*vga_printf("0x%x before ss: 0x%x\n", (uint32_t)current_proc, current_proc->trapframe.ss);*/
  current_proc->trapframe = *cpu_state;
#if 0
  vga_row = vga_col = 0;
  vga_printf(" eax: %x   ds: %x\n", cpu_state->eax, cpu_state->ds);
  vga_printf(" ebx: %x   es: %x\n", cpu_state->ebx, cpu_state->es);
  vga_printf(" ecx: %x   fs: %x\n", cpu_state->ecx, cpu_state->fs);
  vga_printf(" edx: %x   gs: %x\n", cpu_state->edx, cpu_state->gs);
  vga_printf(" esi: %x   cs: %x\n", cpu_state->esi, cpu_state->cs);
  vga_printf(" edi: %x   ss: %x\n", cpu_state->edi, cpu_state->ss);
  vga_printf(" eip: %x  err: %x\n", cpu_state->eip, cpu_state->err);
  vga_printf(" esp: %x  num: %x\n", cpu_state->esp, cpu_state->num);
  vga_printf(" ebp: %x  flg: %x\n", cpu_state->ebp, cpu_state->eflags);
  vga_printf("\n");
#endif
  vga_printf("%s: after ss: 0x%x\n", current_proc->name, current_proc->trapframe.ss);
  /* put it to sleep */
  current_proc->state = PROC_SLEEPING;

  /* 'switch' to the next process in line */
  current_proc = next_proc;
  current_proc->state = PROC_RUNNING;

  /* TODO restore process' registers */
  /*halt();*/
  for (int i = 0; i < 700000000; i++)
    ;

  __asm volatile("movw %[ss], %%ax\n"
                 "movw %%ax, %%ds\n"
                 "movw %%ax, %%es\n"
                 "movw %%ax, %%fs\n"
                 "movw %%ax, %%gs\n"

                 "pushl %[ss]\n"
                 "pushl %[ustack]\n"
                 "pushl %[flags]\n"
                 "pushl %[cs]\n"
                 "pushl %[eip]\n"

                 /* send EOI to master PIC */
                 "movb $0x20, %%al\n"
                 "outb %%al, $0x20\n"
#if 0
                 "cli\n"
                 "hlt\n"
#endif
                 "iretl"
              :: [ss]     "g"(current_proc->trapframe.ss),
                 [ustack] "g"(current_proc->trapframe.esp),
                 [flags]  "g"(current_proc->trapframe.eflags),
                 [cs]     "g"(current_proc->trapframe.cs),
                 [eip]    "g"(current_proc->trapframe.eip));

  /* send an EOI to the master PIC */
  /*__asm volatile("movb %0, %%al" : : "N"(0x20));*/
  /*__asm volatile("outb %%al, %0" : : "N"(0x20));*/
#endif
}

struct proc *proc_new(const char *name, void *entry)
{
  cli();

  struct proc *proc = find_next_proc(PROC_UNUSED);
  uint32_t *stack = palloc() + PAGE_SIZE;

  proc->pid   = next_pid++;
  proc->state = PROC_SLEEPING;
  proc->pdir  = (uint32_t *)v2p(new_pdir());
  proc->memsz = PAGE_SIZE;
  /* set the name */
  /* FIXME use(/have even) strncpy (or even better strlcpy) */
  memcpy(proc->name, (char *)name, MAX_PROC_NAME_LEN);

  /* clear out the trapframe */
  memset(&proc->trapframe, 0x0, sizeof(proc->trapframe));

  proc->trapframe.eip = (uint32_t)entry;
  proc->trapframe.eflags = 0x200; /* interrupts enabled */
  proc->trapframe.esp = (uint32_t)stack;
  proc->trapframe.ds = SEG_UDATA;
  proc->trapframe.ss = SEG_UDATA;
  proc->trapframe.cs = SEG_UCODE;
  proc->trapframe.eax = 0xdeadbeef;

  sti();

  return proc;
}

void proc_exec(void)
{
  /* {{{ */
  cli();

  current_proc->state = PROC_RUNNING;

  /*vga_printf("current_proc %s\n", current_proc->name);*/
  /*vga_printf(" ss: 0x%x\n", current_proc->trapframe.ss);*/
  /*vga_printf("esp: 0x%x\n", current_proc->trapframe.esp);*/
  /*vga_printf("flg: 0x%x\n", current_proc->trapframe.eflags);*/
  /*vga_printf(" cs: 0x%x\n", current_proc->trapframe.cs);*/
  /*vga_printf("eip: 0x%x\n", current_proc->trapframe.eip);*/

  /*__asm volatile("movl %0, %%esp" :: "g"(current_proc->trapframe.esp));*/

  __asm volatile("movw %[ss], %%ax\n"
                 "movw %%ax, %%ds\n"
                 "movw %%ax, %%es\n"
                 "movw %%ax, %%fs\n"
                 "movw %%ax, %%gs\n"

                 "pushl %[ss]\n"
                 "pushl %[ustack]\n"
                 "pushl %[flags]\n"
                 "pushl %[cs]\n"
                 "pushl %[eip]\n"
#if 0
                 "cli\n"
                 "hlt\n"
#endif
                 "iretl"
              :: [ss]     "g"(current_proc->trapframe.ss),
                 [ustack] "g"(current_proc->trapframe.esp),
                 [flags]  "g"(current_proc->trapframe.eflags),
                 [cs]     "g"(current_proc->trapframe.cs),
                 [eip]    "g"(current_proc->trapframe.eip));

  /*halt();*/

#if 0
  /* pop the current process's data segment registers back to the CPU */
  __asm volatile("pop %gs");
  __asm volatile("pop %fs");
  __asm volatile("pop %es");
  __asm volatile("pop %ds");

  /* all the general registers */
  __asm volatile("popa");

  /* skip over the `err' and `num' */
  __asm volatile("add $8, %esp");

  /* hello current process :) */
  __asm volatile("iret");
#endif
  /* }}} */
}

void proc_earlyinit(void)
{
  cli();

  /* initialize the whole processes table to a "zero" state */
  for (int i = 0; i < NPROCS; i++){
    procs[i].pid = -1;
    procs[i].state = PROC_UNUSED;
    procs[i].trapframe.eip = 0x0;
    procs[i].trapframe.esp = 0x0;
  }

  idle = proc_new("idle", proc_idle);

  idt_set_gate(0x7f, int127, 0x8, 0x8e);
  int_install_handler(0x7f, proc_schedule_after_irq);

  sti();
}

void proc_lateinit(void)
{
  /*proc_schedule_without_irq();*/
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

