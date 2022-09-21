#include "net.h"
#include "pc.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


enum
{
    STATE_DEFAULT,
    STATE_KEY,
    STATE_VALUE,
    STATE_SECTION,
    STATE_COMMENT
};
struct ini_field
{
    char *name;
    char *data;
    void *next;
};
struct ini_section
{
    char             *name;
    struct ini_field *fields;
    void             *next;
};
struct ini_enum
{
    const char *name;
    int         value;
};


static const struct ini_enum drive_types[] = {
    {"cd", DRIVE_TYPE_CDROM}, {"hd", DRIVE_TYPE_DISK}, {"none", DRIVE_TYPE_NONE}, {NULL, 0}};
static const struct ini_enum boot_types[] = {
    {"cd", BOOT_CDROM}, {"hd", BOOT_DISK}, {"fd", BOOT_FLOPPY}, {"none", BOOT_NONE}, {NULL, 0}};
static const struct ini_enum driver_types[] = {{"sync", 1},    {"raw", 1}, {"chunked", 0}, {"normal", 0},
                                               {"network", 2}, {"net", 2}, {NULL, 0}};
static const struct ini_enum virtio_types[] = {
    {"9p", VIRTIO_9P}, {"p9", VIRTIO_9P}, {"9pfs", VIRTIO_9P}, {"p9fs", VIRTIO_9P}, {NULL, 0}};



static int load_file(struct loaded_file *lf, char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return -1;
    fseek(f, 0, SEEK_END);
    int l = ftell(f);
    fseek(f, 0, SEEK_SET);
    lf->length = l;
    lf->data   = aalloc(l, 4096);
    if (fread(lf->data, l, 1, f) != 1)
        return -1;
    fclose(f);
    return 0;
}
static inline char *slice_string(char *y, int start, int end)
{
    int   length = end - start;
    char *result = (char *)malloc(length + 1);
    memcpy(result, y + start, length);
    result[length] = 0;
    return result;
}
static struct ini_section *ini_parse(char *x)
{
    int state = STATE_DEFAULT, length = strlen(x), i = 0, strstart = 0, strend = 0, include_whitespace = 0;
    struct ini_section *result = (struct ini_section *)calloc(1, sizeof(struct ini_section)), *head = result;
    struct ini_field   *current_field = NULL;
    while (i < length) {
        int c = x[i++];
        switch (state) {
            case STATE_DEFAULT:
                if (c == '#')
                    state = STATE_COMMENT;
                else if (c == '[')
                    state = STATE_SECTION;
                else if (c > ' ') {
                    i--;
                    state    = STATE_KEY;
                    strstart = i;
                }
                strstart = i;
                break;
            case STATE_COMMENT:
                if (c == '\n')
                    state = STATE_DEFAULT;
                break;
            case STATE_SECTION:
                if (c == ']') {
                    struct ini_section *sect = (struct ini_section *)calloc(1, sizeof(struct ini_section));
                    sect->name               = slice_string(x, strstart, i - 1);
                    head->next               = sect;
                    head                     = sect;
                    state                    = STATE_DEFAULT;
                }
                break;
            case STATE_KEY:
                if (c == '=') {
                    struct ini_field *field = (struct ini_field *)calloc(1, sizeof(struct ini_field)), *temp;
                    field->name             = slice_string(x, strstart, strend);
                    temp                    = head->fields;
                    head->fields            = field;
                    field->next             = temp;
                    current_field           = field;
                    state                   = STATE_VALUE;
                    include_whitespace      = 0;
                    break;
                }
                if (c > 32)
                    strend = i;
                break;
            case STATE_VALUE:
                if (c == '\n') {
                    current_field->data = slice_string(x, strstart, strend);
                    state               = STATE_DEFAULT;
                    current_field       = NULL;
                    break;
                }
                if (!include_whitespace) {
                    if (c <= 32)
                        break;
                    strstart = i - 1;
                    strend   = strstart;
                }
                if (c > 32) {
                    strend = i;
                }
                include_whitespace = 1;
                break;
        }
    }
    return result;
}
static struct ini_section *get_section(struct ini_section *sect, char *name)
{
    while (sect) {
        if (sect->name) {
            if (!strcmp(sect->name, name))
                return sect;
        }
        sect = (struct ini_section *)sect->next;
    }
    return NULL;
}
static char *get_field_string(struct ini_section *sect, char *name)
{
    struct ini_field *f = sect->fields;
    while (f) {
        if (!strcmp(f->name, name))
            return f->data;
        f = (struct ini_field *)f->next;
    }
    return NULL;
}
static int get_field_enum(struct ini_section *sect, char *name, const struct ini_enum *vals, int def)
{
    char *x = get_field_string(sect, name);
    if (!x)
        return def;
    int i = 0;
    while (vals[i].name) {
        if (!strcmp(vals[i].name, x))
            return vals[i].value;
        i++;
    }
    printf("Unknown value: %s\n", name);
    return def;
}
static int get_field_int(struct ini_section *sect, char *name, int def)
{
    char *str = get_field_string(sect, name);
    int   res = 0, i = 0;
    if (!str)
        return def;
    if (str[0] == '0' && str[1] == 'x')
        for (;; ++i) {
            int n;
            if (str[i] >= '0' && str[i] <= '9')
                n = str[i] - '0';
            else if (str[i] >= 'A' && str[i] <= 'F')
                n = str[i] - 'A';
            else if (str[i] >= 'a' && str[i] <= 'f')
                n = str[i] - 'a';
            else
                break;
            res = (res << 4) + n;
        }
    else
        for (;; ++i) {
            if (str[i] < '0' || str[i] > '9')
                break;
            res = res * 10 + str[i] - '0';
        }
    switch (str[i]) {
        case 'K':
        case 'k':
            res <<= 10;
            break;
        case 'M':
        case 'm':
            res <<= 20;
            break;
        case 'G':
        case 'g':
            res <<= 30;
            break;
    }
    return res;
}
static int get_field_long(struct ini_section *sect, char *name, int def)
{
    char    *str = get_field_string(sect, name);
    uint64_t res = 0;
    int      i   = 0;
    if (!str)
        return def;
    for (;; ++i) {
        if (str[i] < '0' || str[i] > '9')
            break;
        res = res * 10 + str[i] - '0';
    }
    return res;
}
static void free_ini(struct ini_section *sect)
{
    while (sect) {
        free(sect->name);
        struct ini_field *f = sect->fields;
        while (f) {
            free(f->name);
            free(f->data);
            struct ini_field *f_next = (struct ini_field *)f->next;
            free(f);
            f = f_next;
        }
        struct ini_section *next = (struct ini_section *)sect->next;
        free(sect);
        sect = next;
    }
}
static int parse_disk(struct drive_info *drv, struct ini_section *s, int id)
{
    if (s == NULL) {
        drv->type = DRIVE_TYPE_NONE;
        return 0;
    }
    drv->type    = get_field_enum(s, (char *)"type", drive_types, DRIVE_TYPE_DISK);
    int driver   = get_field_enum(s, (char *)"driver", driver_types, -1),
        inserted = get_field_int(s, (char *)"inserted", 0), wb = get_field_int(s, (char *)"writeback", 0);
    char *path = get_field_string(s, (char *)"file");
    if (driver < 0 && inserted) {
        driver = drive_autodetect_type(path);
        if (driver < 0)
            FATAL("INI", "Unable to determine driver to use for ata%d-%s!\n", id >> 1, id & 1 ? "slave" : "master");
    }
    if (driver == 0 && wb)
        printf("WARNING: Disk %d uses async (chunked) driver but writeback is not supported!!\n", id);
    drv->modify_backing_file = wb;
    if (path && inserted) {
        UNUSED(id);
        if (driver == 0)
            return drive_init(drv, path);
        else
            return drive_simple_init(drv, path);
    }
    return 0;
}
static char *dupstr(char *src)
{
    if (!src)
        return NULL;
    int   len = strlen(src);
    char *res = (char *)malloc(len + 1);
    strcpy(res, src);
    return res;
}
int parse_cfg(struct pc_settings *pc, char *data)
{
    struct ini_section *global = ini_parse(data);
    char *bios = get_field_string(global, (char *)"bios"), *vgabios = get_field_string(global, (char *)"vgabios");
    if (!bios || !vgabios) {
        fprintf(stderr, (char *)"No BIOS/VGABIOS!\n");
        free_ini(global);
        return -1;
    }
    if (load_file(&pc->bios, bios) || load_file(&pc->vgabios, vgabios)) {
        fprintf(stderr, (char *)"Unable to load BIOS/VGABIOS image\n");
        free_ini(global);
        return -1;
    }
    pc->memory_size     = get_field_int(global, (char *)"memory", 32 * 1024 * 1024);
    pc->vga_memory_size = get_field_int(global, (char *)"vgamemory", 4 * 1024 * 1024);
    pc->current_time    = get_field_long(global, (char *)"now", 0);
    pc->pci_enabled     = get_field_int(global, (char *)"pci", 1);
    pc->acpi_enabled    = get_field_int(global, (char *)"acpi", 1);
    pc->apic_enabled    = get_field_int(global, (char *)"apic", 1);
    pc->floppy_enabled  = get_field_int(global, (char *)"floppy", 1);
    pc->vbe_enabled     = get_field_int(global, (char *)"vbe", 1);
    pc->pci_vga_enabled = get_field_int(global, (char *)"pcivga", 0);
    pc->boot_kernel     = get_field_int(global, (char *)"kernel", 0);
    int res             = parse_disk(&pc->drives[0], get_section(global, (char *)"ata0-master"), 0);
    res |= parse_disk(&pc->drives[1], get_section(global, (char *)"ata0-slave"), 1);
    res |= parse_disk(&pc->drives[2], get_section(global, (char *)"ata1-master"), 2);
    res |= parse_disk(&pc->drives[3], get_section(global, (char *)"ata1-slave"), 3);
    if (res) {
        fprintf(stderr, (char *)"Unable to initialize disk drive images\n");
        free_ini(global);
        return -1;
    }
    res = parse_disk(&pc->floppy_drives[0], get_section(global, (char *)"fda"), 4);
    res |= parse_disk(&pc->floppy_drives[1], get_section(global, (char *)"fdb"), 5);
    if (res) {
        fprintf(stderr, "Unable to initialize floppy drive images\n");
        free_ini(global);
        return -1;
    }
    struct ini_section *net = get_section(global, (char *)"ne2000");
    if (net) {
        pc->ne2000.enabled   = get_field_int(net, (char *)"enabled", 1);
        pc->ne2000.pci       = get_field_int(net, (char *)"pci", pc->pci_enabled);
        pc->ne2000.port_base = get_field_int(net, (char *)"iobase", 0x300);
        pc->ne2000.irq       = get_field_int(net, (char *)"irq", 3);
        char *mac            = get_field_string(net, (char *)"mac");
        if (!mac) {
            for (int i = 0; i < 6; i++)
                pc->ne2000.mac_address[i] = 0;
        } else {
            for (int k = 0, i = 0; k < 6; k++) {
                int mac_part = 0;
                if (k != 0)
                    if (mac[i++] != ':')
                        fprintf(stderr, "Malformed MAC address\n");
                for (int j = 0; j < 2; j++) {
                    int n;
                    if (mac[j + i] >= '0' && mac[j + i] <= '9')
                        n = mac[j + i] - '0';
                    else if (mac[j + i] >= 'A' && mac[j + i] <= 'F')
                        n = mac[j + i] - 'A';
                    else if (mac[j + i] >= 'a' && mac[j + i] <= 'f')
                        n = mac[j + i] - 'a';
                    else
                        FATAL("INI", "Malformed MAC address\n");
                    mac_part = (mac_part << 4) | n;
                }
                i += 2;
            }
        }
        if (pc->ne2000.enabled) {
            char *cfg = get_field_string(net, (char *)"arg");
            net_init(cfg);
        }
    } else {
        pc->ne2000.enabled = 0;
    }
    if (pc->boot_kernel) {
        struct ini_section *kernel = get_section(global, (char *)"kernel");
        pc->kernel_cmdline         = dupstr(get_field_string(kernel, (char *)"cmdline"));
        pc->kernel_img             = dupstr(get_field_string(kernel, (char *)"image"));
    } else {
        pc->kernel_cmdline = NULL;
        pc->kernel_img     = NULL;
    }
    char sid[50];
    for (int i = 0; i < MAX_VIRTIO_DEVICES; i++) {
        sprintf(sid, (char *)"virtio%d", i);
        struct ini_section *virtio = get_section(global, sid);
        struct virtio_cfg  *cfg    = &pc->virtio[i];
        cfg->type                  = -1;
        if (!virtio)
            continue;
        int x = get_field_enum(virtio, (char *)"type", virtio_types, -1);
        if (x == -1) {
            fprintf(stderr, "Unknown virtio%d type - ignoring\n", i);
            continue;
        }
        cfg->type = x;
        switch (x) {
            case VIRTIO_9P:
                cfg->fs9p.path = dupstr(get_field_string(virtio, (char *)"path"));
                cfg->fs9p.ro   = get_field_int(virtio, (char *)"readonly", 1);
                break;
        }
    }
    struct ini_section *boot = get_section(global, (char *)"boot");
    if (boot == NULL) {
        pc->boot_sequence[0] = BOOT_DISK;
        pc->boot_sequence[1] = BOOT_CDROM;
        pc->boot_sequence[2] = BOOT_FLOPPY;
    } else {
        pc->boot_sequence[0] = get_field_enum(boot, (char *)"a", boot_types, BOOT_DISK);
        pc->boot_sequence[1] = get_field_enum(boot, (char *)"b", boot_types, BOOT_CDROM);
        pc->boot_sequence[2] = get_field_enum(boot, (char *)"c", boot_types, BOOT_FLOPPY);
    }
    struct ini_section *cpu = get_section(global, (char *)"cpu");
    if (cpu == NULL) {
        pc->cpu.cpuid_limit_winnt = 0;
    } else {
        pc->cpu.cpuid_limit_winnt = get_field_int(cpu, (char *)"cpuid_limit_winnt", 0);
    }
    UNUSED(get_section);
    free_ini(global);
    return 0;
}