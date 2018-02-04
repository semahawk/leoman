/*
 *
 * smp.h
 *
 * Created at:  17 Dec 2017 20:46:11 +0100 (CET)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_SMP_H
#define KERNEL_SMP_H

#include <kernel/common.h>

#define KERNEL_TRAMPOLINE_LOAD_ADDR 0x8000
#define KERNEL_TRAMPOLINE_MAX_SIZE 0x1000
#define KERNEL_TRAMPOLINE_STACK_SIZE 256
#define KERNEL_TRAMPOLINE_STACK_SIZE_LOG2 8
#define KERNEL_TRAMPOLINE_STACKS_START_ADDR \
    (KERNEL_TRAMPOLINE_LOAD_ADDR + KERNEL_TRAMPOLINE_MAX_SIZE)
#define KERNEL_TRAMPOLINE_VARS_OFFSET 16
#define KERNEL_TRAMPOLINE_VARS_ADDR \
    (KERNEL_TRAMPOLINE_LOAD_ADDR + KERNEL_TRAMPOLINE_VARS_OFFSET)

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__
/* provided by the linker */
extern uint32_t _binary_trampoline_bin_start;
extern uint32_t _binary_trampoline_bin_end;
extern uint32_t _binary_trampoline_bin_size;
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__
extern volatile uint32_t smp_initialized_cores_num;
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__
void smp_init(void);
int smp_init_core(uint8_t core_id);
int smp_send_init_ipi(uint8_t core_id);
int smp_send_startup_ipi(uint8_t core_id, uint8_t code_page);
void kmain_secondary_cores(uint32_t magic);
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__
/* The mp_*table* thingies are from the MultiProcessor specification.
 * Reference: https://pdos.csail.mit.edu/6.828/2011/readings/ia32/MPspec.pdf */
struct mp_float_table {
    uint32_t magic;
    uint32_t phys_addr;
    uint8_t length;
    uint8_t spec_rev;
    uint8_t checksum;
    uint8_t mp_features[5];
} __PACKED;

struct mp_conf_table {
    uint8_t magic[4];
    uint16_t base_length;
    uint8_t spec_rev;
    uint8_t checksum;
    uint8_t oem_id[4];
    uint8_t prod_id[16];
    uint32_t oem_table_ptr;
    uint16_t oem_table_size;
    uint16_t entry_count;
    uint32_t local_apic_addr;
    uint16_t ext_table_length;
    uint8_t ext_table_checksum;
    uint8_t _resvd0;
    uint32_t _entries[0];
} __PACKED;

struct mp_conf_table_cpu_entry {
    uint8_t type;
    uint8_t local_apic_id;
    uint8_t local_apic_ver;
    uint8_t cpu_flags;
    uint32_t cpu_signature;
    uint32_t feature_flags;
    uint32_t _resvd0;
    uint32_t _resvd1;
} __PACKED;

struct mp_conf_table_bus_entry {
    uint8_t type;
    uint8_t id;
    uint8_t type_string[6];
} __PACKED;

struct mp_conf_table_io_apic_entry {
    uint8_t type;
    uint8_t apic_id;
    uint8_t apic_ver;
    uint8_t apic_flags;
    uint32_t apic_addr;
} __PACKED;

struct mp_conf_table_int_assign_entry {
    uint8_t type;
    uint8_t int_type;
    uint16_t int_flag;
    uint8_t source_bus_id;
    uint8_t source_bus_irq;
    uint8_t dest_apic_id;
    uint8_t dest_apic_intin;
} __PACKED;

struct mp_conf_table_local_int_assign_entry {
    uint8_t type;
    uint8_t int_type;
    uint16_t local_int_flag;
    uint8_t source_bus_id;
    uint8_t source_bus_irq;
    uint8_t dest_apic_id;
    uint8_t dest_apic_intin;
} __PACKED;
#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__
#define DECLARE_LOCK(name) volatile int name ## Locked
#define LOCK(name) \
	while (!__sync_bool_compare_and_swap(& name ## Locked, 0, 1)); \
	__sync_synchronize();
#define UNLOCK(name) \
	__sync_synchronize(); \
	name ## Locked = 0;
#endif /* !__ASSEMBLY__ */

#endif /* !KERNEL_SMP_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */
