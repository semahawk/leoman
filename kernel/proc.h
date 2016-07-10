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

#include "common.h"

#define MAX_PROC_NAME_LEN 32

enum proc_state {
  PROC_UNUSED,
  PROC_RUNNING,
  PROC_SLEEPING,
};

struct proc {
  unsigned pid;
  enum proc_state state;
  char name[MAX_PROC_NAME_LEN + 1];

  struct intregs *trapframe;
  uint32_t *kstack;
  uint32_t *entry;

  /* these are not really used yet */
  uint32_t *pdir;  /* the page directory */
  uint32_t memsz;  /* memory size the process has */

  /* without this padding of at least 13 bytes the VGA driver stops printing to
   * screen */
  unsigned char pad[13];
};

struct proc *proc_new(const char *name, void *entry_point, bool user);
void proc_earlyinit(void);
void proc_kickoff_first_process(void);

void proc_schedule_without_irq(void);
struct intregs *proc_schedule_after_irq(struct intregs *);

/* defined in proc.c */
extern struct proc *idle;
extern volatile struct proc *current_proc;

#endif /* PROC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

