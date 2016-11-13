/*
 *
 * ipc.c
 *
 * Created at:  27 Jul 2016 23:12:35 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/common.h> /* for bool type */
#include <kernel/syscall.h>
#include <ipc.h>

bool ipc_send(int receiver, void *send_buf, size_t send_len, void *recv_buf, size_t recv_len)
{
  struct msg_packet msg = {
    /* let the kernel fill in the 'sender' field by herself */
    .receiver = receiver,
    .send_buf = send_buf,
    .send_len = send_len,
    .recv_buf = recv_buf,
    .recv_len = recv_len,
  };

  /* call the kernel, and pass her the message */
  __asm volatile("movl %0, %%eax" :: "g"(&msg) : "%eax");
  __asm volatile("int %0" :: "Nd"(SYSCALL_SEND_MSG_VECTOR));

  /* TODO */
  return false;
}

int ipc_recv(void *recv_buf, size_t recv_len)
{
  int sender;

  struct msg_packet msg = {
    /* let the kernel fill in the 'sender' and 'receiver' fields for herself */
    .recv_buf = recv_buf,
    .recv_len = recv_len,
    .send_buf = NULL,
    .send_len = 0,
  };

  /* call the kernel, and pass her the message */
  __asm volatile("movl %0, %%eax" :: "g"(&msg) : "%eax");
  __asm volatile("int %0" :: "Nd"(SYSCALL_RECV_MSG_VECTOR));

  /* the kernel fills in the original sender's id */
  sender = msg.sender;

  return sender;
}

bool ipc_reply(int sender, void *send_buf, size_t send_len)
{
  struct msg_packet msg = {
    /* let the kernel fill in the 'receiver' field for herself */
    .sender   = sender,
    .send_buf = send_buf,
    .send_len = send_len,
    .recv_buf = NULL,
    .recv_len = 0,
  };

  /* call the kernel, and pass her the message */
  __asm volatile("movl %0, %%eax" :: "g"(&msg) : "%eax");
  __asm volatile("int %0" :: "Nd"(SYSCALL_RPLY_MSG_VECTOR));

  /* TODO */
  return false;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

