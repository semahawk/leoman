/*
 *
 * fairy.c
 *
 * Created at:  04 Aug 2016 21:26:13 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/fairy.h>
#include <kernel/proc.h>
#include <kernel/vm.h>

/*
 * The one that grants nice things
 */
void proc_fairy(void)
{
  struct msg *msg, response;
  struct proc *sender;

  while (1){
    if ((msg = proc_pop_msg(current_proc->pid))){
      /* not entirely sure that's required */
      /* but better safe than debugging */
      proc_disable_scheduling();

      switch (msg->type){
        case FAIRY_REQUEST_VIDEO_MEMORY:
          sender = proc_find_by_pid(msg->sender);

          /* FIXME don't swap those page directories like that */
          /* it's probably inefficent as hell */
          set_cr3((uint32_t)sender->pdir);
          map_page((void *)0xb8000, (void *)0xbabe0000, PTE_W | PTE_U);
          set_cr3((uint32_t)current_proc->pdir);

          response.sender = current_proc->pid;
          response.receiver = msg->sender;
          response.type = FAIRY_VIDEO_MEMORY_ADDRESS;
          response.data = 0xbabe0000;

          proc_push_msg(msg->sender, &response);

          break;
        default:
          response.sender = current_proc->pid;
          response.receiver = msg->sender;
          response.type = FAIRY_REQUEST_FAILED;
          response.data = 0x0badc0de;

          proc_push_msg(msg->sender, &response);

          break;
      }

      proc_enable_scheduling();
    }
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

