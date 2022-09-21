#ifndef PC_H
#define PC_H
#include "drive.h"
#include "cpu/cpu.h"
#include <stdint.h>

#define MAX_VIRTIO_DEVICES 2


enum
{
    DRIVE_TYPE_NONE = 0,
    DRIVE_TYPE_DISK,
    DRIVE_TYPE_CDROM
};
enum
{
    CPU_TYPE_486,
    CPU_TYPE_PENTIUM,
    CPU_TYPE_PENTIUM_PRO,
    CPU_TYPE_PENTIUM_4,
    CPU_TYPE_CORE_DUO
};
enum
{
    VIRTIO_9P
};
enum
{
    BOOT_NONE   = 0,
    BOOT_FLOPPY = 1,
    BOOT_DISK   = 2,
    BOOT_CDROM  = 3
};

struct loaded_file
{
    uint32_t length;
    void    *data;
};
struct ne2000_settings
{
    int     enabled;
    int     port_base;
    int     pci;
    int     irq;
    uint8_t mac_address[6];
};
struct virtio_9p_cfg
{
    char *path;
    int   ro;
};
struct virtio_cfg
{
    int type;
    union
    {
        struct virtio_9p_cfg fs9p;
    };
};
struct pc_settings
{
    uint32_t           memory_size, vga_memory_size;
    struct loaded_file bios, vgabios;
    unsigned int       cpu_type;
    int                pci_enabled, apic_enabled, acpi_enabled, vbe_enabled, pci_vga_enabled;
    uint64_t           current_time;
    uint8_t            boot_sequence[3];
    struct drive_info  drives[4];
    int                floppy_enabled;
    struct
    {
        int write_protected;
    } floppy_settings[2];
    struct ne2000_settings ne2000;
    struct drive_info      floppy_drives[2];
    struct cpu_config      cpu;
    struct virtio_cfg      virtio[MAX_VIRTIO_DEVICES];
    int                    boot_kernel;
    char                  *kernel_cmdline, *kernel_img;
};

int      pc_init(struct pc_settings *pc);
int      pc_execute(void);
uint32_t pc_run(void);
void     pc_set_a20(int state);
void     pc_in_hlt(void);
void     pc_hlt_if_0(void);
void     pc_run_device_timers(void);
void     pc_out_of_hlt(void);
void     pc_hlt_state(void);
#endif
