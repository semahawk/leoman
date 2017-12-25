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

#include <kernel/ipc.h>

int ipc_send_msg(struct msg_packet *msg, struct proc *receiver)
{
  /*vga_printf("[ipc] proc %s sends a message to %s (state: %d)\n", current_proc->name, receiver->name, receiver->state);*/

  switch (receiver->state){
    case PROC_RECV_BLOCKED:
      /* send-block the sender - it now has to wait for the response */
      /* it's a way to eliminate busy-looping */
      current_proc->state = PROC_SEND_BLOCKED;

      void *mapped_recv_buf_base = (void *)0xdead0000;
      void *mapped_recv_buf = (void *)((uint32_t)mapped_recv_buf_base + ((uint32_t)receiver->waiting_msg.recv_buf & 0xfff));

      map_pages(receiver->waiting_msg.phys_recv_buf, mapped_recv_buf_base, 0, receiver->waiting_msg.recv_len);

      memcpy(mapped_recv_buf, msg->send_buf, receiver->waiting_msg.recv_len);

      unmap_pages(mapped_recv_buf_base, receiver->waiting_msg.recv_len);

      void *mapped_msg_packet_base = (void *)0xbabe0000;
      struct msg_packet *mapped_msg_packet = (void *)((uint32_t)mapped_msg_packet_base + ((uint32_t)receiver->waiting_msg.phys_msg_packet & 0xfff));

      map_pages(receiver->waiting_msg.phys_msg_packet, mapped_msg_packet, 0, sizeof(struct msg_packet));

      /* this is what's going to be returned in the receiver's ipc_recv call */
      /* if we don't overwrite it here then it will think it got a message from
       * an undefined process */
      mapped_msg_packet->sender = current_proc->pid;
      /* don't think there were two same messages */
      receiver->waiting_msg.sender = -2;

      unmap_pages(mapped_msg_packet, sizeof(struct msg_packet));

      /* unblock the receiver so it can go and process the message */
      receiver->state = PROC_READY;
      break;
    case PROC_READY:
      receiver->waiting_msg.sender = current_proc->pid;

      /* reply-block the sender - it now has to wait for the response */
      /* it's a way to eliminate busy-looping */
      current_proc->state = PROC_REPLY_BLOCKED;
      break;
    default:
       vga_printf("[ipc] !! the receiver (%s) has an unknown state (%d)!\n",
           receiver->name, receiver->state);
      return 1;
  }

  receiver->waiting_msg.receiver = msg->receiver;
  receiver->waiting_msg.send_buf = msg->send_buf;
  receiver->waiting_msg.send_len = msg->send_len;
  receiver->waiting_msg.recv_buf = msg->recv_buf;
  receiver->waiting_msg.recv_len = msg->recv_len;
  receiver->waiting_msg.phys_send_buf = vm_get_phys_mapping(msg->send_buf);
  receiver->waiting_msg.phys_recv_buf = vm_get_phys_mapping(msg->recv_buf);

  return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
