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
#include <kernel/x86.h>

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
        proc_disable_scheduling();
        /*vga_printf("[kernel] process %s wants to have interrupt %d forwarded\n",*/
            /*sender->name, msg.data.interrupt.which);*/
        int which = msg.data.interrupt.which;
        uint32_t virt = (uint32_t)msg.data.interrupt.handler;
        uint32_t save_curr_cr3 = get_cr3();
        set_cr3((uint32_t)sender->pdir);
        uint32_t phys = (uint32_t)(uintptr_t)vm_get_phys_mapping((void *)virt);
        set_cr3(save_curr_cr3);
        /*vga_printf("[kernel] .. and handled with 0x%x\n", phys);*/
        /*vga_printf("[kernel] handler at virt 0x%x, phys 0x%x\n", virt, phys);*/
        /* TODO sanity/security checking of `which` */
        irq_install_handler(which, (irq_handler_t)virt, (uint32_t)sender->pdir, sender_pid);
        response = 0x33cafe33;
        proc_enable_scheduling();
        break;
      case MSG_MAP_MEMORY:
        proc_disable_scheduling();
        /*vga_printf("[kernel] process %s wants to get %d bytes at 0x%x mapped\n", sender->name,*/
            /*msg.data.map_memory.length, msg.data.map_memory.paddr);*/
        /* TODO: perform some permission-checking when we have users */
        set_cr3((uint32_t)sender->pdir);
        map_pages((void *)msg.data.map_memory.paddr, (void *)0x10000000, PTE_U, msg.data.map_memory.length);
        set_cr3((uint32_t)current_proc->pdir);

        response = 0x10000000;

        proc_enable_scheduling();
        break;
      case MSG_PORT_IN_BYTE:
        response = inb(msg.data.port.which);
        break;
      case MSG_PORT_OUT_BYTE:
        outb(msg.data.port.which, (uint8_t)(msg.data.port.data & 0xff));
        response = 1;
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

