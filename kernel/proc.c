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

void proc_idle1(void)
{
  /*static int i;*/

  while (1){
    vga_putch('1');

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 200000; i++)
      ;
  }
}

void proc_idle2(void)
{
  /*static int i;*/

  while (1){
    vga_puts("2");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 200000; i++)
      ;
  }
}

void proc_idle3(void)
{
  /*static int i;*/

  while (1){
    vga_puts("3");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 200000; i++)
      ;
  }
}

void proc_idle4(void)
{
  /*static int i;*/

  while (1){
    vga_puts("4");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 200000; i++)
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
  /* well that was simple */
  __asm volatile("int $0x7f");
}

void proc_schedule_after_irq(struct intregs *cpu_state)
{
  struct proc *next_proc;

  /* find a new process that could be run */
  next_proc = find_next_proc(PROC_SLEEPING);

  if (current_proc == next_proc)
    return;

  /* save processor's current state */
  current_proc->trapframe = *cpu_state;

  /* put it to sleep */
  current_proc->state = PROC_SLEEPING;

  /* 'switch' to the next process in line */
  current_proc = next_proc;
  current_proc->state = PROC_RUNNING;

  __asm volatile("movw %[ss], %%ax\n"
                 "movw %%ax, %%ds\n"
                 "movw %%ax, %%es\n"
                 "movw %%ax, %%fs\n"
                 "movw %%ax, %%gs\n"

                 "movl %[eax], %%eax\n"
                 "movl %[ebx], %%ebx\n"
                 "movl %[ecx], %%ecx\n"
                 "movl %[edx], %%edx\n"
                 "movl %[esi], %%esi\n"
                 "movl %[edi], %%edi\n"

                 /* send EOI to master PIC */
                 "movb $0x20, %%al\n"
                 "outb %%al, $0x20\n"

                 "pushl %[ss]\n"
                 "pushl %[ustack]\n"
                 "pushl %[flags]\n"
                 "pushl %[cs]\n"
                 "pushl %[eip]\n"

                 "iretl"
              :: [eax]    "g"(current_proc->trapframe.eax),
                 [ebx]    "g"(current_proc->trapframe.ebx),
                 [ecx]    "g"(current_proc->trapframe.ecx),
                 [edx]    "g"(current_proc->trapframe.edx),
                 [esi]    "g"(current_proc->trapframe.esi),
                 [edi]    "g"(current_proc->trapframe.edi),
                 [ss]     "g"(current_proc->trapframe.ss),
                 [ustack] "g"(current_proc->trapframe.esp),
                 [flags]  "g"(current_proc->trapframe.eflags),
                 [cs]     "g"(current_proc->trapframe.cs),
                 [eip]    "g"(current_proc->trapframe.eip));
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
  proc->trapframe.eflags = 0x202; /* interrupts enabled */
  proc->trapframe.esp = (uint32_t)stack;
  proc->trapframe.ds = SEG_UDATA;
  proc->trapframe.ss = SEG_UDATA;
  proc->trapframe.cs = SEG_UCODE;
  proc->trapframe.eax = 0xdeadbeef;
  proc->trapframe.ebx = 0x0badc0de;
  proc->trapframe.ecx = 0xfee1dead;
  proc->trapframe.edx = 0xdee9c0de;
  proc->trapframe.esi = 0xfacefeed;
  proc->trapframe.edi = 0x00bada55;

  sti();

  return proc;
}

void proc_exec(void)
{
  /* {{{ */
  cli();

  current_proc->state = PROC_RUNNING;

  __asm volatile("movw %[ss], %%ax\n"
                 "movw %%ax, %%ds\n"
                 "movw %%ax, %%es\n"
                 "movw %%ax, %%fs\n"
                 "movw %%ax, %%gs\n"

                 "movl %[eax], %%eax\n"
                 "movl %[ebx], %%ebx\n"
                 "movl %[ecx], %%ecx\n"
                 "movl %[edx], %%edx\n"
                 "movl %[esi], %%esi\n"
                 "movl %[edi], %%edi\n"

                 /* send EOI to master PIC */
                 "movb $0x20, %%al\n"
                 "outb %%al, $0x20\n"

                 "pushl %[ss]\n"
                 "pushl %[ustack]\n"
                 "pushl %[flags]\n"
                 "pushl %[cs]\n"
                 "pushl %[eip]\n"

                 "iretl"
              :: [eax]    "g"(current_proc->trapframe.eax),
                 [ebx]    "g"(current_proc->trapframe.ebx),
                 [ecx]    "g"(current_proc->trapframe.ecx),
                 [edx]    "g"(current_proc->trapframe.edx),
                 [esi]    "g"(current_proc->trapframe.esi),
                 [edi]    "g"(current_proc->trapframe.edi),
                 [ss]     "g"(current_proc->trapframe.ss),
                 [ustack] "g"(current_proc->trapframe.esp),
                 [flags]  "g"(current_proc->trapframe.eflags),
                 [cs]     "g"(current_proc->trapframe.cs),
                 [eip]    "g"(current_proc->trapframe.eip));
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

  current_proc = idle = proc_new("idle1", proc_idle1);
  proc_new("idle2", proc_idle2);
  proc_new("idle3", proc_idle3);
  proc_new("idle4", proc_idle4);

  idt_set_gate(0x7f, int127, 0x8, 0xee);
  int_install_handler(0x7f, proc_schedule_after_irq);

  sti();
}

void proc_lateinit(void)
{
  proc_exec();
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

