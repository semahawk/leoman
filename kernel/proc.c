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
#include "paging.h"
#include "proc.h"
#include "vga.h"
#include "timer.h"
#include "tss.h"
#include "x86.h"

static struct proc procs[NPROCS];
static uint32_t next_pid = 0;
volatile struct proc *current_proc = NULL;

static struct proc *find_next_proc(enum proc_state state)
{
  static int i = 0;

  for (i %= NPROCS; /* no guard */; i = (i + 1) % NPROCS)
    if (procs[i].state == state)
      return &procs[i++];

  return NULL;
}

void proc_idle1(void)
{
  while (1){
    vga_puts("idle1 ");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 10000000; i++)
      ;
  }
}

void proc_idle2(void)
{
  while (1){
    vga_puts("idle2 ");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 10000000; i++)
      ;
  }
}

void proc_idle3(void)
{
  while (1){
    vga_puts("idle3 ");

    /* wait a bit not to flood the screen */
    for (int i = 0; i < 10000000; i++)
      ;
  }
}

void proc_sched(void)
{
  struct proc *proc = find_next_proc(PROC_SLEEPING);

  current_proc->state = PROC_SLEEPING;

  /* TODO save the current CPU state */
  /* magic time! */
  __asm volatile("add $72, %esp");
  __asm volatile("movl %%esp, %0" : "=r"(current_proc->esp));

  current_proc = proc;
  current_proc->state = PROC_RUNNING;

  __asm volatile("mov %0, %%esp" : : "r"(current_proc->esp));

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
  __asm volatile("outb %0, %1" : : "a"(0x20), "N"(0x20));

  /* goodbye :) */
  __asm volatile("iret");
}

struct proc *proc_new(void *entry)
{
  cli();

  struct proc *proc = find_next_proc(PROC_UNUSED);
  uint32_t *stack = kalloc() + PAGE_SIZE;

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
  proc->memsz = PAGE_SIZE;

  vga_printf("new proc #%d: entry at 0x%x, stack 0x%x, stacktop 0x%x\n", proc->pid, proc->eip, proc->esp, proc->esptop);

  sti();

  return proc;
}

void switch_to_userspace(void)
{

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

void proc_init(void)
{
  /* initialize the whole processes table to a "zero" state */
  for (int i = 0; i < NPROCS; i++){
    procs[i].pid    = -1;
    procs[i].eip    = 0x0;
    procs[i].esp    = 0x0;
    procs[i].esptop = 0x0;
    procs[i].state  = PROC_UNUSED;
  }

  current_proc = proc_new(proc_idle1);
  current_proc = proc_new(proc_idle2);
  current_proc = proc_new(proc_idle3);

  proc_exec();
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

