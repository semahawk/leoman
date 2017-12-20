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

#include <kernel/common.h>
#include <kernel/elf.h>
#include <kernel/vm.h>
#include <kernel/pm.h>
#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/idt.h>
#include <kernel/kbd.h>
#include <kernel/vga.h>
#include <kernel/proc.h>
#include <kernel/sar.h>
#include <kernel/syscall.h>
#include <kernel/smp.h>
#include <kernel/timer.h>
#include <kernel/tss.h>
#include <kernel/x86.h>

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

/* processes that are considered to be essential and have to be found in the
 * initrd */
static const char *const essential_initrd_processes[] = {
  /* the order here is the order those processes will be loaded in */
  "screen", "kbd", "shell", "angle",
  NULL
};

void kmain(struct kern_bootinfo *bootinfo)
{
  /* whether all of the required process were loaded from the initrd */
  bool all_processes_loaded = true;

  cli();

  adjust_the_memory_map(bootinfo);
  /* set up the printing utilities */
  vga_init();

  vga_printf("[kern] available memory detected %d MiB\n", bootinfo->mem_avail / 1024 / 1024);

  /* set up the segments, kernel code and data, &c */
  gdt_init();
  /* install the IDT (ISRs and IRQs) */
  idt_install();
  /* set up the physical memory manager thingies */
  pm_init(bootinfo);
  /* set up the virtual memory manager thingies */
  vm_init(bootinfo);
  /* install the keyboard */
  kbd_install();
  /* install the timer */
  timer_install();
  /* initiate system calls */
  syscall_install();
  /* part one of processes init */
  proc_earlyinit();
  /* initialize and enumerate PCI devices */
  pci_init();
  /* initialize the kernel heap */
  heap_init();
  /* initialize and start-up the APs (Aplication Processors) */
  smp_init();

  /* load the required processes off of the initrd */
  /* hang if any of those was not found */
  for (const char **initrd_proc_name = (const char **)essential_initrd_processes; *initrd_proc_name != NULL; initrd_proc_name++){
    struct sar_file *initrd_proc_executable;

    if ((initrd_proc_executable = sar_lookup(bootinfo->initrd_addr, *initrd_proc_name))){
      vga_printf("[initrd] loading process %s\n", *initrd_proc_name);
      proc_new_from_memory(*initrd_proc_name, false, true,
          (void *)bootinfo->initrd_addr + initrd_proc_executable->offset, initrd_proc_executable->size);
    } else {
      vga_printf("error: process '%s' was not found in the initrd!\n", *initrd_proc_name);
      all_processes_loaded = false;
    }
  }

  vga_printf("[debug] halting...\n");
  halt();

  if (!all_processes_loaded)
    for (;;) halt();

  /* processes will start running right now */
  proc_kickoff_first_process();

  vga_printf("putting kmain into an endless loop (if you can see me we have a bug).\n");
  /* should never get here */
  for (;;)
    halt();
}

void kmain_secondary_cores(uint32_t core_id)
{
  vga_printf("cpu#%x says hi (and halts)!\n", core_id);

  for (;;)
    halt();
}

#ifdef __cplusplus
}
#endif

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

