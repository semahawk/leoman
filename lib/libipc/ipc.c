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
  __asm volatile("movl %0, %%edi" :: "g"(recv_buf) : "%edi");
  __asm volatile("movl %0, %%esi" :: "g"(send_buf) : "%esi");

  __asm volatile("movl %0, %%ecx" :: "g"(receiver) : "%ecx");
  __asm volatile("movl %0, %%ebx" :: "g"(send_len) : "%ebx");
  __asm volatile("movl %0, %%eax" :: "g"(recv_len) : "%eax");

  __asm volatile("int %0" :: "Nd"(SYSCALL_SEND_MSG_VECTOR));

  /* TODO */
  return false;
}

int ipc_recv(void *recv_buf, size_t recv_len)
{
  int sender;

  /* call the kernel, and pass him the message */
  __asm volatile("movl %0, %%edi" :: "g"(recv_buf) : "%edi");
  __asm volatile("movl %0, %%eax" :: "g"(recv_len) : "%eax");

  __asm volatile("int %0" :: "Nd"(SYSCALL_RECV_MSG_VECTOR));

  /* the kernel returns the original sender's id through ecx */
  __asm volatile("movl %%ecx, %0" : "=g"(sender));

  return sender;
}

bool ipc_reply(int sender, void *send_buf, size_t send_len)
{
  /* call the kernel, and pass him the message */
  __asm volatile("movl %0, %%esi" :: "g"(send_buf) : "%esi");
  __asm volatile("movl %0, %%ebx" :: "g"(send_len) : "%ebx");
  __asm volatile("movl %0, %%ecx" :: "g"(sender) : "%ecx");

  __asm volatile("int %0" :: "Nd"(SYSCALL_RPLY_MSG_VECTOR));

  /* TODO */
  return false;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

