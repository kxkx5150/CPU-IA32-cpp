#include "cpu/cpu.h"
#include "devices.h"
#include "drive.h"
#include <alloca.h>
#include <cstdint>
#include <string.h>

#define IDE_LOG(x, ...)   LOG("IDE", x, ##__VA_ARGS__)
#define IDE_FATAL(x, ...) FATAL("IDE", x, ##__VA_ARGS__);

#define ATA_STATUS_BSY                 0x80
#define ATA_STATUS_DRDY                0x40
#define ATA_STATUS_DF                  0x20
#define ATA_STATUS_DSC                 0x10
#define ATA_STATUS_DRQ                 0x08
#define ATA_STATUS_CORR                0x04
#define ATA_STATUS_IDX                 0x02
#define ATA_STATUS_ERR                 0x01
#define ATA_ERROR_BBK                  0x80
#define ATA_ERROR_UNC                  0x40
#define ATA_ERROR_MC                   0x20
#define ATA_ERROR_IDNF                 0x10
#define ATA_ERROR_MCR                  0x08
#define ATA_ERROR_ABRT                 0x04
#define ATA_ERROR_TK0NF                0x02
#define ATA_ERROR_AMNF                 0x01
#define ATAPI_INTERRUPT_REASON_REL     0x04
#define ATAPI_INTERRUPT_REASON_IO      0x02
#define ATAPI_INTERRUPT_REASON_CoD     0x01
#define ATAPI_SENSE_NONE               0x00
#define ATAPI_SENSE_NOT_READY          0x02
#define ATAPI_SENSE_MEDIUM_ERROR       0x03
#define ATAPI_SENSE_HARDWARE_ERROR     0x04
#define ATAPI_SENSE_ILLEGAL_REQUEST    0x05
#define ATAPI_SENSE_UNIT_ATTENTION     0x06
#define ATAPI_SENSE_ABORTED            0x0B
#define ATAPI_ASC_CAUSE_NOT_REPORTABLE 0x0400
#define ATAPI_ASC_GETTING_READY        0x0401
#define ATAPI_ASC_MANUAL_INTERVENTION  0x0403
#define ATAPI_ASC_BSY                  0x0407
#define ATAPI_ASC_OFFLINE              0x0412
#define ATAPI_ASC_MAINTENANCE          0x0481
#define ATAPI_ASC_OUT_OF_RANGE         0x2000
#define ATAPI_ASC_CLEANING_CARTRIDGE   0x3003
#define ATAPI_ASC_NOT_PRESENT          0x3A02
#define ATAPI_ASC_INVALID_FIELD        0x2400
#define ATAPI_ASC_INVALID_OFFSET       0x2100
#define ATAPI_ERROR_ABRT               0x04
#define ATAPI_ERROR_EOM                0x02
#define ATAPI_ERROR_ILI                0x01
#define DISABLE_MULTIPLE_SECTORS
#define MAX_MULTIPLE_SECTORS   1
#define SELECTED(obj, field)   obj->field[obj->selected]
#define TRANSLATED(obj, field) obj->field[(obj->selected << 1) | (SELECTED(obj, translated))]


extern CPU cpu;

static void ide_read_sectors(struct ide_controller *ctrl, int lba48, int chunk_count);
static void drive_write_callback(void *self, int res);


static struct ide_controller
{
    int      selected;
    int      lba;
    int      sectors_read;
    int      lba48;
    int      multiple_sectors_count;
    uint8_t  error, feature, drive_and_head;
    uint16_t sector_number, cylinder_low, cylinder_high, sector_count;
    uint8_t  device_control, status;
    uint8_t  command_issued;
    uint32_t pio_position, pio_length;
    uint32_t canary_below;
    union
    {
        uint8_t  pio_buffer[16 * 512];
        uint16_t pio_buffer16[16 * 512 / 2];
        uint32_t pio_buffer32[16 * 512 / 4];
    };
    uint32_t canary_above;
    int      irq_status;
    int      type[2];
    int      media_inserted[2];
    int      translated[2];
    uint32_t sectors_per_track[4];
    uint32_t heads[4];
    uint32_t cylinders[4];
    uint32_t total_sectors_chs[2];
    uint32_t total_sectors[2];
    uint8_t  dma_command, dma_status;
    uint32_t prdt_address;
    int      dma_enabled;
    uint16_t mdma, udma;
    uint8_t  sense_key;
    uint16_t asc;
    uint32_t atapi_lba;
    uint32_t atapi_sectors_to_read, atapi_sector_size;
    uint32_t atapi_bytes_to_transfer, atapi_cylinder_count, atapi_frame_bytes_to_transfer,
        atapi_frame_bytes_transferred, atapi_total_bytes_transferred;
    uint8_t            atapi_command;
    uint8_t            atapi_can_eject_cdrom;
    uint8_t            atapi_dma_enabled;
    struct drive_info *info[2];
} ide[2];


static void ide_state(void)
{
    struct bjson_object *obj = state_obj((char *)"ide[NUMBER]", (46) * 2);
    state_field(obj, 4, (char *)"ide[0].selected", &ide[0].selected);
    state_field(obj, 4, (char *)"ide[1].selected", &ide[1].selected);
    state_field(obj, 4, (char *)"ide[0].lba", &ide[0].lba);
    state_field(obj, 4, (char *)"ide[1].lba", &ide[1].lba);
    state_field(obj, 4, (char *)"ide[0].sectors_read", &ide[0].sectors_read);
    state_field(obj, 4, (char *)"ide[1].sectors_read", &ide[1].sectors_read);
    state_field(obj, 4, (char *)"ide[0].lba48", &ide[0].lba48);
    state_field(obj, 4, (char *)"ide[1].lba48", &ide[1].lba48);
    state_field(obj, 4, (char *)"ide[0].multiple_sectors_count", &ide[0].multiple_sectors_count);
    state_field(obj, 4, (char *)"ide[1].multiple_sectors_count", &ide[1].multiple_sectors_count);
    state_field(obj, 1, (char *)"ide[0].error", &ide[0].error);
    state_field(obj, 1, (char *)"ide[1].error", &ide[1].error);
    state_field(obj, 1, (char *)"ide[0].feature", &ide[0].feature);
    state_field(obj, 1, (char *)"ide[1].feature", &ide[1].feature);
    state_field(obj, 1, (char *)"ide[0].drive_and_head", &ide[0].drive_and_head);
    state_field(obj, 1, (char *)"ide[1].drive_and_head", &ide[1].drive_and_head);
    state_field(obj, 2, (char *)"ide[0].sector_number", &ide[0].sector_number);
    state_field(obj, 2, (char *)"ide[1].sector_number", &ide[1].sector_number);
    state_field(obj, 2, (char *)"ide[0].cylinder_low", &ide[0].cylinder_low);
    state_field(obj, 2, (char *)"ide[1].cylinder_low", &ide[1].cylinder_low);
    state_field(obj, 2, (char *)"ide[0].cylinder_high", &ide[0].cylinder_high);
    state_field(obj, 2, (char *)"ide[1].cylinder_high", &ide[1].cylinder_high);
    state_field(obj, 2, (char *)"ide[0].sector_count", &ide[0].sector_count);
    state_field(obj, 2, (char *)"ide[1].sector_count", &ide[1].sector_count);
    state_field(obj, 1, (char *)"ide[0].device_control", &ide[0].device_control);
    state_field(obj, 1, (char *)"ide[1].device_control", &ide[1].device_control);
    state_field(obj, 1, (char *)"ide[0].status", &ide[0].status);
    state_field(obj, 1, (char *)"ide[1].status", &ide[1].status);
    state_field(obj, 1, (char *)"ide[0].command_issued", &ide[0].command_issued);
    state_field(obj, 1, (char *)"ide[1].command_issued", &ide[1].command_issued);
    state_field(obj, 4, (char *)"ide[0].pio_position", &ide[0].pio_position);
    state_field(obj, 4, (char *)"ide[1].pio_position", &ide[1].pio_position);
    state_field(obj, 4, (char *)"ide[0].pio_length", &ide[0].pio_length);
    state_field(obj, 4, (char *)"ide[1].pio_length", &ide[1].pio_length);
    state_field(obj, 8192, (char *)"ide[0].pio_buffer", &ide[0].pio_buffer);
    state_field(obj, 8192, (char *)"ide[1].pio_buffer", &ide[1].pio_buffer);
    state_field(obj, 4, (char *)"ide[0].irq_status", &ide[0].irq_status);
    state_field(obj, 4, (char *)"ide[1].irq_status", &ide[1].irq_status);
    state_field(obj, 8, (char *)"ide[0].type", &ide[0].type);
    state_field(obj, 8, (char *)"ide[1].type", &ide[1].type);
    state_field(obj, 8, (char *)"ide[0].media_inserted", &ide[0].media_inserted);
    state_field(obj, 8, (char *)"ide[1].media_inserted", &ide[1].media_inserted);
    state_field(obj, 8, (char *)"ide[0].translated", &ide[0].translated);
    state_field(obj, 8, (char *)"ide[1].translated", &ide[1].translated);
    state_field(obj, 16, (char *)"ide[0].sectors_per_track", &ide[0].sectors_per_track);
    state_field(obj, 16, (char *)"ide[1].sectors_per_track", &ide[1].sectors_per_track);
    state_field(obj, 16, (char *)"ide[0].heads", &ide[0].heads);
    state_field(obj, 16, (char *)"ide[1].heads", &ide[1].heads);
    state_field(obj, 16, (char *)"ide[0].cylinders", &ide[0].cylinders);
    state_field(obj, 16, (char *)"ide[1].cylinders", &ide[1].cylinders);
    state_field(obj, 8, (char *)"ide[0].total_sectors_chs", &ide[0].total_sectors_chs);
    state_field(obj, 8, (char *)"ide[1].total_sectors_chs", &ide[1].total_sectors_chs);
    state_field(obj, 8, (char *)"ide[0].total_sectors", &ide[0].total_sectors);
    state_field(obj, 8, (char *)"ide[1].total_sectors", &ide[1].total_sectors);
    state_field(obj, 1, (char *)"ide[0].dma_command", &ide[0].dma_command);
    state_field(obj, 1, (char *)"ide[1].dma_command", &ide[1].dma_command);
    state_field(obj, 1, (char *)"ide[0].dma_status", &ide[0].dma_status);
    state_field(obj, 1, (char *)"ide[1].dma_status", &ide[1].dma_status);
    state_field(obj, 4, (char *)"ide[0].prdt_address", &ide[0].prdt_address);
    state_field(obj, 4, (char *)"ide[1].prdt_address", &ide[1].prdt_address);
    state_field(obj, 4, (char *)"ide[0].dma_enabled", &ide[0].dma_enabled);
    state_field(obj, 4, (char *)"ide[1].dma_enabled", &ide[1].dma_enabled);
    state_field(obj, 2, (char *)"ide[0].mdma", &ide[0].mdma);
    state_field(obj, 2, (char *)"ide[1].mdma", &ide[1].mdma);
    state_field(obj, 2, (char *)"ide[0].udma", &ide[0].udma);
    state_field(obj, 2, (char *)"ide[1].udma", &ide[1].udma);
    state_field(obj, 1, (char *)"ide[0].sense_key", &ide[0].sense_key);
    state_field(obj, 1, (char *)"ide[1].sense_key", &ide[1].sense_key);
    state_field(obj, 2, (char *)"ide[0].asc", &ide[0].asc);
    state_field(obj, 2, (char *)"ide[1].asc", &ide[1].asc);
    state_field(obj, 4, (char *)"ide[0].atapi_lba", &ide[0].atapi_lba);
    state_field(obj, 4, (char *)"ide[1].atapi_lba", &ide[1].atapi_lba);
    state_field(obj, 4, (char *)"ide[0].atapi_sectors_to_read", &ide[0].atapi_sectors_to_read);
    state_field(obj, 4, (char *)"ide[1].atapi_sectors_to_read", &ide[1].atapi_sectors_to_read);
    state_field(obj, 4, (char *)"ide[0].atapi_sector_size", &ide[0].atapi_sector_size);
    state_field(obj, 4, (char *)"ide[1].atapi_sector_size", &ide[1].atapi_sector_size);
    state_field(obj, 4, (char *)"ide[0].atapi_bytes_to_transfer", &ide[0].atapi_bytes_to_transfer);
    state_field(obj, 4, (char *)"ide[1].atapi_bytes_to_transfer", &ide[1].atapi_bytes_to_transfer);
    state_field(obj, 4, (char *)"ide[0].atapi_cylinder_count", &ide[0].atapi_cylinder_count);
    state_field(obj, 4, (char *)"ide[1].atapi_cylinder_count", &ide[1].atapi_cylinder_count);
    state_field(obj, 4, (char *)"ide[0].atapi_frame_bytes_to_transfer", &ide[0].atapi_frame_bytes_to_transfer);
    state_field(obj, 4, (char *)"ide[1].atapi_frame_bytes_to_transfer", &ide[1].atapi_frame_bytes_to_transfer);
    state_field(obj, 4, (char *)"ide[0].atapi_frame_bytes_transferred", &ide[0].atapi_frame_bytes_transferred);
    state_field(obj, 4, (char *)"ide[1].atapi_frame_bytes_transferred", &ide[1].atapi_frame_bytes_transferred);
    state_field(obj, 4, (char *)"ide[0].atapi_total_bytes_transferred", &ide[0].atapi_total_bytes_transferred);
    state_field(obj, 4, (char *)"ide[1].atapi_total_bytes_transferred", &ide[1].atapi_total_bytes_transferred);
    state_field(obj, 1, (char *)"ide[0].atapi_command", &ide[0].atapi_command);
    state_field(obj, 1, (char *)"ide[1].atapi_command", &ide[1].atapi_command);
    state_field(obj, 1, (char *)"ide[0].atapi_can_eject_cdrom", &ide[0].atapi_can_eject_cdrom);
    state_field(obj, 1, (char *)"ide[1].atapi_can_eject_cdrom", &ide[1].atapi_can_eject_cdrom);
    state_field(obj, 1, (char *)"ide[0].atapi_dma_enabled", &ide[0].atapi_dma_enabled);
    state_field(obj, 1, (char *)"ide[1].atapi_dma_enabled", &ide[1].atapi_dma_enabled);
    char filename[1000];
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            struct drive_info *info = ide[i].info[j];
            if (ide[i].media_inserted[j]) {
                sprintf(filename, "ide%d-%d", i, j);
                drive_state(info, filename);
            }
        }
    }
}
static inline int get_ctrl_id(struct ide_controller *ctrl)
{
    return ctrl == &ide[1];
}
static inline int selected_drive_has_media(struct ide_controller *ctrl)
{
    return ctrl->type[ctrl->selected] != DRIVE_TYPE_NONE;
}
static inline int controller_has_media(struct ide_controller *ctrl)
{
    return ctrl->media_inserted[0] | ctrl->media_inserted[1];
}
static void ide_update_irq(struct ide_controller *ctrl)
{
    if (ctrl->irq_status && !(ctrl->device_control & 2))
        pic_raise_irq(get_ctrl_id(ctrl) | 14);
    else
        pic_lower_irq(get_ctrl_id(ctrl) | 14);
}
static inline void ide_lower_irq(struct ide_controller *ctrl)
{
    ctrl->irq_status = 0;
    ide_update_irq(ctrl);
}
static inline void ide_raise_irq(struct ide_controller *ctrl)
{
    ctrl->dma_status |= 0x04;
    ctrl->irq_status = 1;
    ide_update_irq(ctrl);
}
static void ide_abort_command(struct ide_controller *ctrl)
{
    ctrl->status       = 0x40 | 0x10 | 0x01;
    ctrl->error        = 0x04;
    ctrl->pio_position = 0;
    ctrl->dma_status |= 2;
    ide_raise_irq(ctrl);
}
static void ide_reset(void)
{
    ide[0].selected      = 0;
    ide[1].selected      = 0;
    ide[0].translated[0] = 0;
    ide[0].translated[1] = 0;
    ide[1].translated[0] = 0;
    ide[1].translated[1] = 0;
}
static uint32_t ide_get_sector_count(struct ide_controller *ctrl, int lba48)
{
    if (lba48)
        return (!ctrl->sector_count) << 16 | ctrl->sector_count;
    else {
        uint8_t real_sector_count = ctrl->sector_count;
        return (!real_sector_count) << 8 | real_sector_count;
    }
}
static uint64_t ide_get_sector_offset(struct ide_controller *ctrl, int lba48)
{
    uint64_t res;
    switch ((lba48 << 1 | ctrl->lba) & 3) {
        case 0: {
            uint64_t cyl = (ctrl->cylinder_low & 0xFF) | ((ctrl->cylinder_high << 8) & 0xFFFF);
            res          = cyl * ctrl->heads[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])] *
                  ctrl->sectors_per_track[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])];
            uint64_t heads = ctrl->drive_and_head & 0x0F;
            res += heads * ctrl->sectors_per_track[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])];
            res += (ctrl->sector_number & 0xFF) - 1;
            break;
        }
        case 1:
            res = ctrl->sector_number & 0xFF;
            res |= (ctrl->cylinder_low & 0xFF) << 8;
            res |= (ctrl->cylinder_high & 0xFF) << 16;
            res |= (ctrl->drive_and_head & 0x0F) << 24;
            break;
        case 2 ... 3: {
            res = ctrl->sector_number & 0xFF;
            res |= (uint64_t)(ctrl->cylinder_low & 0xFF) << 8L;
            res |= (uint64_t)(ctrl->cylinder_high & 0xFF) << 16L;
            res |= (uint64_t)(ctrl->sector_count >> 8 & 0xFF) << 24L;
            res |= (uint64_t)(ctrl->cylinder_low >> 8 & 0xFF) << 32L;
            res |= (uint64_t)(ctrl->cylinder_high >> 8 & 0xFF) << 40L;
            break;
        }
    }
    return res;
}
static void ide_set_sector_offset(struct ide_controller *ctrl, int lba48, uint64_t position)
{
    switch (lba48 << 1 | ctrl->lba) {
        case 0: {
            uint32_t heads_spt = ctrl->heads[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])] *
                                 ctrl->sectors_per_track[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])];
            uint32_t c = position / heads_spt;
            uint32_t t = position % heads_spt;
            uint32_t h = t / ctrl->sectors_per_track[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])];
            uint32_t s = t % ctrl->sectors_per_track[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])];
            ctrl->cylinder_low   = c & 0xFF;
            ctrl->cylinder_high  = c >> 8 & 0xFF;
            ctrl->drive_and_head = (ctrl->drive_and_head & 0xF0) | (h & 0x0F);
            ctrl->sector_number  = s + 1;
            break;
        }
        case 1:
            ctrl->drive_and_head = (ctrl->drive_and_head & 0xF0) | (position >> 24 & 0x0F);
            ctrl->cylinder_high  = position >> 16 & 0xFF;
            ctrl->cylinder_low   = position >> 8 & 0xFF;
            ctrl->sector_number  = position >> 0 & 0xFF;
            break;
        case 2:
        case 3: {
            ctrl->sector_number = position & 0xFF;
            ctrl->cylinder_low  = position >> 8 & 0xFF;
            ctrl->cylinder_high = position >> 16 & 0xFF;
            ctrl->sector_number |= (position >> 24 & 0xFF) << 8;
            ctrl->cylinder_low |= (position >> 32 & 0xFF) << 8;
            ctrl->cylinder_high |= (position >> 40 & 0xFF) << 8;
            break;
        }
    }
}
static void ide_check_canary(struct ide_controller *ctrl)
{
    if (ctrl->canary_above != 0xDEADBEEF || ctrl->canary_below != 0xBEEFDEAD) {
        do {
            util_abort();
            abort();
        } while (0);
        ;
    }
}
static void ide_pio_store_byte(struct ide_controller *ctrl, int offset, uint8_t value)
{
    ctrl->pio_buffer[offset] = value;
}
static void ide_pio_store_word(struct ide_controller *ctrl, int offset, uint16_t value)
{
    ctrl->pio_buffer16[offset >> 1] = value;
}
static void ide_pio_store_word_be(struct ide_controller *ctrl, int offset, uint16_t value)
{
    ctrl->pio_buffer[offset]     = value >> 8;
    ctrl->pio_buffer[offset + 1] = value;
}
static void ide_pio_store_dword_be(struct ide_controller *ctrl, int offset, uint32_t value)
{
    ctrl->pio_buffer[offset]     = value >> 24;
    ctrl->pio_buffer[offset + 1] = value >> 16;
    ctrl->pio_buffer[offset + 2] = value >> 8;
    ctrl->pio_buffer[offset + 3] = value;
}
static void ide_pio_clear(struct ide_controller *ctrl, int offset, int length)
{
    for (int i = offset; i < (offset + length); i++)
        ctrl->pio_buffer[i] = 0;
}
static void ide_pio_store_string(struct ide_controller *ctrl, char *string, int pos, int length, int swap,
                                 int justify_left)
{
    char *buffer = (char *)__builtin_alloca(length + 1);
    if (justify_left) {
        sprintf(buffer, "%-*s", length, string);
    } else {
        sprintf(buffer, "%*s", length, string);
    }
    for (int i = 0; i < length; i++) {
        ide_pio_store_byte(ctrl, pos + (i ^ swap), buffer[i]);
    }
}
static inline uint16_t read16be(void *x)
{
    uint8_t *buf = (uint8_t *)x;
    return buf[0] << 8 | buf[1];
}
static inline uint32_t read32be(void *x)
{
    uint8_t *buf = (uint8_t *)x;
    return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}
static void ide_atapi_init_transfer(struct ide_controller *ctrl)
{
    ctrl->sector_count = (ctrl->sector_count & 0xF8) | 0x02;
    ctrl->status |= 0x08;
}
static void ide_atapi_init_command(struct ide_controller *ctrl)
{
    ctrl->sector_count = (ctrl->sector_count & 0xF8) | 0x01;
    ctrl->status |= 0x08;
}
static void ide_atapi_no_transfer(struct ide_controller *ctrl)
{
    ctrl->sector_count = (ctrl->sector_count & 0xF8) | 0x02 | 0x01;
    ctrl->status &= ~0x08;
}
static void ide_atapi_abort(struct ide_controller *ctrl, int sense_key, int asc)
{
    ctrl->error  = sense_key << 4;
    ctrl->status = 0x40 | 0x01;
    ide_atapi_no_transfer(ctrl);
    ctrl->sense_key = sense_key;
    ctrl->asc       = asc;
}
static void ide_atapi_start_transfer(struct ide_controller *ctrl, int size)
{
    ctrl->pio_position  = 0;
    ctrl->cylinder_low  = size;
    ctrl->cylinder_high = size >> 8;
    ctrl->pio_length    = size;
    ide_atapi_init_transfer(ctrl);
    ctrl->status = 0x40 | 0x10 | 0x08;
    if (ctrl->atapi_dma_enabled)
        do {
            util_abort();
            abort();
        } while (0);
    ;
    ide_raise_irq(ctrl);
}
static void ide_atapi_stop_command(struct ide_controller *ctrl)
{
    ctrl->pio_position = 0;
    ctrl->pio_length   = 0;
    ide_atapi_no_transfer(ctrl);
    ctrl->status = 0x40;
}
static void ide_atapi_read_complete(void *selfptr, int x)
{
    struct ide_controller *ctrl = (struct ide_controller *)selfptr;
    if (x == -1) {
        ide_atapi_abort(ctrl, 0x05, 0);
        do {
            util_abort();
            abort();
        } while (0);
        ;
    }
    ctrl->status &= ~0x80;
    ide_atapi_init_transfer(ctrl);
    ctrl->status |= 0x10 | 0x40;
    ctrl->atapi_sectors_to_read--;
    ctrl->atapi_lba++;
    ctrl->cylinder_low  = ctrl->atapi_bytes_to_transfer & 0xFF;
    ctrl->cylinder_high = ctrl->atapi_bytes_to_transfer >> 8 & 0xFF;
    ide_raise_irq(ctrl);
}
static void ide_atapi_read(struct ide_controller *ctrl)
{
    int res = drive_read(ctrl->info[ctrl->selected], ctrl, ctrl->pio_buffer, ctrl->atapi_sector_size,
                         ctrl->atapi_lba * ctrl->atapi_sector_size, ide_atapi_read_complete);
    if (res != 1) {
        printf(" == Internal IDE inconsistency == ");
        printf("Fetch offset: %08x [blk%08x.bin]\n", ctrl->atapi_lba * ctrl->atapi_sector_size,
               (ctrl->atapi_lba * ctrl->atapi_sector_size) / (256 << 10));
        printf("Fetch bytes: %d\n", ctrl->atapi_sector_size);
        do {
            util_abort();
            abort();
        } while (0);
        ;
    }
    ide_atapi_init_transfer(ctrl);
    ctrl->status |= 0x10 | 0x40;
    ctrl->atapi_sectors_to_read--;
    ctrl->atapi_lba++;
    uint32_t total_bytes = ctrl->atapi_cylinder_count;
    if (total_bytes > ctrl->atapi_bytes_to_transfer)
        total_bytes = ctrl->atapi_bytes_to_transfer;
    ctrl->atapi_frame_bytes_to_transfer = total_bytes;
    ctrl->atapi_frame_bytes_transferred = 0;
    ctrl->cylinder_low                  = total_bytes & 0xFF;
    ctrl->cylinder_high                 = total_bytes >> 8 & 0xFF;
    ctrl->pio_position                  = 0;
    ctrl->pio_length                    = total_bytes > ctrl->atapi_sector_size ? ctrl->atapi_sector_size : total_bytes;
    ide_raise_irq(ctrl);
}
static void ide_atapi_read_cb(void *selfptr, int stat)
{
    struct ide_controller *ctrl = (struct ide_controller *)selfptr;
    if (stat == -1) {
        do {
            util_abort();
            abort();
        } while (0);
        ;
    }
    ctrl->status &= ~0x80;
    ide_atapi_read(ctrl);
}
static void ide_atapi_run_command(struct ide_controller *ctrl)
{
    uint8_t command[12];
    for (int i = 0; i < 12; i++)
        command[i] = ctrl->pio_buffer[i];
    ctrl->atapi_command = command[0];
    int dont_xor        = -1;
    switch (command[0]) {
        case 0x00:
            if (ctrl->media_inserted[ctrl->selected]) {
                ide_atapi_stop_command(ctrl);
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
            }
            ide_raise_irq(ctrl);
            break;
        case 0x03:
            ide_pio_clear(ctrl, 0, 18);
            ide_pio_store_byte(ctrl, 0, 0xF0);
            ide_pio_store_byte(ctrl, 2, ctrl->sense_key);
            ide_pio_store_byte(ctrl, 7, 10);
            ide_pio_store_byte(ctrl, 12, ctrl->asc >> 8);
            ide_pio_store_byte(ctrl, 13, ctrl->asc);
            if (ctrl->sense_key == 6)
                ctrl->sense_key = 0;
            ide_atapi_start_transfer(ctrl, command[4] > 18 ? 18 : command[4]);
            break;
        case 0x12:
            ide_pio_store_byte(ctrl, 0, 0x05);
            ide_pio_store_byte(ctrl, 1, 0x80);
            ide_pio_store_byte(ctrl, 2, 0x00);
            ide_pio_store_byte(ctrl, 3, 0x21);
            ide_pio_store_byte(ctrl, 4, 0x1F);
            ide_pio_store_byte(ctrl, 5, 0x00);
            ide_pio_store_byte(ctrl, 6, 0x00);
            ide_pio_store_byte(ctrl, 7, 0x00);
            ide_pio_store_string(ctrl, (char *)"Halfix", 8, 8, 0, 1);
            ide_pio_store_string(ctrl, (char *)"Halfix CD-ROM", 16, 16, 0, 1);
            ide_pio_store_string(ctrl, (char *)"1.0", 24, 4, 0, 1);
            ide_atapi_start_transfer(ctrl, command[4] > 36 ? 36 : command[4]);
            break;
        case 0x1E:
            if (ctrl->media_inserted[ctrl->selected]) {
                ctrl->atapi_can_eject_cdrom = ~command[4] & 1;
                ide_atapi_stop_command(ctrl);
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
            }
            ide_raise_irq(ctrl);
            break;
        case 0x25:
            if (ctrl->media_inserted[ctrl->selected]) {
                ide_pio_store_dword_be(ctrl, 0, ctrl->total_sectors[ctrl->selected] - 1);
                ide_pio_store_dword_be(ctrl, 4, 2048);
                ide_atapi_start_transfer(ctrl, 8);
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
                ide_raise_irq(ctrl);
            }
            break;
        case 0x43: {
            int length = read16be(command + 7), nlength, format = command[9] >> 6, track_start = command[6], bufpos,
                sectors;
            ide_pio_clear(ctrl, 0, length);
            switch (format) {
                case 0:
                    ide_pio_store_byte(ctrl, 2, 1);
                    ide_pio_store_byte(ctrl, 3, 1);
                    bufpos = 4;
                    if (track_start < 2) {
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, 0x14);
                        ide_pio_store_byte(ctrl, bufpos++, 0x01);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, command[1] & 2);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                    }
                    ide_pio_store_byte(ctrl, bufpos++, 0);
                    ide_pio_store_byte(ctrl, bufpos++, 0x16);
                    ide_pio_store_byte(ctrl, bufpos++, 0xAA);
                    ide_pio_store_byte(ctrl, bufpos++, 0);
                    sectors = ctrl->total_sectors[ctrl->selected];
                    if (command[1] & 2) {
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, ((sectors + 150) / 75) / 60);
                        ide_pio_store_byte(ctrl, bufpos++, ((sectors + 150) / 75) % 60);
                        ide_pio_store_byte(ctrl, bufpos++, ((sectors + 150) % 75));
                    } else {
                        ide_pio_store_dword_be(ctrl, bufpos, sectors);
                        bufpos += 4;
                    }
                    ide_pio_store_word_be(ctrl, 0, bufpos - 2);
                    nlength = bufpos;
                    break;
                case 1:
                    nlength = 12;
                    ide_pio_store_word_be(ctrl, 0, 0x0A);
                    ide_pio_store_byte(ctrl, 2, 1);
                    ide_pio_store_byte(ctrl, 3, 1);
                    break;
                case 2:
                    ide_pio_store_byte(ctrl, 2, 1);
                    ide_pio_store_byte(ctrl, 3, 1);
                    bufpos = 4;
                    for (int i = 0; i < 4; i++) {
                        ide_pio_store_byte(ctrl, bufpos++, 0x01);
                        ide_pio_store_byte(ctrl, bufpos++, 0x14);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        if (i == 3)
                            ide_pio_store_byte(ctrl, bufpos++, 0xA3);
                        else
                            ide_pio_store_byte(ctrl, bufpos++, i);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        ide_pio_store_byte(ctrl, bufpos++, 0);
                        if (i & 2) {
                            sectors = ctrl->total_sectors[ctrl->selected];
                            if (command[1] & 2) {
                                ide_pio_store_byte(ctrl, bufpos++, 0);
                                ide_pio_store_byte(ctrl, bufpos++, ((sectors + 150) / 75) / 60);
                                ide_pio_store_byte(ctrl, bufpos++, ((sectors + 150) / 75) % 60);
                                ide_pio_store_byte(ctrl, bufpos++, ((sectors + 150) % 75));
                            } else {
                                ide_pio_store_dword_be(ctrl, bufpos, sectors);
                                bufpos += 4;
                            }
                        } else {
                            ide_pio_store_byte(ctrl, bufpos++, 0);
                            ide_pio_store_byte(ctrl, bufpos++, 1);
                            ide_pio_store_byte(ctrl, bufpos++, 0);
                            ide_pio_store_byte(ctrl, bufpos++, 0);
                        }
                    }
                    ide_pio_store_word_be(ctrl, 0, bufpos - 2);
                    nlength = bufpos;
                    break;
                case 3:
                    do {
                        util_abort();
                        abort();
                    } while (0);
                    ;
            }
            (void)(length);
            ide_atapi_start_transfer(ctrl, nlength);
            ide_raise_irq(ctrl);
            break;
        }
        case 0x1B:
            ide_atapi_no_transfer(ctrl);
            ide_raise_irq(ctrl);
            break;
        case 0x1A:
        case 0x5A: {
            int length, nlength;
            if (command[0] & 0x40)
                length = read16be(command + 6);
            else
                length = command[4];
            switch (command[2]) {
                case 1:
                    ide_pio_clear(ctrl, 0, nlength = 16);
                    ide_pio_store_word_be(ctrl, 0, 22);
                    ide_pio_store_byte(ctrl, 2, 0x70);
                    ide_pio_store_byte(ctrl, 8, 0x01);
                    ide_pio_store_byte(ctrl, 9, 0x06);
                    ide_pio_store_byte(ctrl, 11, 0x05);
                    break;
                case 0x2A:
                case (2 << 6) | 0x2A:
                    ide_pio_clear(ctrl, 0, nlength = 28);
                    ide_pio_store_word_be(ctrl, 0, 34);
                    ide_pio_store_byte(ctrl, 2, 0x70);
                    ide_pio_store_byte(ctrl, 8, 0x2A);
                    ide_pio_store_byte(ctrl, 9, 0x12);
                    ide_pio_store_byte(ctrl, 12, 0x70);
                    ide_pio_store_byte(ctrl, 13, 0x60);
                    ide_pio_store_byte(ctrl, 14, 41 | 0);
                    ide_pio_store_word_be(ctrl, 16, 706);
                    ide_pio_store_word_be(ctrl, 18, 2);
                    ide_pio_store_word_be(ctrl, 20, 512);
                    ide_pio_store_word_be(ctrl, 22, 706);
                    break;
                default:
                    ide_atapi_abort(ctrl, 0x05, 0x2400);
                    ide_raise_irq(ctrl);
                    return;
            }
            ide_atapi_start_transfer(ctrl, nlength < length ? nlength : length);
            break;
        }
        case 0x28:
        case 0xA8: {
            uint32_t sectors, lba, total_sectors, bytecount;
            if (command[0] & 0x80)
                sectors = read32be(command + 6);
            else
                sectors = read16be(command + 7);
            lba = read32be(command + 2);
            if (ctrl->media_inserted[ctrl->selected]) {
                total_sectors = ctrl->total_sectors[ctrl->selected];
                int tmp;
                if ((lba + sectors) >= total_sectors) {
                    tmp = total_sectors - lba + 1;
                    if (tmp < 0) {
                        ide_atapi_abort(ctrl, 0x02, 0x2000);
                        ide_raise_irq(ctrl);
                        break;
                    } else if (tmp == 0) {
                        ide_atapi_stop_command(ctrl);
                        ide_raise_irq(ctrl);
                        break;
                    } else
                        sectors = tmp - 1;
                }
                if (sectors == 0) {
                    ide_atapi_stop_command(ctrl);
                    break;
                }
                ctrl->atapi_lba               = lba;
                ctrl->atapi_sectors_to_read   = sectors;
                ctrl->atapi_sector_size       = 2048;
                ctrl->atapi_cylinder_count    = (ctrl->cylinder_high << 8 & 0xFF00) | (ctrl->cylinder_low & 0xFF);
                ctrl->atapi_bytes_to_transfer = bytecount = ctrl->atapi_sector_size * ctrl->atapi_sectors_to_read;
                ctrl->atapi_total_bytes_transferred       = 0;
                ctrl->cylinder_low                        = 0;
                ctrl->cylinder_high                       = 0;
                if (ctrl->atapi_cylinder_count & 1)
                    ctrl->atapi_cylinder_count--;
                int res = drive_prefetch(ctrl->info[ctrl->selected], ctrl, ctrl->atapi_bytes_to_transfer,
                                         ctrl->atapi_lba * ctrl->atapi_sector_size, ide_atapi_read_cb);
                if (res == 0) {
                    ctrl->status |= 0x80 | 0x40 | 0x10;
                } else if (res == 1) {
                    ide_atapi_read_cb(ctrl, 0);
                } else {
                    ide_atapi_abort(ctrl, 0x02, 0x3A02);
                    ide_raise_irq(ctrl);
                }
                dont_xor = 0;
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
                ide_raise_irq(ctrl);
            }
            break;
        }
        case 0x2B:
            if (ctrl->media_inserted[ctrl->selected]) {
                uint32_t lba = read32be(command + 2);
                if (lba >= ctrl->total_sectors[ctrl->selected]) {
                    ide_atapi_abort(ctrl, 0x05, 0x2100);
                    ide_raise_irq(ctrl);
                    break;
                }
                ide_atapi_stop_command(ctrl);
                ide_raise_irq(ctrl);
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
                ide_raise_irq(ctrl);
            }
            break;
        case 0x42:
            if (ctrl->media_inserted[ctrl->selected]) {
                int length = command[8] < 8 ? command[8] : 8;
                ide_pio_clear(ctrl, 0, length);
                ide_atapi_start_transfer(ctrl, length);
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
                ide_raise_irq(ctrl);
            }
            break;
        case 0x51:
            ide_atapi_abort(ctrl, 0x05, 36);
            break;
        case 0xBD: {
            int x = read16be(command + 8);
            ide_pio_clear(ctrl, 0, 8);
            ide_pio_store_byte(ctrl, 5, 1);
            ide_atapi_start_transfer(ctrl, x > 8 ? 8 : x);
            break;
        }
        case 0xBE:
            if (ctrl->media_inserted[ctrl->selected]) {
                uint32_t length = read32be(command + 5) & 0x00FFFFFF, lba = read32be(command + 2);
                if (length == 0) {
                    ide_atapi_stop_command(ctrl);
                    break;
                }
                (void)(lba);
                do {
                    util_abort();
                    abort();
                } while (0);
                ;
            } else {
                ide_atapi_abort(ctrl, 0x02, 0x3A02);
                ide_raise_irq(ctrl);
            }
            break;
        case 0x46:
        case 0x8D:
        case 0x55:
        case 0xa6:
        case 0x4b:
        case 0x45:
        case 0x47:
        case 0xbc:
        case 0xb9:
        case 0x44:
        case 0xba:
        case 0xbb:
        case 0x4e:
        case 0x4a:
            ide_atapi_abort(ctrl, 5, 0x2000);
            ide_raise_irq(ctrl);
            break;
        default:
            do {
                abort();
            } while (0);
    }
    if (!dont_xor)
        return;
    int bit = 0x02 & dont_xor;
    if (!(ctrl->status & 0x80)) {
        ide_raise_irq(ctrl);
        if (ctrl->pio_length == 0) {
            bit |= 0x01 & dont_xor;
            ctrl->status &= ~0x08;
        }
    }
    ctrl->sector_count &= 0xF8;
    ctrl->sector_count |= bit;
}
static void ide_pio_read_callback(struct ide_controller *ctrl)
{
    unsigned int old_pio = ctrl->pio_position;
    ctrl->pio_position   = 0;
    ctrl->status &= ~0x08;
    switch (ctrl->command_issued) {
        case 0xA0: {
            switch (ctrl->atapi_command) {
                case 0x28:
                case 0xA8: {
                    if (ctrl->pio_length != old_pio) {
                        do {
                            util_abort();
                            abort();
                        } while (0);
                        ;
                    }
                    ctrl->atapi_frame_bytes_transferred += ctrl->pio_length;
                    ctrl->atapi_total_bytes_transferred += ctrl->pio_length;
                    if (ctrl->atapi_frame_bytes_transferred >= ctrl->atapi_frame_bytes_to_transfer) {
                        ctrl->atapi_bytes_to_transfer -= ctrl->atapi_frame_bytes_transferred;
                        if (ctrl->atapi_bytes_to_transfer == 0) {
                            ctrl->cylinder_low                  = ctrl->atapi_frame_bytes_to_transfer & 0xFF;
                            ctrl->cylinder_high                 = ctrl->atapi_frame_bytes_to_transfer >> 8 & 0xFF;
                            ctrl->status                        = 0x40;
                            ctrl->atapi_frame_bytes_transferred = 1;
                            ide_atapi_stop_command(ctrl);
                            ide_raise_irq(ctrl);
                        } else {
                            ide_raise_irq(ctrl);
                            int continue_frame = 0;
                            if ((continue_frame = ctrl->atapi_total_bytes_transferred % ctrl->atapi_sector_size)) {
                                ctrl->atapi_lba--;
                                ctrl->atapi_sectors_to_read++;
                            }
                            ide_atapi_read(ctrl);
                            memmove(ctrl->pio_buffer, ctrl->pio_buffer + continue_frame,
                                    ctrl->atapi_sector_size - continue_frame);
                            ctrl->pio_position                  = 0;
                            ctrl->pio_length                    = ctrl->atapi_sector_size - continue_frame;
                            ctrl->atapi_frame_bytes_transferred = 0;
                        }
                    } else {
                        int res =
                            drive_read(ctrl->info[ctrl->selected], ctrl, ctrl->pio_buffer, ctrl->atapi_sector_size,
                                       ctrl->atapi_lba * ctrl->atapi_sector_size, ide_atapi_read_complete);
                        if (res != 1) {
                            do {
                                util_abort();
                                abort();
                            } while (0);
                            ;
                        }
                        unsigned int bytes_left =
                            ctrl->atapi_frame_bytes_to_transfer - ctrl->atapi_frame_bytes_transferred;
                        ctrl->pio_length = bytes_left > ctrl->atapi_sector_size ? ctrl->atapi_sector_size : bytes_left;
                        ide_atapi_init_transfer(ctrl);
                        ctrl->atapi_lba++;
                        ctrl->atapi_sectors_to_read--;
                    }
                    break;
                }
                    ctrl->atapi_bytes_to_transfer -= ctrl->pio_length;
                    if (ctrl->atapi_cylinder_count > ctrl->atapi_bytes_to_transfer)
                        ctrl->atapi_cylinder_count = ctrl->atapi_bytes_to_transfer;
                    if (ctrl->atapi_sectors_to_read == 0) {
                        ctrl->cylinder_low  = ctrl->atapi_cylinder_count & 0xFF;
                        ctrl->cylinder_high = ctrl->atapi_cylinder_count >> 8 & 0xFF;
                        ctrl->sector_count &= 0xF8;
                        ctrl->sector_count |= 3;
                        break;
                    }
                    ide_atapi_read(ctrl);
                    break;
                default:
                    ide_raise_irq(ctrl);
                    ctrl->sector_count |= 0x01;
                    ctrl->error = 0;
            }
            break;
        }
        case 0xEC:
        case 0xA1:
            break;
        case 0x29:
        case 0xC4:
        case 0x20:
        case 0x21:
        case 0x24:
            ide_raise_irq(ctrl);
            ide_check_canary(ctrl);
            if (ctrl->lba48 ? ctrl->sector_count != 0 : (ctrl->sector_count & 0xFF) != 0)
                ide_read_sectors(
                    ctrl, ctrl->lba48,
                    (ctrl->command_issued == 0x29 || ctrl->command_issued == 0xC4) ? ctrl->multiple_sectors_count : 1);
            else {
                ctrl->error  = 0;
                ctrl->status = 0x40 | 0x10;
            }
            break;
        case 0:
            break;
        default:
            do {
                util_abort();
                abort();
            } while (0);
            ;
    }
}
static void ide_pio_write_callback(struct ide_controller *ctrl)
{
    ctrl->pio_position = 0;
    switch (ctrl->command_issued) {
        case 0xA0:
            ide_atapi_run_command(ctrl);
            break;
        case 0x39:
        case 0xC5:
        case 0x30:
        case 0x31:
        case 0x34: {
            ide_raise_irq(ctrl);
            uint64_t sector_offset = ide_get_sector_offset(ctrl, ctrl->lba48);

            int res = drive_write(ctrl->info[ctrl->selected], ctrl, ctrl->pio_buffer, ctrl->sectors_read * 512,
                                  sector_offset * (uint64_t)512, drive_write_callback);
            if (res == 1)
                drive_write_callback(ctrl, 0);
            else if (res == 0) {
                ctrl->status = 0x10 | 0x40 | 0x80;
            } else
                ide_abort_command(ctrl);
            break;
        }
        default:
            do {
                util_abort();
                abort();
            } while (0);
            ;
    }
}
static uint32_t ide_pio_readb(uint32_t port)
{
    struct ide_controller *ctrl   = &ide[~port >> 7 & 1];
    uint8_t                result = ctrl->pio_buffer[ctrl->pio_position++];
    if (ctrl->pio_position >= ctrl->pio_length)
        ide_pio_read_callback(ctrl);
    return result;
}
static uint32_t ide_pio_readw(uint32_t port)
{
    struct ide_controller *ctrl = &ide[~port >> 7 & 1];
    if ((ctrl->pio_position | ctrl->pio_length) & 1) {
        uint32_t res = ide_pio_readb(port);
        res |= ide_pio_readb(port) << 8;
        return res;
    }
    uint32_t result = ctrl->pio_buffer16[ctrl->pio_position >> 1];
    ctrl->pio_position += 2;
    if (ctrl->pio_position >= ctrl->pio_length)
        ide_pio_read_callback(ctrl);
    return result;
}
static uint32_t ide_pio_readd(uint32_t port)
{
    struct ide_controller *ctrl = &ide[~port >> 7 & 1];
    if ((ctrl->pio_position | ctrl->pio_length) & 3) {
        uint32_t res = ide_pio_readb(port);
        res |= ide_pio_readb(port) << 8;
        res |= ide_pio_readb(port) << 16;
        res |= ide_pio_readb(port) << 24;
        return res;
    }
    uint32_t result = ctrl->pio_buffer32[ctrl->pio_position >> 2];
    ctrl->pio_position += 4;
    if (ctrl->pio_position >= ctrl->pio_length)
        ide_pio_read_callback(ctrl);
    return result;
}
static void ide_pio_writeb(uint32_t port, uint32_t data)
{
    struct ide_controller *ctrl            = &ide[~port >> 7 & 1];
    ctrl->pio_buffer[ctrl->pio_position++] = data;
    if (ctrl->pio_position >= ctrl->pio_length)
        ide_pio_write_callback(ctrl);
}
static void ide_pio_writew(uint32_t port, uint32_t data)
{
    struct ide_controller *ctrl = &ide[~port >> 7 & 1];
    if ((ctrl->pio_position | ctrl->pio_length) & 1) {
        ide_pio_writeb(port, data & 0xFF);
        ide_pio_writeb(port, data >> 8 & 0xFF);
        return;
    }
    ctrl->pio_buffer16[ctrl->pio_position >> 1] = data;
    ctrl->pio_position += 2;
    if (ctrl->pio_position >= ctrl->pio_length)
        ide_pio_write_callback(ctrl);
}
static void ide_pio_writed(uint32_t port, uint32_t data)
{
    struct ide_controller *ctrl = &ide[~port >> 7 & 1];
    if ((ctrl->pio_position | ctrl->pio_length) & 3) {
        ide_pio_writeb(port, data & 0xFF);
        ide_pio_writeb(port, data >> 8 & 0xFF);
        ide_pio_writeb(port, data >> 16 & 0xFF);
        ide_pio_writeb(port, data >> 24 & 0xFF);
        return;
    }
    ctrl->pio_buffer32[ctrl->pio_position >> 2] = data;
    ctrl->pio_position += 4;
    if (ctrl->pio_position >= ctrl->pio_length)
        ide_pio_write_callback(ctrl);
}
static void ide_set_signature(struct ide_controller *ctrl)
{
    ctrl->drive_and_head &= 15;
    ctrl->sector_number = 1;
    ctrl->sector_count  = 1;
    switch (ctrl->type[ctrl->selected]) {
        case DRIVE_TYPE_NONE:
            ctrl->cylinder_low  = 0xFF;
            ctrl->cylinder_high = 0xFF;
            break;
        case DRIVE_TYPE_DISK:
            ctrl->cylinder_low  = 0;
            ctrl->cylinder_high = 0;
            break;
        case DRIVE_TYPE_CDROM:
            ctrl->cylinder_low  = 0x14;
            ctrl->cylinder_high = 0xEB;
            break;
    }
}
static uint32_t ide_read(uint32_t port)
{
    struct ide_controller *ctrl = &ide[~port >> 7 & 1];
    (void)(ctrl);
    switch (port | 0x80) {
        case 0x1F1:
            return ctrl->error;
        case 0x1F2:
            return ctrl->sector_count;
        case 0x1F3:
            return ctrl->sector_number;
        case 0x1F4:
            return ctrl->cylinder_low;
        case 0x1F5:
            return ctrl->cylinder_high;
        case 0x1F6:
            return ctrl->drive_and_head;
        case 0x1F7:
            if (selected_drive_has_media(ctrl) || ctrl->device_control & 4) {
                ide_lower_irq(ctrl);
                return ctrl->status;
            }
            return 0;
        case 0x3F6:
            return -selected_drive_has_media(ctrl) & ctrl->status;
        default:
            do {
                util_abort();
                abort();
            } while (0);
            ;
    }
}
static void ide_update_head(struct ide_controller *ctrl)
{
    ctrl->lba      = ctrl->drive_and_head >> 6 & 1;
    ctrl->selected = ctrl->drive_and_head >> 4 & 1;
}
static void ide_read_sectors_callback(void *self, int result)
{
    if (result < 0)
        ide_abort_command((struct ide_controller *)self);
    struct ide_controller *ctrl = (struct ide_controller *)self;
    ctrl->sector_count -= ctrl->sectors_read;
    ctrl->error = 0;
    ide_set_sector_offset(ctrl, ctrl->lba48, ide_get_sector_offset(ctrl, ctrl->lba48) + ctrl->sectors_read);
    ctrl->status       = 0x40 | 0x10 | 0x08;
    ctrl->pio_length   = ctrl->sectors_read * 512;
    ctrl->pio_position = 0;
    ide_raise_irq(ctrl);
}
static void ide_read_sectors(struct ide_controller *ctrl, int lba48, int chunk_count)
{
    ctrl->lba48            = lba48;
    uint32_t sector_count  = ide_get_sector_count(ctrl, lba48);
    uint64_t sector_offset = ide_get_sector_offset(ctrl, lba48);
    ctrl->status           = 0x40;
    int sectors_to_read    = chunk_count;
    if (sector_count < (unsigned int)chunk_count)
        sectors_to_read = sector_count;
    ctrl->sectors_read = sectors_to_read;
    int res            = drive_read(ctrl->info[ctrl->selected], ctrl, ctrl->pio_buffer, sectors_to_read * 512,
                                    sector_offset * (uint64_t)512, ide_read_sectors_callback);
    if (res < 0)
        ide_abort_command(ctrl);
    else if (res == 0)
        ctrl->status = 0x40 | 0x10 | 0x80;
    else
        ide_read_sectors_callback(ctrl, 0);
}
static void ide_write_sectors(struct ide_controller *ctrl, int lba48, int chunk_count)
{
    ctrl->lba48               = lba48;
    uint32_t sector_count     = ide_get_sector_count(ctrl, ctrl->lba48);
    int      sectors_to_write = chunk_count;
    if (sector_count < (unsigned int)chunk_count)
        sectors_to_write = sector_count;
    ctrl->sectors_read = sectors_to_write;
    ctrl->error        = 0;
    ctrl->status       = 0x10 | 0x40 | 0x08;
    ctrl->pio_position = 0;
    ctrl->pio_length   = ctrl->sectors_read * 512;
}
static void drive_write_callback(void *self, int result)
{
    if (result < 0)
        ide_abort_command((struct ide_controller *)self);
    struct ide_controller *ctrl = (struct ide_controller *)self;
    ctrl->sector_count -= ctrl->sectors_read;
    ide_set_sector_offset(ctrl, ctrl->lba48, ide_get_sector_offset(ctrl, ctrl->lba48) + ctrl->sectors_read);
    if (ctrl->lba48 ? ctrl->sector_count != 0 : (ctrl->sector_count & 0xFF) != 0)
        ide_write_sectors(ctrl, ctrl->lba48,
                          (ctrl->command_issued == 0x39 || ctrl->command_issued == 0xC5) ? ctrl->multiple_sectors_count
                                                                                         : 1);
    else {
        ctrl->error  = 0;
        ctrl->status = 0x40 | 0x10;
    }
}
static void ide_identify(struct ide_controller *ctrl)
{
    int cdrom = ctrl->type[ctrl->selected] == DRIVE_TYPE_CDROM;
    if (cdrom) {
        ide_pio_clear(ctrl, 0, 512);
        ide_pio_store_word(ctrl, 0, 0x85C0);
        ide_pio_store_string(ctrl, (char *)"HFXCD 000000", 10 << 1, 20, 1, 1);
        ide_pio_store_string(ctrl, (char *)"0.0.1", 23 << 1, 8, 1, 1);
        ide_pio_store_string(ctrl, (char *)"Halfix CD-ROM drive", 27 << 1, 40, 1, 1);
        ide_pio_store_word(ctrl, 48 << 1, 1);
        int v = 512;
        if (ctrl->dma_enabled)
            v |= 128;
        ide_pio_store_word(ctrl, 48 << 1, v);
        ide_pio_store_word(ctrl, 53 << 1, 3);
        v = 0;
        if (ctrl->dma_enabled)
            v |= 7 | ctrl->mdma;
        ide_pio_store_word(ctrl, 63 << 1, v);
        ide_pio_store_word(ctrl, 64 << 1, 1);
        ide_pio_store_word(ctrl, 65 << 1, 0xB4);
        ide_pio_store_word(ctrl, 66 << 1, 0xB4);
        ide_pio_store_word(ctrl, 67 << 1, 0x12C);
        ide_pio_store_word(ctrl, 68 << 1, 0xB4);
        ide_pio_store_word(ctrl, 71 << 1, 30);
        ide_pio_store_word(ctrl, 72 << 1, 30);
        ide_pio_store_word(ctrl, 80 << 1, 0x1E);
    } else {
        ide_pio_store_byte(ctrl, (0 << 1) | 0, 0x40);
        ide_pio_store_byte(ctrl, (0 << 1) | 1, -cdrom & 0x85);
        ide_pio_store_word(ctrl, 1 << 1, ctrl->cylinders[ctrl->selected << 1]);
        ide_pio_store_word(ctrl, 2 << 1, 0);
        ide_pio_store_word(ctrl, 3 << 1, ctrl->heads[ctrl->selected << 1]);
        ide_pio_store_word(ctrl, 4 << 1, ctrl->sectors_per_track[ctrl->selected << 1] * 512);
        ide_pio_store_word(ctrl, 5 << 1, 512);
        ide_pio_store_word(ctrl, 6 << 1, ctrl->sectors_per_track[ctrl->selected << 1]);
        ide_pio_store_word(ctrl, 7 << 1, 0);
        ide_pio_store_word(ctrl, 8 << 1, 0);
        ide_pio_store_word(ctrl, 9 << 1, 0);
        ide_pio_store_string(ctrl, (char *)"HFXHD 000000", 10 << 1, 20, 1, 0);
        ide_pio_store_word(ctrl, 20 << 1, 3);
        ide_pio_store_word(ctrl, 21 << 1, 16 * 512 / 512);
        ide_pio_store_word(ctrl, 22 << 1, 4);
        ide_pio_store_word(ctrl, 23 << 1, 4);
        ide_pio_store_word(ctrl, 24 << 1, 4);
        ide_pio_store_word(ctrl, 25 << 1, 4);
        ide_pio_store_word(ctrl, 26 << 1, 4);
        ide_pio_store_string(ctrl, (char *)"HALFIX 123456", 27 << 1, 40, 1, 1);
        ide_pio_store_word(ctrl, 47 << 1, 1);
        ide_pio_store_word(ctrl, 48 << 1, 1);
        ide_pio_store_word(ctrl, 49 << 1, 1 << 9);
        ide_pio_store_word(ctrl, 50 << 1, 0);
        ide_pio_store_word(ctrl, 51 << 1, 0x200);
        ide_pio_store_word(ctrl, 52 << 1, 0x200 | (ctrl->dma_enabled ? 0x100 : 0));
        ide_pio_store_word(ctrl, 53 << 1, 7);
        ide_pio_store_word(ctrl, 54 << 1, ctrl->cylinders[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])]);
        ide_pio_store_word(ctrl, 55 << 1, ctrl->heads[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])]);
        ide_pio_store_word(ctrl, 56 << 1,
                           ctrl->sectors_per_track[(ctrl->selected << 1) | (ctrl->translated[ctrl->selected])]);
        ide_pio_store_word(ctrl, 57 << 1, ctrl->total_sectors_chs[ctrl->selected] & 0xFFFF);
        ide_pio_store_word(ctrl, 58 << 1, ctrl->total_sectors_chs[ctrl->selected] >> 16 & 0xFFFF);
        int multiple_sector_mask = -(ctrl->multiple_sectors_count != 0);
        ide_pio_store_word(ctrl, 59 << 1, multiple_sector_mask & (0x100 | ctrl->multiple_sectors_count));
        ide_pio_store_word(ctrl, 60 << 1, ctrl->total_sectors[ctrl->selected] & 0xFFFF);
        ide_pio_store_word(ctrl, 61 << 1, ctrl->total_sectors[ctrl->selected] >> 16 & 0xFFFF);
        ide_pio_store_word(ctrl, 62 << 1, 0);
        ide_pio_store_word(ctrl, 63 << 1, 7 | ctrl->mdma);
        for (int i = 64; i < 65; i++)
            ide_pio_store_word(ctrl, i << 1, 0);
        for (int i = 65; i < 69; i++)
            ide_pio_store_word(ctrl, i << 1, 0x78);
        for (int i = 69; i < 80; i++)
            ide_pio_store_word(ctrl, i << 1, 0);
        ide_pio_store_word(ctrl, 80 << 1, 0x7E);
        ide_pio_store_word(ctrl, 81 << 1, 0);
        ide_pio_store_word(ctrl, 82 << 1, 1 << 14);
        ide_pio_store_word(ctrl, 83 << 1, (1 << 14) | (1 << 13) | (1 << 12));
        ide_pio_store_word(ctrl, 84 << 1, 1 << 14);
        ide_pio_store_word(ctrl, 85 << 1, 1 << 14);
        ide_pio_store_word(ctrl, 86 << 1, (1 << 14) | (1 << 13) | (1 << 12));
        ide_pio_store_word(ctrl, 87 << 1, 1 << 14);
        ide_pio_store_word(ctrl, 88 << 1, -(ctrl->dma_enabled != 0) & (0x3F | ctrl->udma));
        for (int i = 89; i < 93; i++)
            ide_pio_store_word(ctrl, i << 1, 0);
        ide_pio_store_word(ctrl, 93 << 1, 24577);
        for (int i = 94; i < 100; i++)
            ide_pio_store_word(ctrl, i << 1, 0);
        ide_pio_store_word(ctrl, 100 << 1, ctrl->total_sectors[ctrl->selected] & 0xFFFF);
        ide_pio_store_word(ctrl, 101 << 1, ctrl->total_sectors[ctrl->selected] >> 16 & 0xFFFF);
        ide_pio_store_word(ctrl, 102 << 1, 0);
        ide_pio_store_word(ctrl, 103 << 1, 0);
    }
    ctrl->pio_length   = 512;
    ctrl->pio_position = 0;
}
static void ide_read_dma_handler(void *self, int status)
{
    struct ide_controller *ctrl = (struct ide_controller *)self;
    (void)(status);
    uint32_t prdt_addr = ctrl->prdt_address, sectors = ide_get_sector_count(ctrl, ctrl->lba48),
             bytes_in_buffer  = sectors * 512;
    uint64_t           offset = ide_get_sector_offset(ctrl, ctrl->lba48) * 512ULL;
    struct drive_info *drv    = ctrl->info[ctrl->selected];
    void              *temp   = __builtin_alloca(65536);
    while (1) {
        uint32_t dest = cpu.cpu_read_phys(prdt_addr), other_stuff = cpu.cpu_read_phys(prdt_addr + 4),
                 count = other_stuff & 0xFFFF, end = other_stuff & 0x80000000;
        count |= !count << 16;
        uint32_t dma_bytes = count;
        if (dma_bytes > bytes_in_buffer)
            dma_bytes = bytes_in_buffer;
        {
            int count_rounded = ((count + 0xFFF) >> 12) << 12;
            for (int i = 0; i < count_rounded; i += 4096)
                cpu.cpu_init_dma(dest + i);
        }
        while (dma_bytes >= 512) {
            int res = drive_read(drv, __null, temp, 512, offset, __null);
            if (res != 1)
                do {
                    util_abort();
                    abort();
                } while (0);
            ;
            cpu.cpu_write_mem(dest, temp, 512);
            dma_bytes -= 512;
            dest += 512;
            offset += 512;
        }
        bytes_in_buffer -= dma_bytes;
        offset += dma_bytes;
        prdt_addr += 8;
        if (!bytes_in_buffer || end)
            break;
    }
    ctrl->status = 0x40 | 0x10;
    ctrl->dma_status &= ~1;
    ctrl->dma_status |= 4;
    ide_set_sector_offset(ctrl, ctrl->lba48, ide_get_sector_offset(ctrl, ctrl->lba48) + sectors);
    ide_raise_irq(ctrl);
}
void drive_debug(int64_t x)
{
    uint32_t offset = x & 511;
    uint8_t  buf[512];
    int      res = drive_read((&ide[0])->info[(&ide[0])->selected], __null, buf, 512, x & ~511, __null);
    for (int i = 0; i < 16; i++) {
        printf("%02x ", buf[offset]);
        offset++;
    }
    printf("\n");
}
static void ide_write_dma_handler(void *self, int status)
{
    struct ide_controller *ctrl = (struct ide_controller *)self;
    (void)(status);
    uint32_t prdt_addr = ctrl->prdt_address, sectors = ide_get_sector_count(ctrl, ctrl->lba48),
             bytes_in_buffer  = sectors * 512;
    uint64_t           offset = ide_get_sector_offset(ctrl, ctrl->lba48) * 512ULL;
    struct drive_info *drv    = ctrl->info[ctrl->selected];
    void              *mem    = cpu.cpu_get_ram_ptr();
    while (1) {
        uint32_t dest = cpu.cpu_read_phys(prdt_addr), other_stuff = cpu.cpu_read_phys(prdt_addr + 4),
                 count = other_stuff & 0xFFFF, end = other_stuff & 0x80000000;
        count |= !count << 16;
        uint32_t dma_bytes = count;
        if (dma_bytes > bytes_in_buffer)
            dma_bytes = bytes_in_buffer;
        while (dma_bytes >= 512) {
            int res = drive_write(drv, __null, (uint8_t *)mem + dest, 512, offset, __null);
            if (res != 1)
                do {
                    util_abort();
                    abort();
                } while (0);
            ;
            dma_bytes -= 512;
            dest += 512;
            offset += 512;
        }
        bytes_in_buffer -= dma_bytes;
        offset += dma_bytes;
        prdt_addr += 8;
        if (!bytes_in_buffer || end)
            break;
    }
    ctrl->status = 0x40 | 0x10;
    ctrl->dma_status &= ~1;
    ctrl->dma_status |= 4;
    ide_set_sector_offset(ctrl, ctrl->lba48, ide_get_sector_offset(ctrl, ctrl->lba48) + sectors);
    ide_raise_irq(ctrl);
}
static void ide_read_dma(struct ide_controller *ctrl, int lba48)
{
    ctrl->status = 0x10 | 0x08 | 0x40;
    ctrl->dma_status |= 1;
    ctrl->lba48 = lba48;
}
static void ide_write_dma(struct ide_controller *ctrl, int lba48)
{
    ctrl->status = 0x10 | 0x08 | 0x40;
    ctrl->dma_status |= 1;
    ctrl->lba48 = lba48;
}
static void ide_write(uint32_t port, uint32_t data)
{
    struct ide_controller *ctrl           = &ide[~port >> 7 & 1];
    int                    ctrl_has_media = -controller_has_media(ctrl);
    switch (port | 0x80) {
        case 0x1F1:
            ctrl->feature = ctrl_has_media & data;
            break;
        case 0x1F2:
            ctrl->sector_count = ctrl_has_media & (ctrl->sector_count << 8 | data);
            break;
        case 0x1F3:
            ctrl->sector_number = ctrl_has_media & (ctrl->sector_number << 8 | data);
            break;
        case 0x1F4:
            ctrl->cylinder_low = ctrl_has_media & (ctrl->cylinder_low << 8 | data);
            break;
        case 0x1F5:
            ctrl->cylinder_high = ctrl_has_media & (ctrl->cylinder_high << 8 | data);
            break;
        case 0x1F6:
            if (ctrl_has_media) {
                ctrl->drive_and_head = data;
                ide_update_head(ctrl);
            }
            break;
        case 0x1F7: {
            if (ctrl->selected == 1 && !selected_drive_has_media(ctrl))
                break;
            ide_lower_irq(ctrl);
            if (ctrl->status & 0x80) {
                break;
            }
            ctrl->status &= ~0x01;
            ctrl->command_issued = data;
            switch (data) {
                case 8:
                    if (ctrl->type[ctrl->selected] == DRIVE_TYPE_CDROM) {
                        ctrl->error &= ~0x80;
                        ctrl->error  = 0x01;
                        ctrl->status = 0;
                        ide_set_signature(ctrl);
                    } else
                        ide_abort_command(ctrl);
                    break;
                case 0x10 ... 0x1F:
                    if (ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    if (!selected_drive_has_media(ctrl)) {
                        ctrl->error  = 0x02;
                        ctrl->status = 0x40 | 0x01;
                    } else {
                        ctrl->cylinder_low  = 0;
                        ctrl->cylinder_high = 0;
                        ctrl->error         = 0;
                        ctrl->status        = 0x40 | 0x10;
                    }
                    ide_raise_irq(ctrl);
                    break;
                case 0x25:
                case 0xC8:
                    ide_read_dma(ctrl, data == 0x25);
                    break;
                case 0x35:
                case 0xCA:
                    ide_write_dma(ctrl, data == 0x35);
                    break;
                case 0x29:
                case 0xC4:
                    if (ctrl->multiple_sectors_count == 0 || ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK) {
                        ide_abort_command(ctrl);
                    } else
                        ide_read_sectors(ctrl, data == 0x29, ctrl->multiple_sectors_count);
                    break;
                case 0x20:
                case 0x21:
                case 0x24:
                    if (ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    else
                        ide_read_sectors(ctrl, data == 0x24, 1);
                    break;
                case 0x39:
                case 0xC5:
                    if (ctrl->multiple_sectors_count == 0 || ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    else
                        ide_write_sectors(ctrl, data == 0x39, ctrl->multiple_sectors_count);
                    break;
                case 0x30:
                case 0x31:
                case 0x34:
                    if (ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    else
                        ide_write_sectors(ctrl, data == 0x34, 1);
                    break;
                case 0x40:
                case 0x41:
                case 0x42:
                    if (ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    else {
                        int lba48 = data == 0x42;
                        ide_set_sector_offset(ctrl, lba48,
                                              ide_get_sector_offset(ctrl, lba48) +
                                                  (uint64_t)((ctrl->sector_count & 0xFF) - 1));
                        ide->status = 0x40;
                        ide_raise_irq(ctrl);
                    }
                    break;
                case 0x91:
                    if (ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    else {
                        if (selected_drive_has_media(ctrl)) {
                            int sectors = ctrl->sector_count & 0xFF, heads = (ctrl->drive_and_head & 0x0F) + 1;
                            if (heads != 1) {
                                ctrl->sectors_per_track[(ctrl->selected << 1) | 1] = sectors;
                                ctrl->heads[(ctrl->selected << 1) | 1]             = heads;
                                ctrl->cylinders[(ctrl->selected << 1) | 1] =
                                    ctrl->total_sectors[ctrl->selected] / (sectors * heads);
                                ctrl->selected = 1;
                            }
                        }
                        ctrl->status = 0x40 | 0x10;
                        ide_raise_irq(ctrl);
                    }
                    break;
                case 0xE5:
                    ctrl->status       = 0x40;
                    ctrl->sector_count = 255;
                    ide_raise_irq(ctrl);
                    break;
                case 0xE0:
                case 0xE1:
                case 0xE7:
                case 0xEA:
                    ctrl->status = 0x40;
                    ide_raise_irq(ctrl);
                    break;
                case 0xA0:
                    if (ctrl->type[ctrl->selected] == DRIVE_TYPE_CDROM) {
                        if (ctrl->feature & 2) {
                            ide_abort_command(ctrl);
                            break;
                        }
                        ide_atapi_init_command(ctrl);
                        ctrl->status &= ~(0x80 | 0x20);
                        ctrl->atapi_dma_enabled = ctrl->feature & 1;
                        ctrl->pio_length        = 12;
                        ctrl->pio_position      = 0;
                    } else
                        ide_abort_command(ctrl);
                    break;
                case 0xA1:
                    if (ctrl->type[ctrl->selected] == DRIVE_TYPE_CDROM) {
                        ctrl->error  = 0;
                        ctrl->status = 0x40 | 0x08 | 0x10;
                        ide_identify(ctrl);
                        ide_raise_irq(ctrl);
                    } else
                        ide_abort_command(ctrl);
                    break;
                case 0xEC:
                    if (!selected_drive_has_media(ctrl)) {
                        ide_abort_command(ctrl);
                    } else if (ctrl->type[ctrl->selected] == DRIVE_TYPE_CDROM) {
                        ide_set_signature(ctrl);
                        ide_abort_command(ctrl);
                    } else {
                        ide_identify(ctrl);
                        ctrl->error  = 0;
                        ctrl->status = 0x40 | 0x08 | 0x10;
                        ide_raise_irq(ctrl);
                    }
                    break;
                case 0xEF:
                    switch (ctrl->feature) {
                        case 3:
                            ide_abort_command(ctrl);
                            break;
                        case 2:
                        case 130:
                        case 0x66:
                        case 0x95:
                        case 0xAA:
                        case 0:
                            ctrl->status = 0x10 | 0x40;
                            ide_raise_irq(ctrl);
                            break;
                        default:
                            do {
                                util_abort();
                                abort();
                            } while (0);
                            ;
                    }
                    break;
                case 0xC6:
                    if (!selected_drive_has_media(ctrl))
                        ide_abort_command(ctrl);
                    else if (ctrl->type[ctrl->selected] == DRIVE_TYPE_CDROM)
                        ide_abort_command(ctrl);
                    else {
                        int multiple_count = ctrl->sector_count & 0xFF;
                        if (multiple_count > 1 || (multiple_count & (multiple_count - 1)) != 0) {
                            ide_abort_command(ctrl);
                        } else {
                            ctrl->multiple_sectors_count = multiple_count;
                            ctrl->status                 = 0x40;
                            ide_raise_irq(ctrl);
                        }
                    }
                    break;
                case 0xF8: {
                    if (ctrl->type[ctrl->selected] != DRIVE_TYPE_DISK)
                        ide_abort_command(ctrl);
                    uint32_t multiple_count = ctrl->info[ctrl->selected]->sectors - 1;
                    ide_set_sector_offset(ctrl, 0, multiple_count);
                    break;
                }
                case 0xF9:
                case 0x2F:
                case 0xF0:
                case 0xF5:
                case 0xDA:
                case 0xDE:
                    ide_abort_command(ctrl);
                    break;
                default:
                    do {
                        util_abort();
                        abort();
                    } while (0);
                    ;
            }
            break;
        }
        case 0x3F6:
            if ((ctrl->device_control ^ data) & 4) {
                if (data & 4)
                    ctrl->status |= 0x80;
                else {
                    ctrl->status = 0x40;
                    ctrl->error  = 1;
                    ide_set_signature(ctrl);
                    ctrl->selected = 0;
                    drive_cancel_transfers();
                }
            }
            ctrl->device_control = data;
            ide_update_irq(ctrl);
            break;
        default:
            do {
                util_abort();
                abort();
            } while (0);
            ;
    }
}
void ide_write_prdt(uint32_t addr, uint32_t data)
{
    struct ide_controller *self = &ide[addr >> 3 & 1];
    switch (addr & 7) {
        case 0: {
            uint8_t diffxor = self->dma_command ^ data;
            if (diffxor & 1) {
                self->dma_command = data & 9;
                int lba48         = self->lba48, result;
                if ((data & 1) == 0)
                    return;
                switch (self->command_issued) {
                    case 0x25:
                    case 0xC8:
                        result =
                            drive_prefetch(self->info[self->selected], self, ide_get_sector_count(self, lba48),
                                           ide_get_sector_offset(self, lba48) << (drv_offset_t)9, ide_read_dma_handler);
                        if (result == 1)
                            ide_read_dma_handler(self, 0);
                        else
                            self->status |= 0x80;
                        break;
                    case 0x35:
                    case 0xCA:
                        result = drive_prefetch(self->info[self->selected], self, ide_get_sector_count(self, lba48),
                                                ide_get_sector_offset(self, lba48) << (drv_offset_t)9,
                                                ide_write_dma_handler);
                        if (result == 1)
                            ide_write_dma_handler(self, 0);
                        else
                            self->status |= 0x80;
                        break;
                }
            }
            break;
        }
        case 2:
            self->dma_status &= ~(data & 6);
            break;
        case 4 ... 7: {
            int shift = ((addr & 3) << 3);
            self->prdt_address &= ~(0xFF << shift);
            self->prdt_address |= data << shift;
            break;
        }
        default:
            do {
                util_abort();
                abort();
            } while (0);
            ;
    }
}
uint32_t ide_read_prdt(uint32_t addr)
{
    struct ide_controller *self = &ide[addr >> 3 & 1];
    switch (addr & 7) {
        case 0:
            return self->dma_command;
        case 2:
            return self->dma_status;
        case 1:
        case 3:
            return 0;
        case 4:
            return self->prdt_address >> 0 & 0xFF;
        case 5:
            return self->prdt_address >> 8 & 0xFF;
        case 6:
            return self->prdt_address >> 16 & 0xFF;
        case 7:
            return self->prdt_address >> 24 & 0xFF;
    }
    return 0;
}
void ide_init(struct pc_settings *pc)
{
    io_register_reset(ide_reset);
    state_register(ide_state);
    io_register_read(0x1F0, 1, ide_pio_readb, ide_pio_readw, ide_pio_readd);
    io_register_write(0x1F0, 1, ide_pio_writeb, ide_pio_writew, ide_pio_writed);
    io_register_read(0x170, 1, ide_pio_readb, ide_pio_readw, ide_pio_readd);
    io_register_write(0x170, 1, ide_pio_writeb, ide_pio_writew, ide_pio_writed);
    io_register_read(0x1F1, 7, ide_read, __null, __null);
    io_register_read(0x171, 7, ide_read, __null, __null);
    io_register_write(0x1F1, 7, ide_write, __null, __null);
    io_register_write(0x171, 7, ide_write, __null, __null);
    io_register_read(0x376, 1, ide_read, __null, __null);
    io_register_read(0x3F6, 1, ide_read, __null, __null);
    io_register_write(0x376, 1, ide_write, __null, __null);
    io_register_write(0x3F6, 1, ide_write, __null, __null);
    for (int i = 0; i < 4; i++) {
        struct drive_info     *info = &pc->drives[i];
        struct ide_controller *ctrl = &ide[i >> 1];
        ctrl->canary_above          = 0xDEADBEEF;
        ctrl->canary_below          = 0xBEEFDEAD;
        int drive_id                = i & 1;
        ctrl->info[drive_id]        = info;
        ctrl->dma_enabled           = pc->pci_enabled;
        if (info->sectors != 0) {
            printf("Initializing disk %d\n", i);
            ctrl->dma_status |= 0x20 << (i & 1);
            ctrl->sectors_per_track[drive_id << 1] = info->sectors_per_cylinder;
            ctrl->heads[drive_id << 1]             = info->heads;
            ctrl->cylinders[drive_id << 1]         = info->cylinders_per_head;
            ctrl->media_inserted[drive_id]         = 1;
            ctrl->total_sectors_chs[drive_id] = info->cylinders_per_head * info->heads * info->sectors_per_cylinder;
            ctrl->total_sectors[drive_id]     = info->type == DRIVE_TYPE_CDROM ? info->sectors >> 2 : info->sectors;
        } else {
            ctrl->media_inserted[drive_id] = 0;
        }
        ctrl->type[drive_id] = info->type;
    }
}
