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
#include <kernel/heap.h>
#include <kernel/pm.h>
#include <kernel/proc.h>
#include <kernel/print.h>
#include <kernel/syscall.h>
#include <kernel/vm.h>
#include <kernel/x86.h>

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

  struct msg_packet_queue *new_msg = NULL;

  if (receiver->state != PROC_RECV_BLOCKED){
    /* if the receiver is already in the RECV_BLOCKED state, then it means a
     * packet was already already allocated, and pushed into the queue */
    new_msg = kalloc(sizeof(struct msg_packet_queue));
    memcpy(&new_msg->msg, msg, sizeof(new_msg->msg));

    new_msg->msg.sender = current_proc->pid;
    new_msg->msg.receiver = msg->receiver;
    new_msg->msg.send_buf = msg->send_buf;
    new_msg->msg.send_len = msg->send_len;
    new_msg->msg.recv_buf = msg->recv_buf;
    new_msg->msg.recv_len = msg->recv_len;
    new_msg->msg.phys_send_buf = vm_get_phys_mapping(msg->send_buf);
    new_msg->msg.phys_recv_buf = vm_get_phys_mapping(msg->recv_buf);

    STAILQ_INSERT_TAIL(&receiver->recv_queue, new_msg, msgs);
  }

  switch (receiver->state){
    case PROC_RECV_BLOCKED:
      /* send-block the sender - it now has to wait for the response */
      /* it's a way to eliminate busy-looping */
      current_proc->state = PROC_SEND_BLOCKED;

      /* TODO add a check that the queue is really non-empty */
      struct msg_packet *msg_in_queue = &STAILQ_FIRST(&receiver->recv_queue)->msg;

      void *mapped_recv_buf_base = (void *)0xdead0000;
      void *mapped_recv_buf = (void *)((uint32_t)mapped_recv_buf_base + ((uint32_t)msg_in_queue->recv_buf & 0xfff));

      /* this is what's going to be returned in the receiver's ipc_recv call */
      /* if we don't overwrite it here then it will think it got a message from
       * an undefined process */
      msg_in_queue->sender = current_proc->pid;
      msg_in_queue->send_buf = msg->send_buf;
      msg_in_queue->send_len = msg->send_len;
      msg_in_queue->phys_send_buf = vm_get_phys_mapping(msg->send_buf);

      map_pages(msg_in_queue->phys_recv_buf, mapped_recv_buf_base, 0, msg_in_queue->recv_len);
      memcpy(mapped_recv_buf, msg->send_buf, msg_in_queue->recv_len);
      unmap_pages(mapped_recv_buf_base, msg_in_queue->recv_len);

      /* the whole point of this mapping here, is to modify the ->sender field, a bit below */
      /* it's what will be returned in the ipc_recv call */
      void *mapped_msg_packet_base = (void *)0xbabe0000;
      struct msg_packet *mapped_msg_packet = (void *)((uint32_t)mapped_msg_packet_base + ((uint32_t)msg_in_queue->phys_msg_packet & 0xfff));

      map_pages(msg_in_queue->phys_msg_packet, mapped_msg_packet, 0, sizeof(struct msg_packet));
      mapped_msg_packet->sender = current_proc->pid;
      unmap_pages(mapped_msg_packet, sizeof(struct msg_packet));

      msg_in_queue->recv_buf = msg->recv_buf;
      msg_in_queue->recv_len = msg->recv_len;
      msg_in_queue->phys_recv_buf = vm_get_phys_mapping(msg->recv_buf);

      /* unblock the receiver so it can go and process the message */
      receiver->state = PROC_READY;
      break;
    case PROC_READY:
      /* reply-block the sender - it now has to wait for the response */
      /* it's a way to eliminate busy-looping */
      current_proc->state = PROC_REPLY_BLOCKED;
      break;
    default:
       kprintf("[ipc] !! the receiver (%s) has an unknown state (%d)!\n",
           receiver->name, receiver->state);
      goto err;
  }

err:
  /* XXX short window for an interrupt to come? */
  proc_enable_scheduling();
  proc_schedule_without_irq();

  /* TODO */
  return regs;
}

struct intregs *syscall_recv_msg(struct intregs *regs)
{
  proc_disable_scheduling();

  struct msg_packet *msg = (struct msg_packet *)regs->eax;

  if (STAILQ_EMPTY(&current_proc->recv_queue)){
    /* if there was no other process which sent a message to the current process
     * then block the current process (eliminating busy looping) */
    current_proc->state = PROC_RECV_BLOCKED;

    struct msg_packet_queue *new_msg = kalloc(sizeof(struct msg_packet_queue));
    memcpy(&new_msg->msg, msg, sizeof(new_msg->msg));

    new_msg->msg.phys_recv_buf   = vm_get_phys_mapping(new_msg->msg.recv_buf);
    new_msg->msg.phys_msg_packet = vm_get_phys_mapping(msg);

    STAILQ_INSERT_TAIL(&current_proc->recv_queue, new_msg, msgs);
  } else {
    struct msg_packet *msg_in_queue = &STAILQ_FIRST(&current_proc->recv_queue)->msg;

    void *mapped_send_buf_base = (void *)0xbabe0000;
    /* the sender's buffer lies in our memory at 0xbabe0000 + 12 lowest bits in
     * the sender's buffer's virtual address which are the offset into the page */
    void *mapped_send_buf = (void *)((uint32_t)mapped_send_buf_base + ((uint32_t)msg_in_queue->send_buf & 0xfff));

    /* map the physical location of the sender's buffer, into our own virtual
     * memory, located at 0xbabe0000 - this is the base address though */
    map_pages(msg_in_queue->phys_send_buf, mapped_send_buf_base, 0, msg_in_queue->send_len);
    /* transfer the data from the sender to the current process (receiver) */
    memcpy(msg->recv_buf, mapped_send_buf, msg->recv_len);
    /* we've copied the data into the receiver's address space, so we don't need
     * the mapping anymore */
    unmap_pages(mapped_send_buf, msg_in_queue->send_len);

    msg->sender = msg_in_queue->sender;
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

  if (!STAILQ_EMPTY(&current_proc->recv_queue)){
    struct msg_packet_queue *first_waiting = STAILQ_FIRST(&current_proc->recv_queue);
    struct msg_packet *msg_in_queue = &first_waiting->msg;
    struct proc *sender = proc_find_by_pid(msg_in_queue->sender);

    void *mapped_recv_buf_base = (void *)0xcafe0000;
    void *mapped_recv_buf = (void *)((uint32_t)mapped_recv_buf_base + ((uint32_t)msg_in_queue->recv_buf & 0xfff));

    map_pages(msg_in_queue->phys_recv_buf, mapped_recv_buf_base, 0, msg_in_queue->recv_len);
    memcpy(mapped_recv_buf, msg->send_buf, msg_in_queue->recv_len);
    unmap_pages(mapped_recv_buf_base, msg_in_queue->recv_len);

    STAILQ_REMOVE_HEAD(&current_proc->recv_queue, msgs);
    kfree(first_waiting);

    /* make sender be ready to use CPU time to process the response */
    sender->state = PROC_READY;
  }

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

  kprintf("[syscall] system call gates configured (int 0x%x, 0x%x, 0x%x)\n",
      SYSCALL_RPLY_MSG_VECTOR, SYSCALL_SEND_MSG_VECTOR, SYSCALL_RECV_MSG_VECTOR);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

