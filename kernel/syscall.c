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
  /* TODO */
  return regs;
}

struct intregs *syscall_recv_msg(struct intregs *regs)
{
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

