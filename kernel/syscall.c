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
#include <kernel/ipc.h>
#include <kernel/pm.h>
#include <kernel/proc.h>
#include <kernel/syscall.h>
#include <kernel/vga.h>
#include <kernel/vm.h>

#include <ipc.h>

struct intregs *syscall_send_msg(struct intregs *regs)
{
  proc_disable_scheduling();

  struct msg_packet *msg = (struct msg_packet *)regs->eax;
  struct proc *receiver = proc_find_by_pid(msg->receiver);

  if (receiver == NULL){
    /* FIXME better error handling */
    return regs;
  }

  ipc_send_msg(msg, receiver);

  /* XXX short window for an interrupt to come? */
  proc_enable_scheduling();
  proc_schedule_without_irq();

err:
  /* TODO */
  return regs;
}

struct intregs *syscall_recv_msg(struct intregs *regs)
{
  proc_disable_scheduling();

  struct msg_packet *msg = (struct msg_packet *)regs->eax;
  struct proc *sender = NULL;

  if (NULL == (sender = proc_find_by_pid(current_proc->waiting_msg.sender))){
    /* if there was no other process which sent a message to the current process
     * then block the current process (eliminating busy looping) */
    current_proc->waiting_msg.recv_len = msg->recv_len;
    current_proc->waiting_msg.recv_buf = msg->recv_buf;
    current_proc->waiting_msg.phys_recv_buf = vm_get_phys_mapping(msg->recv_buf);
    current_proc->waiting_msg.phys_msg_packet = vm_get_phys_mapping(msg);

    current_proc->state = PROC_RECV_BLOCKED;
  } else {
    void *mapped_send_buf_base = (void *)0xbabe0000;
    /* the sender's buffer lies in our memory at 0xbabe0000 + 12 lowest bits in
     * the sender's buffer's virtual address which are the offset into the page */
    void *mapped_send_buf = (void *)((uint32_t)mapped_send_buf_base + ((uint32_t)current_proc->waiting_msg.send_buf & 0xfff));

    /* map the physical location of the sender's buffer, into our own virtual
     * memory, located at 0xbabe0000 - this is the base address though */
    map_pages(current_proc->waiting_msg.phys_send_buf, mapped_send_buf_base, 0, current_proc->waiting_msg.send_len);

    /* transfer the data from the sender to the current process (receiver) */
    memcpy(msg->recv_buf, mapped_send_buf, msg->recv_len);

    /* we've copied the data into the receiver's address space, so we don't need
     * the mapping anymore */
    unmap_pages(mapped_send_buf, current_proc->waiting_msg.send_len);

    /* 'pop' the waiting sender from the 'queue' */
    /* setting it to -2 because proc_earlyinit gives every unused proc -1 */
    /* I'm not sure which party to blame for this 'bug' */
    current_proc->waiting_msg.sender = -2;

    msg->sender = sender->pid;
  }

  /* XXX short window for an interrupt to come? */
  proc_enable_scheduling();
  proc_schedule_without_irq();

  /* TODO */
  return regs;
}

struct intregs *syscall_rply_msg(struct intregs *regs)
{
  proc_disable_scheduling();

  struct msg_packet *msg = (struct msg_packet *)regs->eax;
  struct proc *sender = proc_find_by_pid(msg->sender);

  if (sender == NULL)
    /* TODO better error handling */
    return regs;

  /*vga_printf("[ipc] proc %s is replying to %s\n", current_proc->name, sender->name);*/

  void *mapped_recv_buf_base = (void *)0xcafe0000;
  void *mapped_recv_buf = (void *)((uint32_t)mapped_recv_buf_base + ((uint32_t)current_proc->waiting_msg.recv_buf & 0xfff));

  map_pages(current_proc->waiting_msg.phys_recv_buf, mapped_recv_buf_base, 0, current_proc->waiting_msg.recv_len);

  memcpy(mapped_recv_buf, msg->send_buf, current_proc->waiting_msg.recv_len);

  unmap_pages(mapped_recv_buf_base, current_proc->waiting_msg.recv_len);

  /* make sender be ready to use CPU time to process the response */
  sender->state = PROC_READY;

  proc_enable_scheduling();
  proc_schedule_without_irq();

  /* TODO */
  return regs;
}

void syscall_install(void)
{
  idt_set_gate(SYSCALL_RPLY_MSG_VECTOR, int222, 8, 0xee);
  idt_set_gate(SYSCALL_SEND_MSG_VECTOR, int186, 8, 0xee);
  idt_set_gate(SYSCALL_RECV_MSG_VECTOR, int190, 8, 0xee);

  int_install_handler(SYSCALL_RPLY_MSG_VECTOR, syscall_rply_msg);
  int_install_handler(SYSCALL_SEND_MSG_VECTOR, syscall_send_msg);
  int_install_handler(SYSCALL_RECV_MSG_VECTOR, syscall_recv_msg);

  vga_printf("[syscall] system call gates configured (int 0x%x, 0x%x, 0x%x)\n",
      SYSCALL_RPLY_MSG_VECTOR, SYSCALL_SEND_MSG_VECTOR, SYSCALL_RECV_MSG_VECTOR);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

