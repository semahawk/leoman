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

struct intregs *syscall_handler(struct intregs *regs)
{
  syscall_t syscall = regs->eax;

  switch (syscall){
    case SYS_write: {
      int fd = regs->ebx;
      int len = regs->edx;
      char *msg = (char *)regs->ecx;

      while (len-- > 0)
        vga_putch(*msg++);

      /* return the length */
      regs->eax = regs->edx;
      break;
    }
    case SYS_getpid:
      regs->eax = current_proc->pid;
      break;
    default:
      regs->eax = 0x0badc0de;
      break;
  }

  return regs;
}

void syscall_install(void)
{
  idt_set_gate(0x80, int128, 8, 0xee);
  int_install_handler(0x80, syscall_handler);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

