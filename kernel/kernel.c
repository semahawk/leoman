/*
 *
 * kernel.c
 *
 * Created at:  Fri 28 Mar 13:20:40 2014 13:20:40
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "elf.h"
#include "vm.h"
#include "pm.h"
#include "gdt.h"
#include "idt.h"
#include "kbd.h"
#include "vga.h"
#include "proc.h"
#include "sar.h"
#include "syscall.h"
#include "timer.h"
#include "tss.h"
#include "x86.h"

#ifndef __i386__
#error "the only supported architecture is i386"
#endif

size_t strlen(const char *s)
{
  size_t ret = 0;

  while (s[ret++] != '\0')
    ;

  return ret;
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1 == *s2++)
    if (*s1++ == '\0')
      return 0;

  return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

void *memset(void *dst, int ch, size_t len)
{
  while (len-- != 0)
    *(uint8_t *)dst++ = (unsigned char)ch;

  return dst;
}

void *memcpy(void *dst, void *src, size_t len)
{
  void *ret = dst;

  while (len-- != 0)
    *(uint8_t *)dst++ = *(uint8_t *)src++;

  return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

static void adjust_the_memory_map(struct kern_bootinfo *bootinfo)
{
  /* {{{ */
  /* add a (reserved) memory entry for kernel's guts */
  struct memory_map_entry kernentry = {
    .base_low  = &kernel_phys,
    .base_high = 0x0,
    /* make room for the paging stuff */
    .len_low   = (uint32_t)&kernel_size + KiB(4) + MiB(4),
    .len_high  = 0x0,
    .type      = 2,
    .acpi_ext  = 0
  };

  /* the 61 is intentional (64 - 3 since at most 3 entries may be added) */
  for (int i = 0; i < 61; i++){
    struct memory_map_entry e = bootinfo->memory_map[i];

    /* see if the kernel's is going to fit in that (available) entry */
    if (e.type == 1){
      if (kernentry.base_low >= e.base_low){
        if (e.len_low >= kernentry.len_low + (kernentry.base_low - e.base_low)){
          struct memory_map_entry pre = e;
          struct memory_map_entry pre_align = e;

          pre.len_low = kernentry.base_low - e.base_low;
          pre.len_high = 0x0;

          pre_align.base_low = pre.base_low + pre.len_low;
          pre_align.base_high = 0x0;
          pre_align.len_low = e.len_low - pre.len_low;
          pre_align.len_high = 0x0;

          /* we might need to create an available entry preceding the kernel's */
          if (kernentry.base_low - e.base_low > 0){
            int k;

            for (k = 62; k > i; k--)
              bootinfo->memory_map[k + 1] = bootinfo->memory_map[k];

            bootinfo->memory_map[k] = pre;
            bootinfo->memory_map[k + 1] = pre_align;
            i++;
          }

          bootinfo->memory_map[i] = kernentry;

          /* we might need to create an (available) entry following the kernel's */
          if (e.len_low - kernentry.len_low - (kernentry.base_low - e.base_low) > 0){
            struct memory_map_entry post = {
              .base_low  = kernentry.base_low + kernentry.len_low,
              .base_high = 0x0,
              .len_low   = (e.len_low + e.base_low) - (kernentry.base_low + kernentry.len_low),
              .len_high  = 0x0,
              .type      = 1,
              .acpi_ext  = 0
            };

            int k;

            for (k = 62; k > i; k--)
              bootinfo->memory_map[k + 1] = bootinfo->memory_map[k];

            bootinfo->memory_map[k + 1] = post;
          }
        }
      }
    }
  }

  /* merge any adjacent entries of the same type */
  for (int i = 0; i < 64; i++){
    struct memory_map_entry *e = &bootinfo->memory_map[i];

    if ((e->len_low | e->len_high) == 0) continue;

    if ((e + 1)->type == e->type){
      int j = 1;

      while ((e + j)->type == e->type){
        e->len_low += (e + j)->len_low;
        j++;
      }

      j--;

      for (int k = i + 1; k < 64 - i - 1; k++){
        struct memory_map_entry *p = &bootinfo->memory_map[k];

        *p = *(p + j);
      }
    }
  }

  /* calculate the (total) available memory */
  for (int i = 0; i < 64; i++){
    struct memory_map_entry *e = &bootinfo->memory_map[i];

    if (e->type == 1 || e->type == 3)
      bootinfo->mem_avail += e->len_low;
  }
  /* }}} */
}

void kmain(struct kern_bootinfo *bootinfo)
{
  adjust_the_memory_map(bootinfo);
  /* set up the printing utilities */
  vga_init();
  /* set up the virtual memory thingies */
  void *vm = vm_init(bootinfo);
  /* set up the physical memory thingies */
  void *pm = pm_init(bootinfo);
  /* set up the segments, kernel code and data, &c */
  gdt_init();
  /* install the IDT (ISRs and IRQs) */
  idt_install();
  /* install the keyboard */
  kbd_install();
  /* install the timer */
  timer_install();
  /* initiate system calls */
  syscall_install();
  /* part one of processes init */
  proc_earlyinit();

  sti();

  /* map the initrd file */
  map_pages(bootinfo->initrd_addr, p2v((uint32_t)bootinfo->initrd_addr), PTE_W, bootinfo->initrd_size);
  /* update the initrd's address to be the virtual one */
  bootinfo->initrd_addr = p2v((uint32_t)bootinfo->initrd_addr);

  vga_printf("\n Leoman\n\n");
  vga_puts(" Tha mo bhata-foluaimein loma-lan easgannan\n");
  vga_puts(" ------------------------------------------\n\n");
  vga_printf(" available memory detected: 0x%x (%d MiB)\n\n", bootinfo->mem_avail, bootinfo->mem_avail / 1024 / 1024);
  vga_printf(" kernel's physical address: 0x%x\n", &kernel_phys);
  vga_printf(" kernel's  virtual address: 0x%x\n", &kernel_start);
  vga_printf(" kernel's size:             0x%x\n", &kernel_size);
  vga_printf(" virtual memory:            0x%x\n", vm);
  vga_printf(" physical memory:           0x%x\n", pm);
  vga_printf(" initrd loaded to:          0x%x\n", bootinfo->initrd_addr);
  vga_printf(" initrd's size:             0x%x\n", bootinfo->initrd_size);

  vga_printf("\n");

  void *initrdtestfile = sar_get_contents(bootinfo->initrd_addr, "initrd.o");

  if (initrdtestfile)
    elf_execute(initrdtestfile);

  /* processes will start running right now */
  proc_schedule_without_irq();

  /* should never get here */
  for (;;);
}

#ifdef __cplusplus
}
#endif

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

