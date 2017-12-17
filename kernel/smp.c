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
#include <kernel/smp.h>
#include <kernel/vga.h>

static struct mp_float_table *mp_float_table = NULL;
static struct mp_conf_table *mp_conf_table = NULL;
static unsigned core_num = 0;

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
    vga_printf("[smp] searching for the MP table... ");

    if ((0 != search_for_mp_table((void *)0x9fc00, KiB(1))) ||
        (0 != search_for_mp_table((void *)0xe0000, KiB(128)))){
        vga_printf("found at 0x%x\n", mp_float_table);
    } else {
        vga_printf("not found\n");
        vga_printf("[smp] cannot boot additional cores, as the system is not SMP-compliant!\n");
        return;
    }

    if (mp_float_table->phys_addr == 0x0){
        vga_printf("[smp] MP configuration table not found!\n");
        return;
    }

    mp_conf_table = (void *)mp_float_table->phys_addr;

    vga_printf("[smp] -- length: %d, revision: 1.%d, phys addr: 0x%x\n",
        mp_float_table->length, mp_float_table->spec_rev, mp_float_table->phys_addr);

    void *entry = &mp_conf_table->_entries[0];

    vga_printf("[smp] detecting hardware...\n", entry);

    for (unsigned i = 0; i < mp_conf_table->entry_count; i++){
        uint8_t entry_type = *(uint8_t *)entry;

        switch (entry_type){
            case 0: {
                struct mp_conf_table_cpu_entry *cpu_entry = entry;

                vga_printf("[smp] -- cpu#%x\n", cpu_entry->local_apic_id);

                entry += sizeof(*cpu_entry);
                core_num++;
                break;
            }
            case 1: {
                struct mp_conf_table_bus_entry *bus_entry = entry;

                vga_printf("[smp] -- bus#%c%c%c\n",
                    bus_entry->type_string[0],
                    bus_entry->type_string[1],
                    bus_entry->type_string[2]);

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
                vga_printf("[smp] unknown entry type: %d\n", entry_type);
                return;
        }
    }

    vga_printf("[smp] detected %d core(s)\n", core_num);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
