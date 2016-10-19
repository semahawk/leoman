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
  /* call the kernel, and pass him the message */
  __asm volatile("movl %0, %%edi" :: "r"(recv_buf) : "edi");
  __asm volatile("movl %0, %%esi" :: "r"(send_buf) : "esi");

  __asm volatile("movl %0, %%ecx" :: "r"(recv_len) : "ecx");
  __asm volatile("movl %0, %%ebx" :: "r"(send_len) : "ebx");
  __asm volatile("movl %0, %%eax" :: "r"(receiver) : "eax");

  __asm volatile("int %0" :: "Nd"(SYSCALL_SEND_MSG_VECTOR));

  /* TODO */
  return false;
}

bool ipc_recv(void *msg, size_t len)
{
  /* TODO */
  return false;
}

bool ipc_reply(int sender, void *msg, size_t len)
{
  /* TODO */
  return false;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

