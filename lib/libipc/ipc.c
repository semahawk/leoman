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

#include <stdio.h>
#include <kernel/syscall.h>
#include <ipc.h>

int ipc_send(int receiver, struct msg *msg)
{
  msg->receiver = receiver;

  /* call the kernel, and pass him the message */
  __asm volatile("movl %0, %%eax" :: "r"(msg) : "eax");
  __asm volatile("int %0" :: "Nd"(SYSCALL_SEND_MSG_VECTOR));

  /* FIXME */
  return 1;
}

int ipc_recv(int sender, struct msg *msg)
{
  int any_msg_received;

  /* call the kernel, and pass him the pointer to message to fill in */
  __asm volatile("movl %0, %%ebx" :: "r"(sender) : "ebx");
  __asm volatile("movl %0, %%eax" :: "r"(msg) : "eax");
  __asm volatile("int %0" :: "Nd"(SYSCALL_RECV_MSG_VECTOR));

  /* the recv_msg syscall uses the eax register to indicate whether there was a
   * message fetched from the process' mailbox */
  __asm volatile("movl %%eax, %0" : "=a"(any_msg_received));

  return any_msg_received;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

