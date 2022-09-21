#include "pc.h"
#include "cpu/cpu.h"
#include "devices.h"
#include "io.h"
#include "state.h"
#include "util.h"
#include <cstdint>
#include <stdlib.h>
#include <string.h>

#define PCI_LOG(x, ...)   NOP()
#define PCI_FATAL(x, ...) FATAL("PCI", x, ##__VA_ARGS__)
#define ROM_READ          1
#define ROM_WRITE         2

#define DEV_82371SB_ID 1

static uint8_t *ram;

static const uint8_t configuration_space_82441fx[128] = {
    134, 128, 55, 18, 6, 0, 128, 2, 2, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,  0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0,  0,
    0,   0,   0,  0,  0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 128, 0, 0, 0, 1, 16, 0,
    0,   0,   0,  0,  0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   2, 0, 0, 0, 0,  0};

static const uint8_t configuration_space_82371sb_isa[192] = {
    134, 128, 0, 112, 7,   0,   0,   2, 0, 0, 1, 6, 0, 0,  128, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0,
    0,   0,   0, 0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0,  0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0,
    0,   0,   0, 0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0,  77,  0, 3, 0, 0, 0,   0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0,
    0,   0,   0, 128, 128, 128, 128, 0, 0, 0, 0, 0, 2, 0,  0,   0, 0, 0, 0, 128, 0, 0, 0, 0, 0, 12, 12, 2, 0, 0, 0,
    0,   0,   0, 0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0,  0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0,
    0,   0,   0, 0,   0,   8,   0,   0, 0, 0, 0, 0, 0, 15, 0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,  0,  0, 0};

static const uint8_t configuration_space_82371sb_ide[64] = {
    134, 128, 16, 112, 1, 0, 128, 2, 0, 128, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,  0,   1, 0, 0,   0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


static struct pci_state
{
    uint32_t          configuration_address_register;
    int               configuration_cycle;
    uint8_t          *configuration_address_spaces[256];
    uint8_t           rom_area_memory_mapping[16];
    pci_conf_write_cb configuration_modification[256];
} pci;


static void pci_mark_rom_area(uint32_t addr, int map)
{
    pci.rom_area_memory_mapping[(addr - 0xC0000) >> 14] = map;
}
static void pci_write(uint32_t addr, uint32_t data)
{
    int offset = addr & 3;
    switch (addr & ~3) {
        case 0xCF8:
            offset *= 8;
            pci.configuration_address_register &= ~(0xFF << offset);
            pci.configuration_address_register |= data << offset;
            pci.configuration_address_register &= ~0x7F000003;
            pci.configuration_cycle = pci.configuration_address_register >> 31;
            break;
        case 0xCFC:
            if (pci.configuration_cycle) {
                int bus                 = pci.configuration_address_register >> 16 & 0xFF;
                int device_and_function = pci.configuration_address_register >> 8 & 0xFF;
                int offset              = (pci.configuration_address_register & 0xFC) | (addr & 3);
                if (bus != 0) {
                    return;
                }
                if (!pci.configuration_modification[device_and_function])
                    return;
                uint8_t *arr = pci.configuration_address_spaces[device_and_function];
                if (!pci.configuration_modification[device_and_function](arr, offset, data))
                    arr[offset] = data;
            }
            break;
        default:
            PCI_FATAL("Write to unknown register - %x\n", addr);
    }
}
static uint32_t pci_read(uint32_t addr)
{
    int      offset = addr & 3;
    uint32_t retval = -1;
    switch (addr & ~3) {
        case 0xCF8:
            return pci.configuration_address_register >> (offset * 8) & 0xFF;
        case 0xCFC: {
            if (pci.configuration_cycle) {
                int bus                 = pci.configuration_address_register >> 16 & 0xFF;
                int device_and_function = pci.configuration_address_register >> 8 & 0xFF;
                int offset              = pci.configuration_address_register & 0xFC;
                if (bus != 0) {
                    return -1;
                }
                uint8_t *ptr = pci.configuration_address_spaces[device_and_function];
                if (ptr)
                    retval = ptr[offset | (addr & 3)];
                else
                    retval = -1;
            }
            return retval;
        }
        default:
            PCI_FATAL("Read from unknown register - %x\n", addr);
    }
}
static uint32_t pci_read16(uint32_t addr)
{
    uint16_t result = pci_read(addr);
    result |= pci_read(addr + 1) << 8;
    return result;
}
static uint32_t pci_read32(uint32_t addr)
{
    uint32_t result = pci_read(addr);
    result |= pci_read(addr + 1) << 8;
    result |= pci_read(addr + 2) << 16;
    result |= pci_read(addr + 3) << 24;
    return result;
}
static void pci_write16(uint32_t addr, uint32_t data)
{
    pci_write(addr, data & 0xFF);
    pci_write(addr + 1, data >> 8 & 0xFF);
}
static void pci_write32(uint32_t addr, uint32_t data)
{
    pci_write(addr, data & 0xFF);
    pci_write(addr + 1, data >> 8 & 0xFF);
    pci_write(addr + 2, data >> 16 & 0xFF);
    pci_write(addr + 3, data >> 24 & 0xFF);
}
void *pci_create_device(uint32_t bus, uint32_t device, uint32_t function, pci_conf_write_cb cb)
{
    if (bus != 0)
        PCI_FATAL("Unsupported bus id=%d\n", bus);
    if (device > 31)
        PCI_FATAL("Unsupported device id=%d\n", device);
    if (function > 7)
        PCI_FATAL("Unsupported function id=%d\n", device);
    pci.configuration_modification[device << 3 | function]          = cb;
    return pci.configuration_address_spaces[device << 3 | function] = (uint8_t *)calloc(1, 256);
}
void pci_copy_default_configuration(void *confptr, void *area, int size)
{
    if (size > 256)
        size = 256;
    memcpy(confptr, area, size);
}
void *pci_get_configuration_ptr(uint32_t bus, uint32_t device, uint32_t function)
{
    if (bus != 0)
        PCI_FATAL("Unsupported bus id=%d\n", bus);
    if (device > 31)
        PCI_FATAL("Unsupported device id=%d\n", device);
    if (function > 7)
        PCI_FATAL("Unsupported function id=%d\n", device);
    return pci.configuration_address_spaces[device << 3 | function];
}
static void pci_set_rw(uint32_t addr, int access_bits)
{
    pci.rom_area_memory_mapping[(addr - 0xC0000) >> 14] = access_bits;
}
static int pci_82441fx_write(uint8_t *ptr, uint8_t addr, uint8_t data)
{
    uint8_t res    = data;
    int     retval = 0;
    switch (addr) {
        case 0 ... 3:
        case 6:
        case 8 ... 12:
        case 0x10 ... 0x4F:
        case 0x69 ... 0x6F:
        case 0x73 ... 0x8F:
        case 0x92:
        case 0x94 ... 0xFF:
            retval = 1;
            res    = ptr[addr];
            break;
        case 4:
            res    = (data & 0x40) | 6;
            retval = 1;
            break;
        case 5:
            res &= 1;
            retval = 1;
            break;
        case 7:
            res    = ptr[addr] & ~(data & 0xF9);
            retval = 1;
            break;
        case 0x0D:
            res &= 0xF8;
            retval = 1;
            break;
        case 0x50:
            res &= 0x70;
            retval = 1;
            break;
        case 0x51:
            res    = (data & 0x80) | 1;
            retval = 1;
            break;
        case 0x59 ... 0x5F:
            switch (addr - 0x59) {
                case 0:
                    data >>= 4;
                    pci_set_rw(0xF0000, data);
                    pci_set_rw(0xF4000, data);
                    pci_set_rw(0xF8000, data);
                    pci_set_rw(0xFC000, data);
                    break;
                case 1:
                    pci_set_rw(0xC0000, data);
                    pci_set_rw(0xC4000, data >> 4);
                    break;
                case 2:
                    pci_set_rw(0xC8000, data);
                    pci_set_rw(0xCC000, data >> 4);
                    break;
                case 3:
                    pci_set_rw(0xD0000, data);
                    pci_set_rw(0xD4000, data >> 4);
                    break;
                case 4:
                    pci_set_rw(0xD8000, data);
                    pci_set_rw(0xDC000, data >> 4);
                    break;
                case 5:
                    pci_set_rw(0xE0000, data);
                    pci_set_rw(0xE4000, data >> 4);
                    break;
                case 6:
                    pci_set_rw(0xE8000, data);
                    pci_set_rw(0xEC000, data >> 4);
                    break;
            }
            break;
        case 0x60 ... 0x67:
            PCI_LOG("TODO: Handle 82441FX DRB registers\n");
            break;
        case 0x72:
            PCI_LOG("TODO: Handle SMRAM\n");
            break;
    }
    ptr[addr] = res;
    return retval;
}
static uint32_t mmio_readb(uint32_t addr)
{
    return ram[addr];
}
static void mmio_writeb(uint32_t addr, uint32_t data)
{
    int map = pci.rom_area_memory_mapping[(addr - 0xC0000) >> 14];
    if (map & 2)
        ram[addr] = data;
    else {
        PCI_LOG("Invalid write addr=%08x data=%02x\n", addr, data);
    }
}
void pci_init_mem(void *a)
{
    ram = (uint8_t *)a;
}
static void pci_82441fx_init(void)
{
    void *ptr = pci_create_device(0, 0, 0, pci_82441fx_write);
    pci_copy_default_configuration(ptr, (void *)configuration_space_82441fx, 128);
    io_register_mmio_read(0xC0000, 0x40000, mmio_readb, NULL, NULL);
    io_register_mmio_write(0xC0000, 0x40000, mmio_writeb, NULL, NULL);
}
static void pci_82441fx_reset(void)
{
    pci_copy_default_configuration(pci_get_configuration_ptr(0, 0, 0), (void *)configuration_space_82441fx, 128);
}

static int pci_82371sb_isa_write(uint8_t *ptr, uint8_t addr, uint8_t data)
{
    uint8_t res    = data;
    int     retval = 0;
    switch (addr) {
        case 0 ... 3:
        case 0x08 ... 0x4B:
        case 0x4D:
        case 0x50 ... 0x5F:
        case 0x64 ... 0x68:
        case 0x6C ... 0x6F:
        case 0x71 ... 0x75:
        case 0x7A ... 0x7F:
        case 0x81:
        case 0x83 ... 0x9F:
        case 0xA1:
        case 0xA9:
        case 0xAD:
        case 0xAF ... 0xFF:
            res    = ptr[addr];
            retval = 1;
            break;
        case 4:
            res    = (res & 8) | 7;
            retval = 1;
            break;
        case 5:
            res &= 1;
            retval = 1;
            break;
        case 7:
            res = ptr[addr] & ~(data & 0x78);
            res |= 2;
            retval = 1;
            break;
        case 0x4F:
            res &= 1;
            PCI_LOG("Set enable apic=%d\n", res);
            retval = 1;
            break;
        case 0x60 ... 0x63:
            res &= 0x8F;
            retval = 1;
            break;
        case 0x6A:
            res &= 0xD7;
            retval = 1;
            break;
        case 0x80:
            res &= 0x80;
            retval = 1;
            break;
    }
    ptr[addr] = res;
    return retval;
}
static void pci_82371sb_isa_init(void)
{
    void *ptr = pci_create_device(0, 1, 0, pci_82371sb_isa_write);
    pci_copy_default_configuration(ptr, (void *)configuration_space_82371sb_isa, 192);
}
static void pci_82371sb_isa_reset(void)
{
    pci_copy_default_configuration(pci_get_configuration_ptr(0, 1, 0), (void *)configuration_space_82371sb_isa, 192);
}
static void pci_82371sb_ide_io_write_handler(uint32_t port, uint32_t data)
{
    ide_write_prdt(port, data);
}
static uint32_t pci_82371sb_ide_io_read_handler(uint32_t port)
{
    return ide_read_prdt(port);
}
static void pci_82371sb_ide_remap(uint32_t old)
{
    void    *info      = pci_get_configuration_ptr(0, 1, 1);
    uint32_t base_addr = *((uint32_t *)((uint8_t *)info + 0x20));
    base_addr &= 0xFFFF;
    if ((base_addr & 1) == 0)
        PCI_FATAL("Remapping PIIX3 PRDT to memory not supported\n");
    base_addr &= ~15;
    io_unregister_read(old, 16);
    io_unregister_write(old, 16);
    io_register_read(base_addr, 16, pci_82371sb_ide_io_read_handler, NULL, NULL);
    io_register_write(base_addr, 16, pci_82371sb_ide_io_write_handler, NULL, NULL);
}
static int pci_82371sb_ide_write(uint8_t *ptr, uint8_t addr, uint8_t data)
{
    uint8_t  res      = data;
    int      retval   = 0;
    uint32_t old_bar4 = *((uint32_t *)(ptr + 0x20)) & 0xFFFC, new_bar4;
    switch (addr) {
        case 0 ... 3:
        case 8 ... 12:
        case 0x0E ... 0x1F:
        case 0x24 ... 0x3F:
        case 0x45 ... 0xFF:
            res    = ptr[addr];
            retval = 1;
            break;
        case 0x20:
            res |= 1;
            retval = 1;
            break;
        case 0x22 ... 0x23:
            retval = 1;
            break;
    }
    ptr[addr] = res;
    new_bar4  = *((uint32_t *)(ptr + 0x20)) & 0xFFFC;
    if (old_bar4 != new_bar4 && addr == 0x23) {
        PCI_LOG("Remapping PRDT to %04x\n", new_bar4);
        pci_82371sb_ide_remap(old_bar4);
    }
    return retval;
}
static void pci_82371sb_ide_init(void)
{
    uint8_t *ptr = (uint8_t *)pci_create_device(0, 1, 1, pci_82371sb_ide_write);
    pci_copy_default_configuration(ptr, (void *)configuration_space_82371sb_ide, 64);
    ptr[0x41]           = 0x80;
    ptr[0x43]           = 0x80;
    const int prdt_base = 0xc000 | 1;
    ptr[0x20]           = prdt_base & 0xFF;
    ptr[0x21]           = prdt_base >> 8 & 0xFF;
    pci_82371sb_ide_remap(prdt_base);
}
static void pci_82371sb_ide_reset(void)
{
    uint8_t *ptr = (uint8_t *)pci_get_configuration_ptr(0, 1, 1);
    pci_copy_default_configuration(ptr, (void *)configuration_space_82371sb_ide, 64);
    ptr[0x41]           = 0x80;
    ptr[0x43]           = 0x80;
    const int prdt_base = 0xc000 | 1;
    ptr[0x20]           = prdt_base & 0xFF;
    ptr[0x21]           = prdt_base >> 8 & 0xFF;
    pci_82371sb_ide_remap(prdt_base);
}
static void pci_reset(void)
{
    pci_82441fx_reset();
    pci_82371sb_isa_reset();
    pci_82371sb_ide_reset();
    pci_mark_rom_area(0xC0000, 0);
    pci_mark_rom_area(0xC4000, 0);
    pci_mark_rom_area(0xC8000, 0);
    pci_mark_rom_area(0xCC000, 0);
    pci_mark_rom_area(0xD0000, 0);
    pci_mark_rom_area(0xD4000, 0);
    pci_mark_rom_area(0xD8000, 0);
    pci_mark_rom_area(0xDC000, 0);
    pci_mark_rom_area(0xE0000, 0);
    pci_mark_rom_area(0xE4000, 0);
    pci_mark_rom_area(0xE8000, 0);
    pci_mark_rom_area(0xEC000, 0);
    pci_mark_rom_area(0xF0000, 0);
    pci_mark_rom_area(0xF4000, 0);
    pci_mark_rom_area(0xF8000, 0);
    pci_mark_rom_area(0xFC000, 0);
}
static void pci_state(void)
{
    int n = 0;
    for (int i = 0; i < 256; i++) {
        if (pci.configuration_address_spaces[i])
            n++;
    }
    struct bjson_object *obj = state_obj((char *)"pci", 3 + n);
    state_field(obj, 4, (char *)"pci.configuration_address_register", &pci.configuration_address_register);
    state_field(obj, 4, (char *)"pci.configuration_cycle", &pci.configuration_cycle);
    state_field(obj, 16, (char *)"pci.rom_area_memory_mapping", &pci.rom_area_memory_mapping);
    for (int i = 0; i < 16; i++) {
        pci_set_rw(0xC0000 + (i << 14), pci.rom_area_memory_mapping[i]);
    }
    for (int i = 0; i < 256; i++) {
        char pci_name[50];
        sprintf(pci_name, "pci.configuration[%d]", i);
        if (pci.configuration_address_spaces[i]) {
            state_field(obj, 256, pci_name, pci.configuration_address_spaces[i]);
        }
    }
}
void pci_init(struct pc_settings *pc)
{
    if (!pc->pci_enabled)
        return;
    io_register_read(0xCF8, 8, pci_read, pci_read16, pci_read32);
    io_register_write(0xCF8, 8, pci_write, pci_write16, pci_write32);
    state_register(pci_state);
    io_register_reset(pci_reset);
    pci_82441fx_init();
    pci_82371sb_isa_init();
    pci_82371sb_ide_init();
}
void pci_set_irq_line(int dev, int state)
{
    uint8_t *config  = pci.configuration_address_spaces[(dev << 3) & 0xFF],
            *config2 = pci.configuration_address_spaces[(DEV_82371SB_ID << 3) & 0xFF];
    if (!config)
        PCI_FATAL("Trying to raise IRQ line for non-existent device!\n");
    if (state == 1) {
        int pin = config[0x3D] - 1, devN = (uint8_t)(dev - 1), pin_offset = 0x60 + ((pin + devN) & 3);
        pic_raise_irq(config2[pin_offset]);
    } else {
        int pin = config[0x3D], devN = dev, pin_offset = 0x60 + ((pin + devN - 2) & 3);
        pic_lower_irq(config2[pin_offset]);
    }
}