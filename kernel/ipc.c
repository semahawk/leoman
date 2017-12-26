/*
 *
 * ipc.c
 *
 * Created at:  25 Dec 2017 13:53:37 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/heap.h>
#include <kernel/ipc.h>
#include <kernel/vm.h>
#include <kernel/x86.h>

int ipc_send_msg(struct msg_packet *msg, struct proc *receiver)
{
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
       vga_printf("[ipc] !! the receiver (%s) has an unknown state (%d)!\n",
           receiver->name, receiver->state);
      return 1;
  }

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
