/*
 *
 * pci.c
 *
 * Created at:  04 Jul 2017 19:42:14 +0200 (CEST)
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <kernel/common.h>
#include <kernel/pci.h>
#include <kernel/print.h>
#include <kernel/x86.h>

uint16_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t fun, uint8_t reg)
{
  uint16_t result = 0x0;
  uint32_t id =
    (1 << 31)   |
    (bus << 16) |
    (dev << 11) |
    (fun << 8)  |
    (reg & 0xfc);

  outl(PCI_COMMAND_PORT, id);

  result = inl(PCI_DATA_PORT);
  result >>= (reg & 2) * 8;
  result &= 0xffff;

  return result;
}

int pci_init(void)
{
  kprintf("[pci] enumerating devices:\n");

  for (unsigned bus = 0; bus < 8; bus++){
    for (unsigned device = 0; device < 32; device++){
      for (unsigned function = 0; function < 8; function++){
        uint16_t vendor_id = pci_read_config(bus, device, function, PCI_REG_VENDOR_ID);
        uint16_t device_id = pci_read_config(bus, device, function, PCI_REG_DEVICE_ID);

        if (vendor_id == 0x0000 || vendor_id == 0xffff)
          continue;

        kprintf("[pci] -- %x:%x.%x %x:%x\n", bus, device, function, vendor_id, device_id);
      }
    }
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

