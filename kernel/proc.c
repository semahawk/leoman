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

struct proc *idle = NULL;
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

struct intregs *proc_schedule_after_irq(struct intregs *cpu_state)
{
  cli();

  struct proc *next_proc;

  if (!current_proc)
    return cpu_state;

  /* find a new process that could be run */
  next_proc = find_next_proc(PROC_SLEEPING);

  current_proc->trapframe = cpu_state;
  current_proc->state = PROC_SLEEPING;

  current_proc = next_proc;

  tss_set_ss(SEG_KDATA);
  tss_set_esp((uint32_t)current_proc->kstack);
  set_cr3((uint32_t)current_proc->pdir);

  return current_proc->trapframe;
}

void proc_load(void)
{
  uint32_t *entry = elf_load(current_proc->location.memory.address);

  vga_printf("proc_load says hi! (should jump to 0x%x though, pdir at %x)\n", entry, current_proc->pdir);

  for (;;);
}

struct proc *proc_new(const char *name, bool user)
{
  struct proc *proc = find_next_proc(PROC_UNUSED);
  uint32_t *stack = pm_alloc();

  map_page(stack, stack, PTE_W | (user ? PTE_U : 0));

  proc->pid   = next_pid++;
  proc->state = PROC_SLEEPING;
  proc->pdir  = vm_copy_kernel_pdir();
  proc->memsz = PAGE_SIZE;
  proc->kstack = (uint32_t *)((uint32_t)stack + PAGE_SIZE);
  /* set the name */
  /* FIXME use(/have even) strncpy (or even better strlcpy) */
  memcpy(proc->name, (char *)name, MAX_PROC_NAME_LEN);

  stack = (uint32_t *)((uint8_t *)(stack) + PAGE_SIZE);

  *--stack = user ? SEG_UDATA : SEG_KDATA; /* ss */
  *--stack = (uint32_t)proc->kstack; /* useresp */
  *--stack = 0x202; /* eflags - interrupts enabled */
  *--stack = user ? SEG_UCODE : SEG_KCODE; /* cs */
  *--stack = (uint32_t)proc_load; /* eip */

  *--stack = 0xdeadbeef; /* err */
  *--stack = 0xfeedbabe; /* num */

  *--stack = 0x0; /* eax */
  *--stack = 0x0; /* ecx */
  *--stack = 0x0; /* edx */
  *--stack = 0x0; /* ebx */
  *--stack = 0x0; /* esp */
  *--stack = 0x0; /* ebp */
  *--stack = 0x0; /* esi */
  *--stack = 0x0; /* edi */

  *--stack = user ? SEG_UDATA : SEG_KDATA; /* ds */
  *--stack = user ? SEG_UDATA : SEG_KDATA; /* es */
  *--stack = user ? SEG_UDATA : SEG_KDATA; /* fs */
  *--stack = user ? SEG_UDATA : SEG_KDATA; /* gs */

  proc->trapframe = (struct intregs *)stack;

  vga_printf("created new proc %s (kstack 0x%x, tf 0x%x, pdir 0x%x)\n", proc->name, proc->kstack, proc->trapframe, proc->pdir);

  return proc;
}

struct proc *proc_new_from_memory(const char *name, bool user, void *addr, uint32_t size)
{
  struct proc *proc = proc_new(name, user);

  proc->location.memory.address = addr;
  proc->location.memory.size    = size;

  return proc;
}

void proc_kickoff_first_process(void)
{
  vga_printf("kicking off the first process (%s)\n", current_proc->name);

  current_proc->state = PROC_RUNNING;

  tss_set_ss(SEG_KDATA);
  tss_set_esp((uint32_t)current_proc->kstack);
  set_cr3((uint32_t)current_proc->pdir);

  __asm volatile("movl %0, %%esp" :: "g"(current_proc->trapframe));
  __asm volatile("popl %gs");
  __asm volatile("popl %fs");
  __asm volatile("popl %es");
  __asm volatile("popl %ds");
  __asm volatile("popa");
  __asm volatile("addl $8, %esp");
  __asm volatile("iretl");
}

void proc_earlyinit(void)
{
  /* initialize the whole processes table to a "zero" state */
  for (int i = 0; i < NPROCS; i++){
    procs[i].pid = -1;
    procs[i].state = PROC_UNUSED;
  }

  idt_set_gate(0x7f, int127, 0x8, 0xee);
  int_install_handler(0x7f, proc_schedule_after_irq);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

