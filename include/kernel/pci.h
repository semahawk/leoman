/*
 *
 * pci.h
 *
 * Created at:  04 Jul 2017 19:42:57 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef KERNEL_PCI_H
#define KERNEL_PCI_H

#define PCI_COMMAND_PORT 0xcf8
#define PCI_DATA_PORT    0xcfc

#define PCI_REG_VENDOR_ID 0x00
#define PCI_REG_DEVICE_ID 0x02

int pci_init(void);

#endif /* !KERNEL_PCI_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

