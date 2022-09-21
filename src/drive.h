#ifndef DRIVE_H
#define DRIVE_H
#include "state.h"
#include <stdint.h>

typedef uint64_t drv_offset_t;

#define DRIVE_LOG(x, ...) LOG("DRIVE", x, ##__VA_ARGS__)
#define DRIVE_FATAL(x, ...)                                                                                            \
    do {                                                                                                               \
        fprintf(stderr, x, ##__VA_ARGS__);                                                                             \
        abort();                                                                                                       \
    } while (0)
#define DRIVE_RESULT_ASYNC 0
#define DRIVE_RESULT_SYNC  1


typedef void (*drive_cb)(void *, int);
typedef int (*drive_read_func)(void *self, void *cb_ptr, void *buffer, uint32_t size, drv_offset_t offset, drive_cb cb);
typedef int (*drive_write_func)(void *self, void *cb_ptr, void *buffer, uint32_t size, drv_offset_t offset,
                                drive_cb cb);
typedef int (*drive_prefetch_func)(void *self, void *cb_ptr, uint32_t size, drv_offset_t offset, drive_cb cb);

struct drive_info
{
    int                 type;
    uint32_t            cylinders_per_head;
    uint32_t            heads;
    uint32_t            sectors_per_cylinder;
    uint32_t            sectors;
    int                 modify_backing_file;
    void               *data;
    int                 driver;
    drive_read_func     read;
    drive_write_func    write;
    drive_prefetch_func prefetch;
    void (*state)(void *self, char *path);
};


void drive_state(struct drive_info *info, char *filename);
int  drive_autodetect_type(char *path);
int  drive_init(struct drive_info *info, char *path);
int  drive_sync_init(struct drive_info *info, char *path);
int  drive_async_init(struct drive_info *info, char *path);
int  drive_simple_init(struct drive_info *info, char *path);
int  drive_read(struct drive_info *, void *, void *, uint32_t, drv_offset_t, drive_cb);
int  drive_write(struct drive_info *, void *, void *, uint32_t, drv_offset_t, drive_cb);
int  drive_prefetch(struct drive_info *, void *, uint32_t, drv_offset_t, drive_cb);
void drive_cancel_transfers(void);
void drive_check_complete(void);
int  drive_async_event_in_progress(void);
#endif