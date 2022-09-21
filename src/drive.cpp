#include "drive.h"
#include <alloca.h>
#include "state.h"
#include "util.h"
#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>


#define SIMULATE_ASYNC_ACCESS
#define BLOCK_SHIFT 18
#define BLOCK_SIZE  (256 * 1024)
#define BLOCK_MASK  (BLOCK_SIZE - 1)
#define PATHSEP     '/'


static void (*global_cb)(void *, int);
static void *global_cb_arg1;

static int transfer_in_progress = 0;


void drive_cancel_transfers(void)
{
    transfer_in_progress = 0;
}
int drive_read(struct drive_info *info, void *a, void *b, uint32_t c, drv_offset_t d, drive_cb e)
{
    return info->read(info->data, a, b, c, d, e);
}
int drive_prefetch(struct drive_info *info, void *a, uint32_t b, drv_offset_t c, drive_cb d)
{
    return info->prefetch(info->data, a, b, c, d);
}
int drive_write(struct drive_info *info, void *a, void *b, uint32_t c, drv_offset_t d, drive_cb e)
{
    return info->write(info->data, a, b, c, d, e);
}
struct block_info
{
    uint32_t pathindex;
    uint32_t modified;
    uint8_t *data;
};
struct drive_internal_info
{
    void (*callback)(void *, int);
    void              *ide_callback_arg1;
    void              *argument_buffer;
    drv_offset_t       argument_position, argument_length;
    drv_offset_t       size;
    uint32_t           block_count, block_size, path_count;
    char             **paths;
    struct block_info *blocks;
};
struct drive_info_file
{
    uint32_t size;
    uint32_t block_size;
};
static void join_path(char *dest, int alen, char *a, char *b)
{
    strcpy(dest, a);
    if (dest[alen - 1] != PATHSEP) {
        dest[alen] = PATHSEP;
        alen++;
    }
    strcpy(dest + alen, b);
}
static void drive_get_path(char *dest, char *pathbase, uint32_t x)
{
    char temp[16];
    sprintf(temp, "blk%08x.bin", x);
    join_path(dest, strlen(pathbase), pathbase, temp);
}
static int drive_read_block_internal(struct drive_internal_info *self, struct block_info *info, void *buffer,
                                     uint32_t length, uint32_t position)
{
    uint32_t blockoffs = position % self->block_size;
    memcpy(buffer, info->data + blockoffs, length);
    return 0;
}
static void *drive_read_file(struct drive_internal_info *self, char *fn)
{
    char         temp[1024 + 8];
    int          fd;
    unsigned int size;
    void        *readbuf, *data;
    sprintf(temp, "%s.gz", fn);
    fd = open(temp, O_RDONLY | 0);
    if (fd < 0) {
        fd = open(fn, O_RDONLY | 0);
        if (fd < 0) {
            perror("open: ");
            DRIVE_FATAL("Could not open file %s\n", fn);
        }
        data = malloc(self->block_size);
        size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        if (read(fd, data, size) != (ssize_t)size)
            DRIVE_FATAL("Could not read file %s\n", fn);
        close(fd);
        return data;
    }
    data = malloc(self->block_size);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    readbuf = malloc(size);
    if (read(fd, readbuf, size) != (ssize_t)size)
        DRIVE_FATAL("Could not read file %s\n", fn);
    z_stream inflate_stream = {0};
    inflate_stream.zalloc   = Z_NULL;
    inflate_stream.zfree    = Z_NULL;
    inflate_stream.opaque   = Z_NULL;
    inflate_stream.avail_in = inflate_stream.total_in = size;
    inflate_stream.next_in                            = (Bytef *)readbuf;
    inflate_stream.avail_out = inflate_stream.total_out = self->block_size;
    inflate_stream.next_out                             = (Bytef *)data;
    int err                                             = inflateInit(&inflate_stream);
    if (err == Z_OK) {
        err = inflate(&inflate_stream, Z_FINISH);
        if (err != Z_STREAM_END) {
            inflateEnd(&inflate_stream);
            DRIVE_FATAL("Unable to inflate %s\n", temp);
        }
    } else {
        inflateEnd(&inflate_stream);
        DRIVE_FATAL("Unable to inflate %s\n", temp);
    }
    free(readbuf);
    close(fd);
    return data;
}
static int drive_internal_read_remote(struct drive_internal_info *self, struct block_info *blockinfo, uint8_t *buffer,
                                      uint32_t pos, uint32_t length)
{
    char     temp[1024];
    uint32_t block = pos / self->block_size;
    drive_get_path(temp, self->paths[blockinfo->pathindex], block);
    blockinfo->data = (uint8_t *)drive_read_file(self, temp);

    UNUSED(buffer);
    UNUSED(length);
    return 0;
}
static int drive_internal_read_check(struct drive_internal_info *self, void *buffer, uint32_t length,
                                     drv_offset_t position, int no_xhr)
{
    uint32_t readEnd             = position + length,
             blocksToRead        = ((((readEnd - 1) & ~BLOCK_MASK) - (position & ~BLOCK_MASK)) >> BLOCK_SHIFT) + 1;
    uint32_t currentFilePosition = position;
    int      retval              = 0;
    for (unsigned int i = 0; i < blocksToRead; i++) {
        struct block_info *blockInformation = &self->blocks[currentFilePosition >> BLOCK_SHIFT];
        uint32_t           begin = 0, end = BLOCK_SIZE, len = 0;
        if (currentFilePosition & BLOCK_MASK) {
            begin = currentFilePosition & BLOCK_MASK;
        }
        if ((currentFilePosition ^ readEnd) < BLOCK_SIZE) {
            end = readEnd & BLOCK_MASK;
            if (end == 0)
                end = BLOCK_SIZE;
        }

        len = end - begin;
        if (blockInformation->data) {
            retval |= drive_read_block_internal(self, blockInformation, buffer, len, currentFilePosition);
        } else {
            if (!no_xhr)
                if (drive_internal_read_remote(self, blockInformation, (uint8_t *)buffer, currentFilePosition, len) < 0)
                    DRIVE_FATAL("Unable to load disk\n");

            retval |= 1;
        }
        buffer = (uint8_t *)buffer + len;
        currentFilePosition += len;
    }
    return retval;
}
static void drive_internal_read_cb(void *self_ptr, int status)
{
    if (!transfer_in_progress)
        return;
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    if (drive_internal_read_check(self, self->argument_buffer, self->argument_length, self->argument_position, 1))
        DRIVE_FATAL("We haven't loaded everything..?\n");
    self->callback(self->ide_callback_arg1, status);
}
static int drive_internal_read(void *self_ptr, void *cb_ptr, void *buffer, uint32_t length, drv_offset_t position,
                               drive_cb cb)
{
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    if (!drive_internal_read_check(self, buffer, length, position, 0))
        return DRIVE_RESULT_SYNC;
    self->argument_buffer   = buffer;
    self->argument_length   = length;
    self->argument_position = position;
    self->callback          = cb;
    self->ide_callback_arg1 = cb_ptr;
    transfer_in_progress    = 1;
    global_cb               = drive_internal_read_cb;
    global_cb_arg1          = self;
    return DRIVE_RESULT_ASYNC;
}
static int drive_internal_write_remote(struct drive_internal_info *self, struct block_info *blockinfo, uint8_t *buffer,
                                       uint32_t pos, drv_offset_t length)
{
    char     temp[1024];
    uint32_t block = pos / self->block_size;
    drive_get_path(temp, self->paths[blockinfo->pathindex], block);
    blockinfo->data = (uint8_t *)drive_read_file(self, temp);

    UNUSED(buffer);
    UNUSED(length);
    return 0;
}
static int drive_write_block_internal(struct drive_internal_info *self, struct block_info *info, void *buffer,
                                      uint32_t length, drv_offset_t position)
{
    info->modified     = 1;
    uint32_t blockoffs = position % self->block_size;
    memcpy(info->data + blockoffs, buffer, length);
    return 0;
}
static int drive_internal_write_check(struct drive_internal_info *self, void *buffer, uint32_t length,
                                      drv_offset_t position, int no_xhr)
{
    drv_offset_t writeEnd        = position + length,
                 blocksToWrite   = ((((writeEnd - 1) & ~BLOCK_MASK) - (position & ~BLOCK_MASK)) >> BLOCK_SHIFT) + 1;
    uint32_t currentFilePosition = position;
    int      retval              = 0;
    for (unsigned int i = 0; i < blocksToWrite; i++) {
        struct block_info *blockInformation = &self->blocks[currentFilePosition >> BLOCK_SHIFT];
        uint32_t           begin = 0, end = BLOCK_SIZE, len = 0;
        if (currentFilePosition & BLOCK_MASK) {
            begin = currentFilePosition & BLOCK_MASK;
        }
        if ((currentFilePosition ^ writeEnd) < BLOCK_SIZE) {
            end = writeEnd & BLOCK_MASK;
            if (end == 0)
                end = BLOCK_SIZE;
        }
        blockInformation->modified = 1;
        len                        = end - begin;
        if (blockInformation->data)
            retval |= drive_write_block_internal(self, blockInformation, buffer, len, currentFilePosition);
        else {
            if (!no_xhr)
                if (drive_internal_write_remote(self, blockInformation, (uint8_t *)buffer, currentFilePosition, len) <
                    0)
                    DRIVE_FATAL("Unable to load disk\n");

            retval |= 1;
        }
        buffer = (uint8_t *)buffer + len;
        currentFilePosition += len;
    }
    return retval;
}
static void drive_internal_write_cb(void *self_ptr, int status)
{
    if (!transfer_in_progress)
        return;
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    if (drive_internal_write_check(self, self->argument_buffer, self->argument_length, self->argument_position, 1))
        DRIVE_FATAL("We haven't loaded everything..?\n");
    self->callback(self->ide_callback_arg1, status);
}
static int drive_internal_write(void *self_ptr, void *cb_ptr, void *buffer, uint32_t length, drv_offset_t position,
                                drive_cb cb)
{
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    if (!drive_internal_write_check(self, buffer, length, position, 0))
        return DRIVE_RESULT_SYNC;
    self->argument_buffer   = buffer;
    self->argument_length   = length;
    self->argument_position = position;
    self->callback          = cb;
    self->ide_callback_arg1 = cb_ptr;
    transfer_in_progress    = 1;
    global_cb               = drive_internal_write_cb;
    global_cb_arg1          = self;
    return DRIVE_RESULT_ASYNC;
}
static void drive_internal_prefetch_cb(void *self_ptr, int status)
{
    if (!transfer_in_progress)
        return;
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    self->callback(self->ide_callback_arg1, status);
}
static int drive_internal_prefetch_remote(struct drive_internal_info *self, struct block_info *blockinfo, uint32_t pos,
                                          drv_offset_t length)
{
    char     temp[1024];
    uint32_t block = pos / self->block_size;
    drive_get_path(temp, self->paths[blockinfo->pathindex], block);
    blockinfo->data = (uint8_t *)drive_read_file(self, temp);
    UNUSED(length);
    return 0;
}
static int drive_internal_prefetch_check(struct drive_internal_info *self, uint32_t length, drv_offset_t position)
{
    drv_offset_t readEnd         = position + length,
                 blocksToRead    = ((((readEnd - 1) & ~BLOCK_MASK) - (position & ~BLOCK_MASK)) >> BLOCK_SHIFT) + 1;
    uint32_t currentFilePosition = position;
    int      retval              = 0;
    for (unsigned int i = 0; i < blocksToRead; i++) {
        struct block_info *blockInformation = &self->blocks[currentFilePosition >> BLOCK_SHIFT];
        uint32_t           begin = 0, end = BLOCK_SIZE, len = 0;
        if (currentFilePosition & BLOCK_MASK) {
            begin = currentFilePosition & BLOCK_MASK;
        }
        if ((currentFilePosition ^ readEnd) < BLOCK_SIZE) {
            end = readEnd & BLOCK_MASK;
            if (end == 0)
                end = BLOCK_SIZE;
        }
        len = end - begin;
        if (blockInformation->data) {
        } else {
            if (drive_internal_prefetch_remote(self, blockInformation, currentFilePosition, len) < 0)
                DRIVE_FATAL("Unable to load disk\n");

            retval |= 1;
        }
        currentFilePosition += len;
    }
    return retval;
}
static int drive_internal_prefetch(void *self_ptr, void *cb_ptr, uint32_t length, drv_offset_t position, drive_cb cb)
{
    if (position > 0xFFFFFFFF)
        DRIVE_FATAL("TODO: big access\n");
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    if (!drive_internal_prefetch_check(self, length, position))
        return DRIVE_RESULT_SYNC;
    self->argument_length   = length;
    self->argument_position = position;
    self->callback          = cb;
    self->ide_callback_arg1 = cb_ptr;
    transfer_in_progress    = 1;
    global_cb               = drive_internal_prefetch_cb;
    global_cb_arg1          = self;
    return DRIVE_RESULT_ASYNC;
}
static void drive_internal_state(void *self_ptr, char *pn)
{
    char                        temp[100];
    struct drive_internal_info *self = (drive_internal_info *)self_ptr;
    struct bjson_object        *obj  = state_obj(pn, 4 + self->path_count + 1);
    state_field(obj, 4, (char *)"size", &self->size);
    state_field(obj, 4, (char *)"path_count", &self->path_count);
    state_field(obj, 4, (char *)"block_count", &self->block_count);
    uint32_t *block_infos = (uint32_t *)alloca(self->block_count * 4);
    if (state_is_reading()) {
        int old_path_counts = self->path_count;
        state_field(obj, 4, (char *)"path_count", &self->path_count);
        for (int i = 0; i < old_path_counts; i++)
            free(self->paths[i]);
        self->paths = (char **)realloc(self->paths, self->path_count * sizeof(char *));
        int paths0  = 0;
        for (unsigned int i = 0; i < self->path_count; i++) {
            sprintf(temp, "path%d", i);
            state_string(obj, temp, &self->paths[i]);
            printf("%s\n", self->paths[i]);
            paths0++;
        }
        state_field(obj, self->block_count * 4, (char *)"block_array", block_infos);
        for (unsigned int i = 0; i < self->block_count; i++) {
            if (self->blocks[i].data)
                free(self->blocks[i].data);
            self->blocks[i].data      = NULL;
            self->blocks[i].modified  = 0;
            self->blocks[i].pathindex = block_infos[i];
        }
    } else {
        char pathname[1000];
        sprintf(pathname, "%s/%s", state_get_path_base(), pn);
        state_mkdir(pathname);
        uint32_t path_count   = self->path_count;
        int      existing_add = 1;
        for (unsigned int i = 0; i < self->path_count; i++) {
            printf("%s %s\n", pathname, self->paths[i]);
            if (!strcmp(pathname, self->paths[i])) {
                existing_add = 0;
                break;
            }
        }
        path_count += existing_add;
        state_field(obj, 4, (char *)"path_count", &path_count);
        int pwdindex = 0, pwdinc = 1;
        for (unsigned int i = 0; i < self->path_count; i++) {
            if (!strcmp(pathname, self->paths[i]))
                pwdinc = 0;
            pwdindex += pwdinc;
            sprintf(temp, "path%d", i);
            state_string(obj, temp, &self->paths[i]);
        }
        if (pwdinc) {
            sprintf(temp, "path%d", pwdindex);
            char *foo_bad = pathname;
            state_string(obj, temp, &foo_bad);
        }
        for (unsigned int i = 0; i < self->block_count; i++) {
            if (self->blocks[i].modified) {
                sprintf(pathname, "%s/blk%08x.bin", pn, i);
                state_file(BLOCK_SIZE, pathname, self->blocks[i].data);
                self->blocks[i].pathindex = pwdindex;
            }
            block_infos[i] = self->blocks[i].pathindex;
        }
        state_field(obj, self->block_count * 4, (char *)"block_array", block_infos);
    }
}
static int drive_internal_init(struct drive_info *info, char *filename, void *info_dat, int drvid)
{
    struct drive_internal_info *drv      = (struct drive_internal_info *)malloc(sizeof(struct drive_internal_info));
    int                         len      = strlen(filename);
    void                       *pathbase = malloc(len + 1);
    strcpy((char *)pathbase, filename);
    drv->path_count = 1;
    drv->paths      = (char **)malloc(sizeof(char *));
    drv->paths[0]   = (char *)pathbase;
    UNUSED(drvid);

    struct drive_info_file *internal = (struct drive_info_file *)info_dat;
    drv->block_size                  = internal->block_size;
    drv->size                        = internal->size;
    drv->block_count                 = (internal->block_size + internal->size - 1) / internal->block_size;
    drv->blocks                      = (struct block_info *)calloc(sizeof(struct block_info), drv->block_count);
    info->data                       = drv;
    info->read                       = drive_internal_read;
    info->write                      = drive_internal_write;
    info->state                      = drive_internal_state;
    info->prefetch                   = drive_internal_prefetch;
    info->sectors                    = internal->size / 512;
    info->sectors_per_cylinder       = 63;
    info->heads                      = 16;
    info->cylinders_per_head         = info->sectors / (info->sectors_per_cylinder * info->heads);
    return 0;
}
void drive_check_complete(void)
{
    if (transfer_in_progress) {
        global_cb(global_cb_arg1, 0);
        transfer_in_progress = 0;
    }
}
int drive_async_event_in_progress(void)
{
    return transfer_in_progress;
}
int drive_init(struct drive_info *info, char *filename)
{
    char buf[1024];
    int  filelen = strlen(filename);
    if (filelen > 1000)
        return -1;
    join_path(buf, filelen, filename, (char *)"info.dat");
    int fd = open(buf, O_RDONLY | 0);
    if (fd == -1)
        return -1;
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    void *data = malloc(size);
    if (read(fd, data, size) != size)
        return -1;
    close(fd);
    drive_internal_init(info, filename, data, -1);
    free(data);
    return 0;
}
void drive_state(struct drive_info *info, char *filename)
{
    info->state(info->data, filename);
}
struct simple_driver
{
    int          fd;
    drv_offset_t image_size, block_size;
    uint32_t     block_array_size;
    int          write_protected;
    int          raw_file_access;
    uint8_t    **blocks;
};
static void drive_simple_state(void *self, char *path)
{
    UNUSED(self);
    UNUSED(path);
}
static int drive_simple_prefetch(void *self_ptr, void *cb_ptr, uint32_t length, drv_offset_t position, drive_cb cb)
{
    UNUSED(self_ptr);
    UNUSED(cb_ptr);
    UNUSED(length);
    UNUSED(position);
    UNUSED(cb);
    return DRIVE_RESULT_SYNC;
}
static inline int drive_simple_has_cache(struct simple_driver *info, drv_offset_t offset)
{
    return info->blocks[offset / info->block_size] != NULL;
}
static int drive_simple_fetch_cache(struct simple_driver *info, void *buffer, drv_offset_t offset)
{
    if (offset & 511)
        DRIVE_FATAL("Offset not aligned to 512 byte boundary");
    uint32_t blockid = offset / info->block_size;
    if (info->blocks[blockid]) {
        uint32_t block_offset = offset % info->block_size;
        void    *ptr          = info->blocks[blockid] + block_offset;
        memcpy(buffer, ptr, 512);
        return 1;
    }
    return 0;
}
static int drive_simple_add_cache(struct simple_driver *info, drv_offset_t offset)
{
    void *dest = info->blocks[offset / info->block_size] = (uint8_t *)malloc(info->block_size);
    lseek(info->fd, offset & (drv_offset_t) ~(info->block_size - 1), SEEK_SET);
    if ((uint32_t)read(info->fd, dest, info->block_size) != info->block_size)
        DRIVE_FATAL("Unable to read %d bytes from image file\n", (int)info->block_size);
    return 0;
}
static inline int drive_simple_write_cache(struct simple_driver *info, void *buffer, drv_offset_t offset)
{
    uint32_t block_offset = offset % info->block_size;
    void    *ptr          = info->blocks[offset / info->block_size] + block_offset;
    memcpy(ptr, buffer, 512);
    return 0;
}
static int drive_simple_write(void *self, void *cb_ptr, void *buffer, uint32_t size, drv_offset_t offset, drive_cb cb)
{
    UNUSED(self);
    UNUSED(cb);
    UNUSED(cb_ptr);
    if ((size | offset) & 511)
        DRIVE_FATAL("Length/offset must be multiple of 512 bytes\n");
    struct simple_driver *info = (struct simple_driver *)self;
    drv_offset_t          end  = size + offset;
    while (offset != end) {
        if (!info->raw_file_access) {
            if (!drive_simple_has_cache(info, offset))
                drive_simple_add_cache(info, offset);
            drive_simple_write_cache(info, buffer, offset);
        } else {
            UNUSED(drive_simple_add_cache);
            lseek(info->fd, offset, SEEK_SET);
            if (write(info->fd, buffer, 512) != 512)
                DRIVE_FATAL("Unable to write 512 bytes to image file\n");
        }
        buffer = (uint8_t *)buffer + 512;
        offset += 512;
    }
    return DRIVE_RESULT_SYNC;
}
static int drive_simple_read(void *self, void *cb_ptr, void *buffer, uint32_t size, drv_offset_t offset, drive_cb cb)
{
    UNUSED(self);
    UNUSED(cb);
    UNUSED(cb_ptr);
    if ((size | offset) & 511)
        DRIVE_FATAL("Length/offset must be multiple of 512 bytes\n");
    struct simple_driver *info = (struct simple_driver *)self;
    drv_offset_t          end  = size + offset;
    while (offset != end) {
        if (!drive_simple_fetch_cache(info, buffer, offset)) {
            lseek(info->fd, offset, SEEK_SET);
            if (read(info->fd, buffer, 512) != 512)
                DRIVE_FATAL("Unable to read 512 bytes from image file\n");
        }
        buffer = (uint8_t *)buffer + 512;
        offset += 512;
    }
    return DRIVE_RESULT_SYNC;
}
int drive_simple_init(struct drive_info *info, char *filename)
{
    int fd;
    if (!info->modify_backing_file)
        fd = open(filename, O_RDONLY | 0);
    else
        fd = open(filename, O_RDWR | 0);
    if (fd < 0)
        return -1;
    uint64_t size = lseek(fd, 0, SEEK_END);
    if (size == (uint64_t)-1)
        return -1;
    lseek(fd, 0, SEEK_SET);
    struct simple_driver *sync_info = (struct simple_driver *)malloc(sizeof(struct simple_driver));
    info->data                      = sync_info;
    sync_info->fd                   = fd;
    sync_info->image_size           = size;
    sync_info->block_size           = BLOCK_SIZE;
    sync_info->block_array_size     = (size + sync_info->block_size - 1) / sync_info->block_size;
    sync_info->blocks               = (uint8_t **)calloc(sizeof(uint8_t *), sync_info->block_array_size);
    sync_info->raw_file_access      = info->modify_backing_file;
    info->read                      = drive_simple_read;
    info->state                     = drive_simple_state;
    info->write                     = drive_simple_write;
    info->prefetch                  = drive_simple_prefetch;
    info->sectors                   = size / 512;
    info->sectors_per_cylinder      = 63;
    info->heads                     = 16;
    info->cylinders_per_head        = info->sectors / (info->sectors_per_cylinder * info->heads);
    return 0;
}
void drive_destroy_simple(struct drive_info *info)
{
    struct simple_driver *simple_info = (struct simple_driver *)info->data;
    for (unsigned int i = 0; i < simple_info->block_array_size; i++)
        free(simple_info->blocks[i]);
    free(simple_info->blocks);
    free(simple_info);
}
int drive_autodetect_type(char *path)
{
    struct stat statbuf;
    if (strstr(path, "http://") != NULL || strstr(path, "https://") != NULL)
        return 2;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;
    if (fstat(fd, &statbuf)) {
        close(fd);
        return -1;
    }
    close(fd);
    if (S_ISDIR(statbuf.st_mode))
        return 0;
    else
        return 1;
}
