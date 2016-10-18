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

#include <kernel/config.h>
#include <kernel/common.h>
#include <kernel/elf.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pm.h>
#include <kernel/vm.h>
#include <kernel/proc.h>
#include <kernel/vga.h>
#include <kernel/timer.h>
#include <kernel/tss.h>
#include <kernel/x86.h>

#include <ipc.h>

static struct proc procs[NPROCS];
static uint32_t next_pid = 0;

static struct proc *idle = NULL;
volatile struct proc *current_proc = NULL;

static int scheduling_enabled = false;

static struct proc *find_next_proc(enum proc_state state)
{
  static int i = 0;
  int n = 0;

  for (i %= NPROCS; /* no guard */; n++, i = (i + 1) % NPROCS){
    if (n >= NPROCS)
      /* we've visited every slot */
      break;

    /* don't take the idle process into account */
    /* the scheduling bit takes care to use idle if there's no process to run */
    if (&procs[i] == idle)
      continue;

    if (procs[i].state == state)
      return &procs[i++];
  }

  return NULL;
}

struct proc *proc_find_by_pid(int pid)
{
  for (struct proc *proc = procs; proc < &procs[NPROCS]; proc++)
    if (proc->pid == pid)
      return proc;

  return NULL;
}

void proc_idle(void)
{
  for (;;){
    vga_printf("i");
    for (unsigned i = 0; i < 1000000; i++)
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

struct intregs *proc_schedule_after_irq(struct intregs *cpu_state)
{
  cli();

  struct proc *next_proc;

  if (!scheduling_enabled){
    return cpu_state;
  }

  if (!current_proc)
    return cpu_state;

  current_proc->trapframe = cpu_state;

  /* don't put blocked processes to sleep (they'd get scheduled then) */
  /* TODO: try passing the proc structure directly, not just the pid */
  if (!proc_is_blocked(current_proc->pid))
    current_proc->state = PROC_READY;

  /* find a new process that could be run */
  if (NULL == (next_proc = find_next_proc(PROC_READY))){
    /* if there's no process to switch to then default to idle */
    next_proc = idle;
  }

  current_proc = next_proc;

  tss_set_ss(SEG_KDATA);
  tss_set_esp((uint32_t)current_proc->kstack);
  set_cr3((uint32_t)current_proc->pdir);

  return current_proc->trapframe;
}

void proc_load(void)
{
  /* NOTE: current_proc->trapframe isn't valid here */
  /*       the contents of the trapframe have been popped during irq_common_stub
   *       before the iret */

  uint32_t *entry = NULL;
  uint32_t *addr = current_proc->location.memory.address;

  if (*addr == 0x464c457f){
    /* the contents at that memory location is an ELF */
    entry = elf_load(current_proc->location.memory.address);
  } else {
    /* assume it's straight-forward binary data */
    entry = addr;
  }

  void *stack = pm_alloc();

  uint16_t code_seg = current_proc->privileged ? SEG_KCODE : SEG_UCODE;
  uint16_t data_seg = current_proc->privileged ? SEG_KDATA : SEG_UDATA;

  map_pages(stack, (void *)VM_USER_STACK_ADDR - PAGE_SIZE,
      PTE_W | (current_proc->privileged ? 0 : PTE_U), PAGE_SIZE);

  /* this is kind of ugly.. */
  __asm volatile("movw   %0, %%ax\n"
                 "movw %%ax, %%ds\n"
                 "movw %%ax, %%es\n"
                 "movw %%ax, %%fs\n"
                 "movw %%ax, %%gs\n" :: "Nd"(data_seg) : "%ax");

  __asm volatile("movl $0xfeedbabe, %eax\n"
                 "movl $0xfeedbabe, %ebx\n"
                 "movl $0xfeedbabe, %ecx\n"
                 "movl $0xfeedbabe, %edx\n"
                 "movl $0xfeedbabe, %esi\n"
                 "movl $0xfeedbabe, %edi\n");

  __asm volatile("pushl %0" :: "g"(data_seg));
  __asm volatile("pushl %0" :: "Nd"(VM_USER_STACK_ADDR));
  __asm volatile("pushl $0x200"); /* flags, interrupts enabled */
  __asm volatile("pushl %0" :: "g"(code_seg));
  __asm volatile("pushl %0" :: "g"(entry));
  __asm volatile("iretl");
}

void proc_kickoff_first_process(void)
{
  current_proc->state = PROC_RUNNING;

  vga_printf("[proc] executing first process: %s\n", current_proc->name);

  tss_set_ss(SEG_KDATA);
  tss_set_esp((uint32_t)current_proc->kstack);
  set_cr3((uint32_t)current_proc->pdir);

  proc_enable_scheduling();

  proc_load();
}

struct proc *proc_new(const char *name, bool privileged)
{
  struct proc *proc = find_next_proc(PROC_UNUSED);
  uint32_t *stack = pm_alloc();

  map_page(stack, stack, PTE_W);

  proc->pid   = next_pid++;
  proc->state = PROC_READY;
  proc->pdir  = vm_copy_kernel_pdir();
  proc->memsz = PAGE_SIZE;
  proc->privileged = privileged;
  proc->kstack = (uint32_t *)((uint32_t)stack + PAGE_SIZE);

  proc->mailbox.head  = 0;
  proc->mailbox.tail  = 0;
  proc->mailbox.count = 0;

  /* set the name */
  /* FIXME use(/have even) strncpy (or even better strlcpy) */
  memcpy(proc->name, (char *)name, MAX_PROC_NAME_LEN);

  stack = (uint32_t *)((uint8_t *)(stack) + PAGE_SIZE);

  /* we're going to be jumping to proc_load anyway
   * that's the reason for the hardcoded KDATA/KCODE stuff */
  *--stack = SEG_KDATA; /* ss */
  *--stack = (uint32_t)proc->kstack; /* useresp */
  *--stack = 0x200; /* eflags - interrupts enabled */
  *--stack = SEG_KCODE; /* cs */
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

  *--stack = SEG_KDATA; /* ds */
  *--stack = SEG_KDATA; /* es */
  *--stack = SEG_KDATA; /* fs */
  *--stack = SEG_KDATA; /* gs */

  proc->trapframe = (struct intregs *)stack;

  current_proc = proc;

  vga_printf("[proc] new process: %s (pid %d)\n", proc->name, proc->pid);

  return proc;
}

struct proc *proc_new_from_memory(const char *name, bool user, void *addr, uint32_t size)
{
  struct proc *proc = proc_new(name, user);

  proc->location.memory.address = addr;
  proc->location.memory.size    = size;

  return proc;
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

  current_proc = idle = proc_new_from_memory("idle", true, (void *)proc_idle, 0);

  vga_printf("[proc] early stage initialized\n");
}

/* TODO: have a variant of those 'blocking' functions which would take the
 *       struct proc directly, and not have to traverse the process list */
void proc_set_state(int pid, enum proc_state state)
{
  struct proc *proc = proc_find_by_pid(pid);

  proc->state = state;
}

void proc_awake(int pid)
{
  struct proc *proc = proc_find_by_pid(pid);

  /*
   * Awaken, awaken, awaken, awaken,
   * Take the land that must be taken.
   * Awaken, awaken, awaken, awaken,
   * Devour worlds, smite, forsaken.
   *
   */
  proc->state = PROC_READY;
}

int proc_is_blocked(int pid)
{
  struct proc *proc = proc_find_by_pid(pid);

  return (proc->state == PROC_SEND_BLOCKED)
      || (proc->state == PROC_RECV_BLOCKED)
      || (proc->state == PROC_REPLY_BLOCKED)
    ;
}

void proc_disable_scheduling(void)
{
  scheduling_enabled = false;
}

void proc_enable_scheduling(void)
{
  scheduling_enabled = true;
}

bool proc_scheduling_enabled(void)
{
  return !!scheduling_enabled;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

