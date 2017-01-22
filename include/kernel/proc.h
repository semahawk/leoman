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
#include <ipc.h>

#include "common.h"

#define MAX_PROC_NAME_LEN 32
#define MAX_PROC_MESSAGES 32

#define MSG_GETPID 1

enum proc_state {
  PROC_UNUSED,
  PROC_RUNNING,
  PROC_READY,
  PROC_SEND_BLOCKED,
  PROC_RECV_BLOCKED,
  PROC_REPLY_BLOCKED,
  /* yes, I've been reading about QNX lately... */
};

struct proc {
  unsigned pid;
  enum proc_state state;
  char name[MAX_PROC_NAME_LEN + 1];
  bool privileged; /* whether it's running in ring 0 */
  bool superuser; /* whether it's running as a superuser (ring 3 but access to ports etc.) */

  struct intregs *trapframe;
  uint32_t *kstack;
  uint32_t *ustack;

  uint32_t *pdir;  /* the page directory */
  uint32_t memsz;  /* memory size the process has */

  /* this struct tells where to load the processes guts from */
  struct {
    struct {
      /* where to load from memory */
      void *address;
      /* how much to load from memory */
      uint32_t size;
    } memory;

    /* TODO: data for loading a file off of the disk */
  } location;

  /* TODO make it a queue */
  struct msg_packet waiting_msg;
};

struct proc *proc_new(const char *name, bool privileged, bool superuser);
struct proc *proc_new_from_memory(const char *name, bool privileged, bool superuser, void *addr, uint32_t size);
void proc_earlyinit(void);
void proc_kickoff_first_process(void);

void proc_set_state(int pid, enum proc_state);
void proc_awake(int pid);
int  proc_is_blocked(int pid);

void proc_disable_scheduling(void);
void proc_enable_scheduling(void);
bool proc_scheduling_enabled(void);

void proc_schedule_without_irq(void);
struct intregs *proc_schedule_after_irq(struct intregs *);

struct proc *proc_find_by_pid(int pid);

/* defined in proc.c */
extern volatile struct proc *current_proc;

#endif /* PROC_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

