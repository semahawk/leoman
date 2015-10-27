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

void proc_sched(void)
{
  struct proc *proc = find_next_proc(PROC_SLEEPING);

  current_proc->state = PROC_SLEEPING;

  /* the CPU state is actually saved, by the interrupt handler
   * (timer_handler), but it's 18 dwords below the top of the stack */
  /* why 18? yeah, I have no freakin' idea... */
  __asm volatile("add $72, %esp");

  __asm volatile("movl %%esp, %0" : "=r"(current_proc->esp));

  current_proc = proc;
  current_proc->state = PROC_RUNNING;

  /*__asm volatile("movl %0, %%cr3" : : "r"(current_proc->pdir));*/
  __asm volatile("movl %0, %%esp" : : "r"(current_proc->esp));

  /* restore all the data segments */
  __asm volatile("pop %gs");
  __asm volatile("pop %fs");
  __asm volatile("pop %es");
  __asm volatile("pop %ds");

  /* restore the general registers */
  __asm volatile("popa");

  /* skip over the `err' and `num' */
  __asm volatile("add $8, %esp");

  /* send an EOI to the master PIC */
  __asm volatile("movb %0, %%al" : : "N"(0x20));
  __asm volatile("outb %%al, %0" : : "N"(0x20));

  /* goodbye :) */
  __asm volatile("iret");
}

struct proc *proc_new(void *entry)
{
  cli();

  struct proc *proc = find_next_proc(PROC_UNUSED);
  uint32_t *stack = palloc() + PAGE_SIZE;

  proc->esptop = (uint32_t)stack;

  /* making the stack correspond to struct intregs */
  /* processor data */
  *--stack = 0x0;             /* ss */
  *--stack = 0x0;             /* useresp */
  *--stack = 0x202;           /* eflags */
  *--stack = SEG_KCODE;       /* cs */
  *--stack = (uint32_t)entry; /* eip */

  *--stack = 0xdeadbeef;      /* err */
  *--stack = 0xcafebabe;      /* num */

  /* pusha */
  *--stack = 0x0;             /* eax */
  *--stack = 0x0;             /* ecx */
  *--stack = 0x0;             /* edx */
  *--stack = 0x0;             /* ebx */
  *--stack = 0x0;             /* esp */
  *--stack = 0x0;             /* ebp */
  *--stack = 0x0;             /* esi */
  *--stack = 0x0;             /* edi */

  /* data segments */
  *--stack = SEG_KDATA;       /* ds */
  *--stack = SEG_KDATA;       /* es */
  *--stack = SEG_KDATA;       /* fs */
  *--stack = SEG_KDATA;       /* gs */

  proc->pid   = next_pid++;
  proc->state = PROC_SLEEPING;
  proc->esp   = (uint32_t)stack;
  proc->eip   = (uint32_t)entry;
  proc->pdir  = (uint32_t *)v2p(new_pdir());
  proc->memsz = PAGE_SIZE;

  /*vga_printf("new proc #%d: entry at 0x%x, stack 0x%x, stacktop 0x%x, pdir 0x%x\n", proc->pid, proc->eip, proc->esp, proc->esptop, proc->pdir);*/

  sti();

  return proc;
}

void proc_exec(void)
{
  current_proc->state = PROC_RUNNING;

  __asm volatile("movl %0, %%esp" : : "g"(current_proc->esp));

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
}

void proc_earlyinit(void)
{
  cli();

  /* initialize the whole processes table to a "zero" state */
  for (int i = 0; i < NPROCS; i++){
    procs[i].pid    = -1;
    procs[i].eip    = 0x0;
    procs[i].esp    = 0x0;
    procs[i].esptop = 0x0;
    procs[i].state  = PROC_UNUSED;
  }

  sti();
}

void proc_lateinit(void)
{
  cli();

  current_proc = idle = proc_new(proc_idle);

  cli();
  /* proc_exec switches interrupts on */
  proc_exec();
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

