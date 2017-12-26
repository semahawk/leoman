/*
 *
 * smp.c
 *
 * Created at:  17 Dec 2017 20:41:13 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/common.h>
#include <kernel/mmio.h>
#include <kernel/smp.h>
#include <kernel/print.h>
#include <kernel/vm.h>
#include <kernel/x86.h>

volatile uint32_t smp_initialized_cores_num = 0x1;

static struct mp_float_table *mp_float_table = NULL;
static struct mp_conf_table *mp_conf_table = NULL;

/* search for the MP table, starting at <addr> but not going further than <len> bytes */
/* return 1 if found and 0 if table was not found */
static int search_for_mp_table(void *addr, size_t len)
{
    char *p = addr;

    for (int i = 0; i < len; i++, p++){
        if (*p != '_')
            continue;

        if (*(p + 1) == 'M' && *(p + 2) == 'P' && *(p + 3) == '_'){
            mp_float_table = (void *)p;
            return 1;
        }
    }

    return 0;
}

void smp_init(void)
{
    kprintf("[smp] searching for the MP table... ");

    if ((0 != search_for_mp_table((void *)0x9fc00, KiB(1))) ||
        (0 != search_for_mp_table((void *)0xe0000, KiB(128)))){
        kprintf("found at 0x%x\n", mp_float_table);
    } else {
        kprintf("not found\n");
        kprintf("[smp] cannot boot additional cores, as the system is not SMP-compliant!\n");
        return;
    }

    if (mp_float_table->phys_addr == 0x0){
        kprintf("[smp] MP configuration table not found!\n");
        return;
    }

    mp_conf_table = (void *)mp_float_table->phys_addr;

    kprintf("[smp] -- length: %d, revision: 1.%d, phys addr: 0x%x\n",
        mp_float_table->length, mp_float_table->spec_rev, mp_float_table->phys_addr);

    /* identity-map the local APIC so we can access it's registers */
    map_pages((void *)mp_conf_table->local_apic_addr,
        (void *)mp_conf_table->local_apic_addr, PTE_C | PTE_T, MiB(1) - 1);

    vm_change_pages_attrs((void *)KERNEL_TRAMPOLINE_LOAD_ADDR,
        PTE_P | PTE_C | PTE_T | PTE_W, KERNEL_TRAMPOLINE_MAX_SIZE);

    /* copy the trampoline to the destination */
    memcpy((void *)KERNEL_TRAMPOLINE_LOAD_ADDR,
        (void *)((uint32_t)&_binary_trampoline_bin_start),
        (size_t)((uint32_t)&_binary_trampoline_bin_size));

    /* NOTE: the addresses here _must_ match the variables in trampoline_bin.asm! */
    /* send the kernel's page directory address to the AP so it can enable paging */
    mmio_write32((void *)KERNEL_TRAMPOLINE_VARS_ADDR + 0, get_cr3());
    mmio_write32((void *)KERNEL_TRAMPOLINE_VARS_ADDR + 4, 0x0);
    mmio_write32((void *)KERNEL_TRAMPOLINE_VARS_ADDR + 8, (uint32_t)kmain_secondary_cores);

    void *entry = &mp_conf_table->_entries[0];

    kprintf("[smp] detecting hardware...\n");

    for (unsigned i = 0; i < mp_conf_table->entry_count; i++){
        uint8_t entry_type = *(uint8_t *)entry;

        switch (entry_type){
            case 0: {
                struct mp_conf_table_cpu_entry *cpu_entry = entry;

                // kprintf("[smp] -- cpu#%x\n", cpu_entry->local_apic_id);

                /* don't initialize the BSP (bootstrap processor) */
                if (cpu_entry->local_apic_id != 0x0){
                  /* FIXME find a better way of figuring out the BSP */
                  smp_init_core(cpu_entry->local_apic_id);
                }

                entry += sizeof(*cpu_entry);
                break;
            }
            case 1: {
                struct mp_conf_table_bus_entry *bus_entry = entry;

                // kprintf("[smp] -- bus#%c%c%c\n",
                    // bus_entry->type_string[0],
                    // bus_entry->type_string[1],
                    // bus_entry->type_string[2]);

                entry += sizeof(*bus_entry);
                break;
            }
            case 2: {
                struct mp_conf_table_io_apic_entry *io_apic_entry = entry;

                entry += sizeof(*io_apic_entry);
                break;
            }
            case 3: {
                struct mp_conf_table_int_assign_entry *int_assign_entry = entry;

                entry += sizeof(*int_assign_entry);
                break;
            }
            case 4: {
                struct mp_conf_table_local_int_assign_entry *local_int_assign_entry = entry;

                entry += sizeof(*local_int_assign_entry);
                break;
            }
            default:
                kprintf("[smp] unknown entry type: %d\n", entry_type);
                return;
        }
    }

    kprintf("[smp] woke %d core(s) up\n", smp_initialized_cores_num);
}

int smp_init_core(uint8_t core_id)
{
    volatile uint32_t saved_inited_cores_num = smp_initialized_cores_num;
    uint32_t tries = 0;

    if ((uint32_t)&_binary_trampoline_bin_size > KERNEL_TRAMPOLINE_MAX_SIZE){
        kprintf("[smp] error: trampoline is bigger than 4KiB!\n");
        return 1;
    }

    if (0 != smp_send_init_ipi(core_id)){
        kprintf("[smp] couldn't send INIT IPI to cpu#0x%x\n", core_id);
        return 1;
    }

    /* wait for a bit */
    for (volatile int i = 0; i < 100; i++);

    /* send the startup IPI so the AP actually starts executing code
     * in the trampoline */
    if (0 != smp_send_startup_ipi(core_id, KERNEL_TRAMPOLINE_LOAD_ADDR / PAGE_SIZE)){
        kprintf("[smp] couldn't send STARTUP IPI to cpu#0x%x\n", core_id);
        return 1;
    }

    /* wait for a bit to give the AP time to increase the initialized
     * cores counter */
    do {
        uint32_t current_inited_cores_num = smp_initialized_cores_num;

        /* check if the AP actually booted */
        /* TODO poll for this */
        if (current_inited_cores_num > saved_inited_cores_num + 1){
            kprintf("[smp] more cores woke-up than intended!\n");
            return 1;
        } else if (current_inited_cores_num == saved_inited_cores_num + 1)
            break;
    } while (tries++ < 10000000);

    return 0;
}

int smp_send_init_ipi(uint8_t core_id)
{
    void *apic = (void *)((uintptr_t)mp_conf_table->local_apic_addr);

    /* make sure we don't have any pending IPIs */
    while (mmio_read32(apic + 0x30) & BIT(12));

    /* set the delivery target (high doubleword of ICR) */
    mmio_write32(apic + 0x310, ((uint32_t)core_id) << 24);
    /* level triggered, INIT */
    mmio_write32(apic + 0x300, 0x4500);

    return 0;
}

int smp_send_startup_ipi(uint8_t core_id, uint8_t code_page)
{
    void *apic = (void *)((uintptr_t)mp_conf_table->local_apic_addr);

    /* make sure we don't have any pending IPIs */
    while (mmio_read32(apic + 0x30) & BIT(12));

    /* set the delivery target (high doubleword of ICR) */
    mmio_write32(apic + 0x310, ((uint32_t)core_id) << 24);
    /* level triggered, STARTUP, page where the code lies */
    mmio_write32(apic + 0x300, 0x4600 | code_page);

    return 0;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
