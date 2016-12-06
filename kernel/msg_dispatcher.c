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

#include <ipc.h>
#include <msg/interrupt.h>

/*
 * This is the 'kernel process'
 */
void msg_dispatcher(void)
{
  /* TODO: make it more generic - not just interrupts */
  struct msg_interrupt msg;
  int response;
  int sender;

  while (1){
    sender = ipc_recv(&msg, sizeof msg);
    response = 1;

    switch (msg.type){
      case MSG_INTERRUPT_REQUEST_FORWARDING:
        vga_printf("[kernel] process %s (%x) wants to have interrupt %d forwarded\n", proc_find_by_pid(sender)->name, sender, msg.which);
        break;
      default:
        response = 0;
        break;
    }

    ipc_reply(sender, &response, sizeof response);
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

