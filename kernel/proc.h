/*
 *
 * proc.h
 *
 * Created at:  Fri 28 Nov 21:26:28 2014 21:26:28
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef PROC_H
#define PROC_H

#include <stdint.h>

enum proc_state {
  PROC_UNUSED,
  PROC_RUNNING,
  PROC_SLEEPING,
};

struct proc {
  unsigned pid;
  enum proc_state state;

  uint32_t eip;
  uint32_t esptop; /* the very top of the process's stack */
  uint32_t esp;    /* process's stack */

  /* these are not really used yet */
  uint32_t *pdir;  /* the page directory */
  uint32_t memsz;  /* memory size the process has */
};

struct proc *proc_new(void *);
void proc_sched(void);
void proc_earlyinit(void);
void proc_lateinit(void);
void proc_exec(void);

/* defined in proc.asm */
void switch_to_userspace(void *kstack, void *ustack);

/* defined in proc.c */
extern volatile struct proc *current_proc;

#endif /* PROC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

