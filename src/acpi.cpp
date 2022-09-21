
#include "devices.h"
#include "io.h"
#include <cstdint>
#include "util.h"

#define ACPI_LOG(x, ...)   LOG("ACPI", x, ##__VA_ARGS__)
#define ACPI_FATAL(x, ...) FATAL("ACPI", x, ##__VA_ARGS__)
#define ACPI_CLOCK_SPEED   3579545

struct acpi_state
{
    int      enabled;
    uint32_t pmba;
    int      pmiose;
    uint32_t pmsts_en;
    uint32_t pmcntrl;
    uint32_t last_pm_clock;
    uint32_t smba;
    int      smiose;
} acpi;

static const uint8_t acpi_configuration_space[256] = {
    134, 128, 19, 113, 0, 0, 128, 2,   0, 0, 128, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,  0,   0, 0, 0,   0,   0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    1,   0,   0,  0,   0, 0, 0,   0,   0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 144,
    0,   0,   0,  96,  0, 0, 0,   152, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,  0,   0, 0, 0,   0,   0, 0, 0,   0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,  0,   0, 0, 0,   0,   0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,  0,   0, 0, 0,   0,   0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,   0,   0,  0,   0, 0, 0,   0,   0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


static void acpi_reset(void)
{
    acpi.pmcntrl = 1;
}
static inline uint32_t read32le(uint8_t *x, uint32_t offset)
{
    uint32_t res = x[offset];
    res |= x[offset + 1] << 8;
    res |= x[offset + 2] << 16;
    res |= x[offset + 3] << 24;
    return res;
}
static uint32_t acpi_get_clock(itick_t now)
{
    return (double)now * (double)ACPI_CLOCK_SPEED / (double)ticks_per_second;
}
static uint32_t acpi_pm_read(uint32_t addr)
{
    int      offset = addr & 3;
    uint32_t result = 0;
    switch (addr & 0x3C) {
        case 0:
            result = acpi.pmsts_en;
            break;
        case 4:
            result = acpi.pmcntrl;
            break;
        case 8:
            result = acpi_get_clock(get_now());
            break;
        default:
            ACPI_FATAL("TODO: power management read: %04x\n", addr);
    }
    return result >> (offset * 8);
}
static void acpi_pm_write(uint32_t addr, uint32_t data)
{
    int shift = (addr & 3) * 8;
    switch (addr & 0x3C) {
        case 0:
            if ((addr & 2) == 0) {
                data = ~data;
                acpi.pmsts_en &= (data << shift) | (0xFF << (shift ^ 8));
            } else {
                acpi.pmsts_en &= 0xFF << (shift ^ 8);
                acpi.pmsts_en |= data << shift;
            }
            break;
        case 4:
            acpi.pmcntrl &= ~(0xFF << shift);
            acpi.pmcntrl |= data << shift;
            if (acpi.pmsts_en & (1 << 13)) {
                if ((acpi.pmiose >> 10 & 7) != 5)
                    ACPI_FATAL("Unimplemented: Suspend state %d\n", acpi.pmiose >> 10 & 7);
                acpi.pmsts_en ^= 1 << 13;
            }
            break;
        default:
            ACPI_FATAL("TODO: power management write: %04x data %04x\n", addr, data);
    }
}
static uint32_t acpi_sm_read(uint32_t addr)
{
    switch (addr & 0xF) {
        case 0:
            return 0;
        case 2:
            return 0;
        case 3:
            return 0;
        case 4:
            return 0;
        case 5 ... 6:
            return 0;
        default:
            ACPI_FATAL("TODO: system management read: %04x\n", addr);
    }
}
static void acpi_sm_write(uint32_t addr, uint32_t data)
{
    switch (addr & 0xF) {
        case 0:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        case 5 ... 6:
            break;
        default:
            ACPI_FATAL("TODO: system management read: %04x data %04x\n", addr, data);
    }
}
static void acpi_remap_pmba(uint32_t io)
{
    ACPI_LOG("Remapping Power Management I/O ports to %04x\n", io);
    if (acpi.pmba != 0) {
        io_unregister_read(acpi.pmba, 64);
        io_unregister_write(acpi.pmba, 64);
    }
    acpi.pmba = io & 0xFFC0;
    if (io != 0) {
        io_register_read(acpi.pmba, 64, acpi_pm_read, NULL, NULL);
        io_register_write(acpi.pmba, 64, acpi_pm_write, NULL, NULL);
    }
}
static void acpi_remap_smba(uint32_t io)
{
    ACPI_LOG("Remapping System Management I/O ports to %04x\n", io);
    if (acpi.smba != 0) {
        io_unregister_read(acpi.smba, 64);
        io_unregister_write(acpi.smba, 64);
    }
    acpi.smba = io & 0xFFC0;
    if (io != 0) {
        io_register_read(acpi.smba, 64, acpi_sm_read, NULL, NULL);
        io_register_write(acpi.smba, 64, acpi_sm_write, NULL, NULL);
    }
}
static int acpi_pci_write(uint8_t *ptr, uint8_t addr, uint8_t data)
{
    switch (addr) {
        case 0 ... 3:
        case 4 ... 5:
            ptr[addr]   = data;
            acpi.smiose = data & 1;
            return 0;
        case 6 ... 7:
            return 0;
        case 8 ... 0x3B:
            return 1;
        case 0x3C:
            return 0;
        case 0x40 ... 0x43:
            ptr[addr] = data | (addr == 0x40);
            if (addr == 0x43)
                acpi_remap_pmba(read32le(ptr, 0x40));
            return 0;
        case 0x58 ... 0x5B:
            return 0;
        case 0x80:
            acpi.pmiose = data & 1;
            return 0;
        case 0x90 ... 0x93:
            ptr[addr] = data | (addr == 0x90);
            if (addr == 0x93)
                acpi_remap_smba(read32le(ptr, 0x90));
            return 0;
        case 0xD2:
            acpi.smiose = data & 1;
            if ((data >> 1 & 7) != 4) {
                ACPI_FATAL("Unknown SMBus interrupt delivery mechanism\n");
            }
            return 0;
    }
    ACPI_FATAL("Unknown write: %p addr=%02x data=%02x\n", ptr, addr, data);
}
int acpi_next(itick_t now_tick)
{
    if (acpi.enabled == 0)
        return -1;
    uint32_t now = acpi_get_clock(now_tick) & 0x00FFFFFF, then = acpi.last_pm_clock & 0x00FFFFFF, raise_irq = 0;
    if (now < then) {
        raise_irq = 1;
    }
    if (acpi.pmsts_en & (1 << 16)) {
        acpi.pmsts_en |= 1;
        if (raise_irq) {
            pic_raise_irq(9);
        } else
            pic_lower_irq(9);
        acpi.last_pm_clock  = acpi_get_clock(now_tick);
        uint32_t ticks_left = 0x1000000 - now;
        return (double)ticks_left * (double)ticks_per_second / (double)ACPI_CLOCK_SPEED;
    } else {
        pic_lower_irq(9);
        return -1;
    }
}
static void acpi_state(void)
{
    struct bjson_object *obj = state_obj((char *)"acpi", 8);
    state_field(obj, 4, (char *)"acpi.enabled", &acpi.enabled);
    state_field(obj, 4, (char *)"acpi.pmba", &acpi.pmba);
    state_field(obj, 4, (char *)"acpi.pmiose", &acpi.pmiose);
    state_field(obj, 4, (char *)"acpi.pmsts_en", &acpi.pmsts_en);
    state_field(obj, 4, (char *)"acpi.pmcntrl", &acpi.pmcntrl);
    state_field(obj, 4, (char *)"acpi.last_pm_clock", &acpi.last_pm_clock);
    state_field(obj, 4, (char *)"acpi.smba", &acpi.smba);
    state_field(obj, 4, (char *)"acpi.smiose", &acpi.smiose);
    acpi_remap_pmba(acpi.pmba);
    acpi_remap_smba(acpi.smba);
}
void acpi_init(struct pc_settings *pc)
{
    if (!pc->acpi_enabled)
        return;
    acpi.enabled = 1;
    if (!pc->pci_enabled)
        ACPI_LOG("Disabling ACPI because PCI is disabled\n");
    io_register_reset(acpi_reset);
    state_register(acpi_state);
    uint8_t *ptr = (uint8_t *)pci_create_device(0, 7, 0, acpi_pci_write);
    pci_copy_default_configuration(ptr, (void *)acpi_configuration_space, 256);
    ptr[0x5B] |= 2;
}