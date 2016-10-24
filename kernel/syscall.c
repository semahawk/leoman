/*
 *
 * syscall.c
 *
 * Created at:  Sat Sep 26 09:23:03 2015 09:23:03
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdint.h>

#include <kernel/common.h>
#include <kernel/idt.h>
#include <kernel/pm.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <kernel/vga.h>

#include <ipc.h>

struct intregs *syscall_send_msg(struct intregs *regs)
{
  /*proc_disable_scheduling();*/

  struct proc *receiver = proc_find_by_pid(regs->ecx);

  if (receiver == NULL){
    /* FIXME better error handling */
    return regs;
  }

  size_t send_len = (size_t)regs->ebx;
  size_t recv_len = (size_t)regs->eax;

  void *send_buf = (void *)regs->esi;
  void *recv_buf = (void *)regs->edi;

  vga_printf("[ipc] process '%s' wants to send a message to '%s'\n", current_proc->name, receiver->name);

  switch (receiver->state){
    case PROC_RECV_BLOCKED:
      vga_printf("[ipc] -- the receiver is already recv-blocked\n");
      vga_printf("[ipc] -- send-blocking the sender\n");
      /* send-block the sender - it now has to wait for the response */
      /* it's a way to eliminate busy-looping */
      current_proc->state = PROC_SEND_BLOCKED;

      vga_printf("[ipc] -- unblocking the receiver\n");
      /* unblock the receiver so it can go and process the message */
      receiver->state = PROC_READY;
      break;
    case PROC_READY:
      vga_printf("[ipc] -- the receiver is ready to reply\n");
      vga_printf("[ipc] -- reply-blocking the sender\n");
      /* reply-block the sender - it now has to wait for the response */
      /* it's a way to eliminate busy-looping */
      current_proc->state = PROC_REPLY_BLOCKED;
      break;
    default:
      vga_printf("[ipc] now this is interesting...\n");
      vga_printf("[ipc] !! the receiver has no state!\n");
      goto err;
  }

  receiver->waiting_sender = current_proc;

  /*proc_enable_scheduling();*/

err:
  /* TODO */
  return regs;
}

struct intregs *syscall_recv_msg(struct intregs *regs)
{
  /*proc_disable_scheduling();*/

  vga_printf("[ipc] process '%s' sees if a message came\n", current_proc->name);

  struct proc *sender = NULL;

  /* how much of a message do we accept */
  size_t recv_len = (size_t)regs->ecx;
  /* where to store the received message */
  void *recv_buf = (void *)regs->edi;

  if (NULL == (sender = current_proc->waiting_sender)){
    /* if there was no other process which sent a message to the current process
     * then block the current process (eliminating busy looping) */
    vga_printf("[ipc] -- no pending message\n");
    vga_printf("[ipc] -- receive-blocking the receiver\n");

    current_proc->state = PROC_RECV_BLOCKED;
  } else {
    vga_printf("[ipc] -- indeed '%s' was waiting\n", sender->name);

    /* 'pop' the waiting sender from the 'queue' */
    current_proc->waiting_sender = NULL;
  }

  /*proc_enable_scheduling();*/

  /* TODO */
  return regs;
}

void syscall_install(void)
{
  idt_set_gate(SYSCALL_SEND_MSG_VECTOR, int186, 8, 0xee);
  idt_set_gate(SYSCALL_RECV_MSG_VECTOR, int190, 8, 0xee);

  int_install_handler(SYSCALL_SEND_MSG_VECTOR, syscall_send_msg);
  int_install_handler(SYSCALL_RECV_MSG_VECTOR, syscall_recv_msg);

  vga_printf("[syscall] system call gates configured (int 0x%x, 0x%x)\n", SYSCALL_SEND_MSG_VECTOR, SYSCALL_RECV_MSG_VECTOR);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

