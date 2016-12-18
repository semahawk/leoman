/*
 *
 * msg_dispatcher.c
 *
 * Created at:  26 Nov 2016 21:34:02 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/proc.h>
#include <kernel/vga.h>
#include <kernel/vm.h>

#include <ipc.h>
#include <msg/kernel.h>

/*
 * This is the 'kernel process'
 */
void msg_dispatcher(void)
{
  struct msg_kernel msg;
  int response;
  int sender_pid;
  struct proc *sender;

  while (1){
    sender_pid = ipc_recv(&msg, sizeof msg);
    response = 1;

    /* TODO: some sanity checking? */
    sender = proc_find_by_pid(sender_pid);

    switch (msg.type){
      case MSG_REQUEST_INTERRUPT_FORWARDING:
        vga_printf("[kernel] process %s wants to have interrupt %d forwarded\n",
            sender->name, msg.data.interrupt.which);
        break;
      case MSG_MAP_MEMORY:
        proc_disable_scheduling();
        vga_printf("[kernel] process %s wants to get %d bytes at 0x%x mapped\n", sender->name,
            msg.data.map_memory.length, msg.data.map_memory.paddr);
        /* TODO: perform some permission-checking when we have users */
        set_cr3((uint32_t)sender->pdir);
        map_pages((void *)msg.data.map_memory.paddr, (void *)0x10000000, PTE_U, msg.data.map_memory.length);
        set_cr3((uint32_t)current_proc->pdir);

        response = 0x10000000;

        proc_enable_scheduling();
        break;
      default:
        response = 0;
        break;
    }

    ipc_reply(sender->pid, &response, sizeof response);
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

