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
  proc_disable_scheduling();

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
  vga_printf("[ipc] .. first byte of the message: %x\n", *(uint32_t *)send_buf);
  vga_printf("[ipc] .. message lies at 0x%x\n", send_buf);
  vga_printf("[ipc] .. it is mapped to 0x%x\n", vm_get_phys_mapping(send_buf));

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

  receiver->waiting_msg.sender = current_proc;
  receiver->waiting_msg.receiver = receiver;
  receiver->waiting_msg.send_buf = send_buf;
  receiver->waiting_msg.send_len = send_len;
  receiver->waiting_msg.recv_buf = recv_buf;
  receiver->waiting_msg.recv_len = recv_len;
  receiver->waiting_msg.phys_send_buf = vm_get_phys_mapping(send_buf);
  receiver->waiting_msg.phys_recv_buf = vm_get_phys_mapping(recv_buf);

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

  vga_printf("[ipc] process '%s' sees if a message came\n", current_proc->name);

  struct proc *sender = NULL;

  /* how much of a message do we accept */
  size_t recv_len = (size_t)regs->ecx;
  /* where to store the received message */
  void *recv_buf = (void *)regs->edi;

  void *mapped_send_buf_base = 0xbabe0000;
  /* the sender's buffer lies in our memory at 0xbabe0000 + 12 lowest bits in
   * the sender's buffer's virtual address which are the offset into the page */
  void *mapped_send_buf = (uint32_t)mapped_send_buf_base + ((uint32_t)current_proc->waiting_msg.send_buf & 0xfff);

  /* map the physical location of the sender's buffer, into our own virtual
   * memory, located at 0xbabe0000 - this is the base address though */
  map_pages(current_proc->waiting_msg.phys_send_buf, mapped_send_buf_base, 0, current_proc->waiting_msg.send_len);

  if (NULL == (sender = current_proc->waiting_msg.sender)){
    /* if there was no other process which sent a message to the current process
     * then block the current process (eliminating busy looping) */
    vga_printf("[ipc] -- no pending message\n");
    vga_printf("[ipc] -- receive-blocking the receiver\n");

    current_proc->state = PROC_RECV_BLOCKED;
  } else {
    vga_printf("[ipc] -- indeed '%s' was waiting\n", sender->name);
    vga_printf("[ipc] .. the message lies at: %x\n", current_proc->waiting_msg.phys_send_buf);
    vga_printf("[ipc] .. it's mapped into receiver's address space at: %x\n", mapped_send_buf);
    vga_printf("[ipc] .. first byte of it's message: %x\n", *(uint32_t *)mapped_send_buf);

    /* transfer the data from the sender to the current process (receiver) */
    memcpy(recv_buf, mapped_send_buf, current_proc->waiting_msg.recv_len);

    /* we've copied the data into the receiver's address space, so we don't need
     * the mapping anymore */
    unmap_pages(mapped_send_buf, current_proc->waiting_msg.send_len);

    /* 'pop' the waiting sender from the 'queue' */
    current_proc->waiting_msg.sender = NULL;
  }

  /* XXX short window for an interrupt to come? */
  proc_enable_scheduling();
  proc_schedule_without_irq();

  regs->ecx = sender->pid;

  /* TODO */
  return regs;
}

struct intregs *syscall_rply_msg(struct intregs *regs)
{
  struct proc *sender = proc_find_by_pid(regs->ecx);

  if (sender == NULL)
    /* TODO better error handling */
    return regs;

  void *send_buf = (void *)regs->esi;
  size_t send_len = (size_t)regs->ebx;

  void *mapped_recv_buf_base = 0xcafe0000;
  void *mapped_recv_buf = (uint32_t)mapped_recv_buf_base + ((uint32_t)current_proc->waiting_msg.recv_buf & 0xfff);

  map_pages(current_proc->waiting_msg.phys_recv_buf, mapped_recv_buf_base, 0, current_proc->waiting_msg.recv_len);

  vga_printf("[ipc] process '%s' wishes to reply to '%s'\n", current_proc->name, sender->name);
  vga_printf("[ipc] .. first byte of the reply: %x\n", *(uint32_t *)send_buf);
  vga_printf("[ipc] .. the sender's recv buffer lies at: %x\n", current_proc->waiting_msg.recv_buf);
  vga_printf("[ipc] .. it's mapped into receiver's address space at: %x\n", mapped_recv_buf);

  vga_printf("[ipc] filling the reply from 0x%x into 0x%x\n", (void*)send_buf, mapped_recv_buf);

  memcpy(mapped_recv_buf, send_buf, current_proc->waiting_msg.recv_len);
  vga_printf("[ipc] sender's receive buffer's first byte: 0x%x\n", *(uint32_t *)mapped_recv_buf);

  vga_printf("[ipc] -- unblocking the original sender\n");
  /* make sender be ready to use CPU time to process the response */
  sender->state = PROC_READY;

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

