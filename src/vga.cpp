#include "cpu/cpu.h"
#include "devices.h"
#include "display.h"
#include "io.h"
#include "state.h"
#include "util.h"
#include <cstdint>
#include <string.h>

extern CPU cpu;


#define VGA_LOG(x, ...) LOG("VGA", x, ##__VA_ARGS__)
#define VGA_FATAL(x, ...)                                                                                              \
    do {                                                                                                               \
        VGA_LOG(x, ##__VA_ARGS__);                                                                                     \
        abort();                                                                                                       \
    } while (0)
#define VBE_LFB_BASE 0xE0000000

#define VBE_DISPI_DISABLED    0x00
#define VBE_DISPI_ENABLED     0x01
#define VBE_DISPI_GETCAPS     0x02
#define VBE_DISPI_8BIT_DAC    0x20
#define VBE_DISPI_LFB_ENABLED 0x40
#define VBE_DISPI_NOCLEARMEM  0x80
#define MASK(n)               (uint8_t)(~n)
#define DO_MASK(n)            _xor ^= mask_enabled & n ? value & lut32[n] : mask & lut32[n]


enum
{
    CHAIN4,
    ODDEVEN,
    NORMAL,
    READMODE_1
};
enum
{
    BLANK_RENDERER        = 0,
    ALPHANUMERIC_RENDERER = 2,
    MODE_13H_RENDERER     = 4,
    RENDER_4BPP           = 6,
    RENDER_32BPP          = 8,
    RENDER_8BPP           = 10,
    RENDER_16BPP          = 12,
    RENDER_24BPP          = 14
};
static struct vga_info
{
    uint8_t   crt[256], crt_index;
    uint8_t   attr[32], attr_index, attr_palette[16];
    uint8_t   seq[8], seq_index;
    uint8_t   gfx[256], gfx_index;
    uint8_t   dac[1024];
    uint32_t  dac_palette[256];
    uint8_t   dac_mask, dac_state, dac_address, dac_color, dac_read_address;
    uint8_t   status[2];
    uint8_t   misc;
    uint8_t   char_width;
    uint32_t  character_map[2];
    uint8_t   pixel_panning, current_pixel_panning;
    uint32_t  total_height, total_width;
    int       renderer;
    uint32_t  current_scanline, character_scanline;
    uint32_t *framebuffer;
    uint32_t  framebuffer_offset;
    uint32_t  vram_addr;
    uint32_t  scanlines_to_update;
    uint8_t   write_access, read_access, write_mode;
    uint32_t  vram_window_base, vram_window_size;
    union
    {
        uint8_t  latch8[4];
        uint32_t latch32;
    };
    uint16_t vbe_index, vbe_version, vbe_enable;
    uint32_t vbe_regs[10];
    uint32_t vbe_bank;
    uint32_t vgabios_addr;
    uint8_t *mem;
    int      vram_size;
    uint8_t *vram, *rom;
    uint32_t rom_size;
    uint8_t *vbe_scanlines_modified;
    int      memory_modified;
} vga;

static const uint8_t  pci_config_space[16] = {0x34, 0x12, 0x11, 0x11, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0};
static const uint32_t vbe_maximums[3]      = {1024, 768, 32};
static int            framectr             = 0;

static void vga_update_size(void);


static void vga_alloc_mem(void)
{
    if (vga.vram)
        afree(vga.vram);
    vga.vram = (uint8_t *)aalloc(vga.vram_size, 8);
    memset(vga.vram, 0, vga.vram_size);
}
static void vga_state(void)
{
    struct bjson_object *obj = state_obj((char *)"vga", 42);
    state_field(obj, 256, (char *)"vga.crt", &vga.crt);
    state_field(obj, 1, (char *)"vga.crt_index", &vga.crt_index);
    state_field(obj, 32, (char *)"vga.attr", &vga.attr);
    state_field(obj, 1, (char *)"vga.attr_index", &vga.attr_index);
    state_field(obj, 16, (char *)"vga.attr_palette", &vga.attr_palette);
    state_field(obj, 8, (char *)"vga.seq", &vga.seq);
    state_field(obj, 1, (char *)"vga.seq_index", &vga.seq_index);
    state_field(obj, 256, (char *)"vga.gfx", &vga.gfx);
    state_field(obj, 1, (char *)"vga.gfx_index", &vga.gfx_index);
    state_field(obj, 1024, (char *)"vga.dac", &vga.dac);
    state_field(obj, 1024, (char *)"vga.dac_palette", &vga.dac_palette);
    state_field(obj, 1, (char *)"vga.dac_mask", &vga.dac_mask);
    state_field(obj, 1, (char *)"vga.dac_state", &vga.dac_state);
    state_field(obj, 1, (char *)"vga.dac_address", &vga.dac_address);
    state_field(obj, 1, (char *)"vga.dac_color", &vga.dac_color);
    state_field(obj, 1, (char *)"vga.dac_read_address", &vga.dac_read_address);
    state_field(obj, 2, (char *)"vga.status", &vga.status);
    state_field(obj, 1, (char *)"vga.misc", &vga.misc);
    state_field(obj, 1, (char *)"vga.char_width", &vga.char_width);
    state_field(obj, 8, (char *)"vga.character_map", &vga.character_map);
    state_field(obj, 1, (char *)"vga.pixel_panning", &vga.pixel_panning);
    state_field(obj, 1, (char *)"vga.current_pixel_panning", &vga.current_pixel_panning);
    state_field(obj, 4, (char *)"vga.total_height", &vga.total_height);
    state_field(obj, 4, (char *)"vga.total_width", &vga.total_width);
    state_field(obj, 4, (char *)"vga.renderer", &vga.renderer);
    state_field(obj, 4, (char *)"vga.current_scanline", &vga.current_scanline);
    state_field(obj, 4, (char *)"vga.character_scanline", &vga.character_scanline);
    state_field(obj, 4, (char *)"vga.framebuffer_offset", &vga.framebuffer_offset);
    state_field(obj, 4, (char *)"vga.vram_addr", &vga.vram_addr);
    state_field(obj, 1, (char *)"vga.write_access", &vga.write_access);
    state_field(obj, 1, (char *)"vga.read_access", &vga.read_access);
    state_field(obj, 1, (char *)"vga.write_mode", &vga.write_mode);
    state_field(obj, 4, (char *)"vga.vram_window_base", &vga.vram_window_base);
    state_field(obj, 4, (char *)"vga.vram_window_size", &vga.vram_window_size);
    state_field(obj, 4, (char *)"vga.latch8", &vga.latch8);
    state_field(obj, 2, (char *)"vga.vbe_index", &vga.vbe_index);
    state_field(obj, 2, (char *)"vga.vbe_version", &vga.vbe_version);
    state_field(obj, 2, (char *)"vga.vbe_enable", &vga.vbe_enable);
    state_field(obj, 40, (char *)"vga.vbe_regs", &vga.vbe_regs);
    state_field(obj, 4, (char *)"vga.vbe_bank", &vga.vbe_bank);
    state_field(obj, 4, (char *)"vga.vgabios_addr", &vga.vgabios_addr);
    state_field(obj, 4, (char *)"vga.vram_size", &vga.vram_size);

    if (state_is_reading()) {
        vga_update_size();
        vga_alloc_mem();
    }

    state_file(vga.vram_size, (char *)"vram", vga.vram);
    vga.memory_modified = 3;
}
static void vga_update_mem_access(void)
{
    if (vga.seq[4] & 8)
        vga.write_access = CHAIN4;
    else if (!(vga.seq[4] & 4))
        vga.write_access = ODDEVEN;
    else
        vga.write_access = NORMAL;
    if (vga.gfx[5] & 8)
        vga.read_access = READMODE_1;
    else if (vga.seq[4] & 8)
        vga.read_access = CHAIN4;
    else if (vga.gfx[5] & 0x10)
        vga.read_access = ODDEVEN;
    else
        vga.read_access = NORMAL;

    vga.write_mode = vga.gfx[5] & 3;
}
static void vga_complete_redraw(void)
{
    vga.current_scanline      = 0;
    vga.character_scanline    = vga.crt[8] & 0x1F;
    vga.current_pixel_panning = vga.pixel_panning;
    vga.vram_addr             = ((vga.crt[0x0C] << 8) | vga.crt[0x0D]) << 2;
    vga.framebuffer_offset    = 0;
    vga.memory_modified       = 3;
}
static void vga_change_renderer(void)
{
    if (vga.vbe_enable & VBE_DISPI_ENABLED) {
        switch (vga.vbe_regs[3]) {
            case 8:
                vga.renderer = RENDER_8BPP;
                break;
            case 16:
                vga.renderer = RENDER_16BPP;
                break;
            case 24:
                vga.renderer = RENDER_24BPP;
                break;
            case 32:
                vga.renderer = RENDER_32BPP;
                break;
            default:
                VGA_FATAL("TODO: support %dbpp displays!\n", vga.vbe_regs[3]);
        }
        goto done;
    }

    if (((vga.seq[1] & 0x20) == 0) && (vga.attr_index & 0x20)) {
        if (vga.gfx[6] & 1) {
            if (vga.gfx[5] & 0x40) {
                vga.renderer = MODE_13H_RENDERER;
                vga.renderer |= vga.attr[0x10] >> 6 & 1;
                goto done;
            } else {
                if (!(vga.gfx[5] & 0x20)) {
                    vga.renderer = RENDER_4BPP;
                } else
                    VGA_FATAL("TODO: other gfx mode\n");
            }
        } else {
            vga.renderer = ALPHANUMERIC_RENDERER;
        }
    } else {
        vga.renderer = BLANK_RENDERER;
    }

    vga.renderer |= (vga.seq[1] >> 3 & 1);
done:
    vga_complete_redraw();
}
static uint32_t vga_char_map_address(int b)
{
    return b << 13;
}
static void vga_update_size(void)
{
    int width, height;
    if (vga.vbe_enable & VBE_DISPI_ENABLED) {
        width  = vga.vbe_regs[1];
        height = vga.vbe_regs[2];
    } else {
        int horizontal_display_enable_end = vga.crt[1] + 1;
        int horizontal_blanking_start     = vga.crt[2];
        int total_horizontal_characters   = (horizontal_display_enable_end < horizontal_blanking_start)
                                                ? horizontal_display_enable_end
                                                : horizontal_blanking_start;
        width                             = total_horizontal_characters * vga.char_width;
        int vertical_display_enable_end =
            (vga.crt[0x12] + (((vga.crt[0x07] >> 1 & 1) | (vga.crt[0x07] >> 5 & 2)) << 8)) + 1;
        int vertical_blanking_start = vga.crt[0x15] + (((vga.crt[0x07] >> 3 & 1) | (vga.crt[0x09] >> 4 & 2)) << 8);
        height = vertical_display_enable_end < vertical_blanking_start ? vertical_display_enable_end
                                                                       : vertical_blanking_start;
    }
    display_set_resolution(width, height);
    vga.framebuffer  = (uint32_t *)display_get_pixels();
    vga.total_height = height;
    vga.total_width  = width;

    if (vga.vbe_scanlines_modified)
        vga.vbe_scanlines_modified = (uint8_t *)realloc(vga.vbe_scanlines_modified, vga.total_height);
    else
        vga.vbe_scanlines_modified = (uint8_t *)malloc(vga.total_height);

    memset(vga.vbe_scanlines_modified, 1, vga.total_height);
    vga.scanlines_to_update = height >> 1;
}
static uint8_t c6to8(uint8_t a)
{
    if (vga.vbe_enable & VBE_DISPI_8BIT_DAC)
        return a;
    uint8_t b = a & 1;
    return a << 2 | b << 1 | b;
}
static void update_one_dac_entry(int i)
{
    int index = i << 2;
    vga.dac_palette[i] =
        255 << 24 | c6to8(vga.dac[index | 0]) << 16 | c6to8(vga.dac[index | 1]) << 8 | c6to8(vga.dac[index | 2]);
}
static void update_all_dac_entries(void)
{
    for (int i = 0; i < 256; i++) {
        update_one_dac_entry(i);
    }
}
static void vga_change_attr_cache(int i)
{
    if (vga.attr[0x10] & 0x80)
        vga.attr_palette[i] = (vga.attr[i] & 0x0F) | ((vga.attr[0x14] << 4) & 0xF0);
    else
        vga.attr_palette[i] = (vga.attr[i] & 0x3F) | ((vga.attr[0x14] << 4) & 0xC0);
}
static void vga_write(uint32_t port, uint32_t data)
{
    if ((port >= 0x3B0 && port <= 0x3BF && (vga.misc & 1)) || (port >= 0x3D0 && port <= 0x3DF && !(vga.misc & 1))) {
        return;
    }
    uint8_t diffxor;
    switch (port) {
        case 0x1CE:
            vga.vbe_index = data;
            break;
        case 0x1CF:
            switch (vga.vbe_index) {
                case 0:
                    vga.vbe_version = data;
                    break;
                case 1 ... 3:
                    if (vga.vbe_enable & VBE_DISPI_GETCAPS)
                        ;
                    else {
                        if (vga.vbe_index == 3 && data == 0)
                            data = 8;
                        if (!(vga.vbe_enable & VBE_DISPI_ENABLED)) {
                            if (data <= vbe_maximums[vga.vbe_index - 1])
                                vga.vbe_regs[vga.vbe_index] = data;
                        }
                    }
                    break;
                case 4:
                    diffxor = vga.vbe_enable ^ data;
                    if (diffxor) {
                        if (!(diffxor & VBE_DISPI_ENABLED)) {
                            data &= ~VBE_DISPI_LFB_ENABLED;
                            data |= vga.vbe_enable & VBE_DISPI_LFB_ENABLED;
                        }

                        vga.vbe_enable = data;

                        int width = vga.vbe_regs[1], height = vga.vbe_regs[2];
                        vga.total_height = height;
                        vga.total_width  = width;
                        vga_update_size();
                        if (diffxor & VBE_DISPI_ENABLED) {
                            vga_change_renderer();
                            if (vga.vbe_enable & VBE_DISPI_ENABLED)
                                if (!(data & VBE_DISPI_NOCLEARMEM))
                                    memset(vga.vram, 0, vga.vram_size);
                        }
                        if (diffxor & VBE_DISPI_8BIT_DAC) {
                            update_all_dac_entries();
                        }
                        vga.vbe_regs[8] = 0;
                        vga.vbe_regs[9] = 0;
                        vga.vbe_regs[6] = vga.total_width;
                        vga.vbe_regs[7] = vga.total_height;
                    }
                    break;
                case 5:
                    data <<= 16;
                    vga.vbe_regs[5] = data;
                    break;
                case 6: {
                    int bpp         = (vga.vbe_regs[3] + 7) >> 3;
                    vga.vbe_regs[6] = data;
                    if (bpp)
                        vga.vbe_regs[7] = vga.vram_size / bpp;
                    else
                        vga.vbe_regs[7] = 1;
                    break;
                }
                case 7:
                    vga.vbe_regs[7] = data;
                    break;
                case 8 ... 9:
                    vga.vbe_regs[vga.vbe_index] = data;
                    break;
                default:
                    VGA_FATAL("Unknown VBE register: %d\n", vga.vbe_index);
            }
            break;
        case 0x3C0:
            if (!(vga.attr_index & 0x80)) {
                diffxor        = (vga.attr_index ^ data);
                vga.attr_index = data & 0x7F;
                if (diffxor & 0x20)
                    vga_change_renderer();
                vga.attr_index = data & 0x7F;
            } else {
                uint8_t index = vga.attr_index & 0x1F;
                diffxor       = vga.attr[index] ^ data;
                if (diffxor) {
                    vga.attr[index] = data;
                    switch (index) {
                        case 0 ... 15:
                            if (diffxor & 0x3F)
                                vga_change_attr_cache(index);
                            break;
                        case 16:
                            if (diffxor & ((1 << 0) | (1 << 6)))
                                vga_change_renderer();
                            if (diffxor & 0x80)
                                for (int i = 0; i < 16; i++)
                                    vga_change_attr_cache(i);
                            if (diffxor & ((1 << 2) | (1 << 3) | (1 << 5)))
                                vga_complete_redraw();

                            break;
                        case 17:

                            break;
                        case 18:
                            vga.attr[18] &= 0x0F;
                            break;
                        case 19:
                            if (vga.gfx[5] & 0x40)
                                vga.pixel_panning = data >> 1 & 3;
                            else
                                vga.pixel_panning = (data & 7) + (vga.char_width & 1);
                            break;
                        case 20:
                            if (diffxor & 15)
                                for (int i = 0; i < 16; i++)
                                    vga_change_attr_cache(i);
                            break;
                    }
                }
            }
            vga.attr_index ^= 0x80;
            break;
        case 0x3C2:
            vga.misc = data;
            break;
        case 0x3B8:
        case 0x3BF:
        case 0x3C3:
        case 0x3DA:
        case 0x3D8:
        case 0x3CD:
            break;
        case 0x3C4:
            vga.seq_index = data & 7;
            break;
        case 0x3C5: {
            const uint8_t mask[8] = {MASK(0b00000000), MASK(0b11000010), MASK(0b11110000), MASK(0b11000000),
                                     MASK(0b11110001), MASK(0b11111111), MASK(0b11111111), MASK(0b11111111)};
            data &= mask[vga.seq_index];
            diffxor = vga.seq[vga.seq_index] ^ data;
            if (diffxor) {
                vga.seq[vga.seq_index] = data;
                switch (vga.seq_index) {
                    case 0:
                        break;
                    case 1:
                        if (diffxor & 0x20)
                            vga_change_renderer();
                        if (diffxor & 0x08) {
                            vga_change_renderer();
                            vga_update_size();
                        }
                        if (diffxor & 0x01) {
                            vga.char_width = 9 ^ (data & 1);
                            vga_update_size();
                            vga_complete_redraw();
                        }
                        break;
                    case 2:
                        break;
                    case 3:
                        vga.character_map[0] = vga_char_map_address((data >> 5 & 1) | (data >> 1 & 6));
                        vga.character_map[1] = vga_char_map_address((data >> 4 & 1) | (data << 1 & 6));
                        break;
                    case 4:
                        if (diffxor & 0b1100)
                            vga_update_mem_access();
                        break;
                }
            }
        } break;
        case 0x3C6:
            vga.dac_mask = data;
            vga_complete_redraw();
            break;
        case 0x3C7:
            vga.dac_read_address = data;
            vga.dac_color        = 0;
            break;
        case 0x3C8:
            vga.dac_address = data;
            vga.dac_color   = 0;
            break;
        case 0x3C9:
            vga.dac_state                                     = 3;
            vga.dac[(vga.dac_address << 2) | vga.dac_color++] = data;
            if (vga.dac_color == 3) {
                update_one_dac_entry(vga.dac_address);
                vga.dac_address++;
                vga.dac_color = 0;
            }
            break;
        case 0x3CE:
            vga.gfx_index = data & 15;
            break;
        case 0x3CF: {
            const uint8_t mask[16] = {
                MASK(0b11110000), MASK(0b11110000), MASK(0b11110000), MASK(0b11100000),
                MASK(0b11111100), MASK(0b10000100), MASK(0b11110000), MASK(0b11110000),
                MASK(0b00000000), MASK(0b11111111), MASK(0b00001000), MASK(0b00000000),
                MASK(0b11111111), MASK(0b11111111), MASK(0b11111111), MASK(0b11111111),
            };

            data &= mask[vga.gfx_index];
            diffxor = vga.gfx[vga.gfx_index] ^ data;

            if (diffxor) {
                vga.gfx[vga.gfx_index] = data;
                switch (vga.gfx_index) {
                    case 0:
                        break;
                    case 1:
                        break;
                    case 2:
                        break;
                    case 3:
                        break;
                    case 4:
                        break;
                    case 5:
                        if (diffxor & (3 << 5))
                            vga_change_renderer();
                        if (diffxor & ((1 << 3) | (1 << 4) | 3))
                            vga_update_mem_access();
                        break;
                    case 6:
                        switch (data >> 2 & 3) {
                            case 0:
                                vga.vram_window_base = 0xA0000;
                                vga.vram_window_size = 0x20000;
                                break;
                            case 1:
                                vga.vram_window_base = 0xA0000;
                                vga.vram_window_size = 0x10000;
                                break;
                            case 2:
                                vga.vram_window_base = 0xB0000;
                                vga.vram_window_size = 0x8000;
                                break;
                            case 3:
                                vga.vram_window_base = 0xB8000;
                                vga.vram_window_size = 0x8000;
                                break;
                        }
                        if (diffxor & 1)
                            vga_change_renderer();
                        break;
                    case 7:
                        break;
                    case 8:
                        break;
                }
            }
            break;
        }
        case 0x3D4:
        case 0x3B4:
            vga.crt_index = data;
            break;
        case 0x3D5:
        case 0x3B5: {
            static uint8_t mask[64] = {
                MASK(0b00000000), MASK(0b00000000), MASK(0b00000000), MASK(0b00000000), MASK(0b00000000),
                MASK(0b00000000), MASK(0b00000000), MASK(0b00000000), MASK(0b10000000), MASK(0b00000000),
                MASK(0b11000000), MASK(0b10000000), MASK(0b00000000), MASK(0b00000000), MASK(0b00000000),
                MASK(0b00000000), MASK(0b00000000), MASK(0b00110000), MASK(0b00000000), MASK(0b00000000),
                MASK(0b10000000), MASK(0b00000000), MASK(0b10000000), MASK(0b00010000), MASK(0b00000000)};
            if (vga.crt_index > 0x3F)
                break;
            data &= mask[vga.crt_index];
            diffxor = (data ^ vga.crt[vga.crt_index]) & mask[vga.crt_index];
            if (diffxor) {
                vga.crt[vga.crt_index] = data | (vga.crt[vga.crt_index] & ~mask[vga.crt_index]);
                switch (vga.crt_index) {
                    case 1:
                        vga_update_size();
                        break;
                    case 2:
                        vga_update_size();
                        break;
                    case 7:
                        vga_update_size();
                        break;
                    case 9:
                        if (diffxor & 0x20)
                            vga_update_size();
                        break;
                    case 0x11:
                        if (diffxor & 0x80) {
                            uint8_t fill_value = (int8_t)(vga.crt[0x11] ^ 0x80) >> 7;
                            for (int i = 0; i < 8; i++)
                                mask[i] = fill_value;
                            mask[7] &= ~0x10;
                            data &= mask[vga.crt_index];
                        }
                        break;
                    case 0x12:
                        vga_update_size();
                        break;
                    case 0x15:
                        vga_update_size();
                        break;
                }
            }
            break;
        }
        default:
            VGA_LOG("VGA write: 0x%x [data: 0x%02x]\n", port, data);
    }
}
static uint32_t vga_read(uint32_t port)
{
    if ((port >= 0x3B0 && port <= 0x3BF && (vga.misc & 1)) || (port >= 0x3D0 && port <= 0x3DF && !(vga.misc & 1))) {
        return -1;
    }
    switch (port) {
        case 0x1CE:
            return vga.vbe_index;
        case 0x1CF:
            switch (vga.vbe_index) {
                case 0:
                    return vga.vbe_version;
                case 1 ... 3:
                    if (vga.vbe_enable & VBE_DISPI_GETCAPS)
                        return vbe_maximums[vga.vbe_index - 1];
                    else
                        return vga.vbe_regs[vga.vbe_index];
                    break;
                case 4:
                    return vga.vbe_enable & (VBE_DISPI_ENABLED | VBE_DISPI_GETCAPS | VBE_DISPI_8BIT_DAC);
                case 5:
                    return vga.vbe_regs[5] >> 16;
                case 6:
                    return vga.vbe_regs[6];
                case 7:
                    return vga.vbe_regs[7];
                case 8 ... 9:
                    return vga.vbe_regs[vga.vbe_index];
                case 10:
                    return vga.vram_size >> 16;
                default:
                    VGA_FATAL("VBE read: %d\n", vga.vbe_index);
            }
            break;
        case 0x3C0:
            return vga.attr_index;
        case 0x3C1:
            return vga.attr[vga.attr_index & 0x1F];
        case 0x3C2:
            return vga.misc;
        case 0x3C4:
            return vga.seq_index;
        case 0x3C5:
            return vga.seq[vga.seq_index];
        case 0x3C6:
            return vga.dac_mask;
        case 0x3C7:
            return vga.dac_state;
        case 0x3C8:
            return vga.dac_address;
        case 0x3C9: {
            vga.dac_state = 0;
            uint8_t data  = vga.dac[(vga.dac_read_address << 2) | (vga.dac_color++)];
            if (vga.dac_color == 3) {
                vga.dac_read_address++;
                vga.dac_color = 0;
            }
            return data;
        }
        case 0x3CC:
            return vga.misc;
        case 0x3CE:
            return vga.gfx_index;
        case 0x3CF:
            return vga.gfx[vga.gfx_index];
        case 0x3B8:
        case 0x3D8:
        case 0x3CD:
            return -1;
        case 0x3BA:
        case 0x3DA:
            vga.status[1] ^= 9;
            vga.attr_index &= ~0x80;
            return vga.status[1];
        case 0x3B5:
        case 0x3D5:
            return vga.crt[vga.crt_index];
        default:
            return -1;
    }
}
static inline uint8_t bpp4_to_offset(uint8_t i, uint8_t j, uint8_t k)
{
    return ((i & (0x80 >> j)) != 0) ? 1 << k : 0;
}
void vga_update(void)
{
    framectr                           = (framectr + 1) & 0x3F;
    int          scanlines_to_update   = vga.scanlines_to_update;
    unsigned int cursor_scanline_start = 0, cursor_scanline_end = 0, cursor_enabled = 0, cursor_address = 0,
                 underline_location = 0, line_graphics = 0;
    unsigned int enableMask = 0, address_bit_mapping = 0;
    unsigned int offset_between_lines = (((!vga.crt[0x13]) << 8 | vga.crt[0x13]) * 2) << 2;
    switch (vga.renderer & ~1) {
        case BLANK_RENDERER:
            break;
        case ALPHANUMERIC_RENDERER:
            cursor_scanline_start = vga.crt[0x0A] & 0x1F;
            cursor_scanline_end   = vga.crt[0x0B] & 0x1F;
            cursor_enabled        = (vga.crt[0x0B] & 0x20) || (framectr >= 0x20);
            cursor_address        = (vga.crt[0x0E] << 8 | vga.crt[0x0F]) << 2;
            underline_location    = vga.crt[0x14] & 0x1F;
            line_graphics         = vga.char_width == 9 ? ((vga.attr[0x10] & 4) ? 0xE0 : 0) : 0;
            break;
        case RENDER_4BPP:
            enableMask          = vga.attr[0x12] & 15;
            address_bit_mapping = vga.crt[0x17] & 1;
            break;
        case RENDER_16BPP:
            offset_between_lines = vga.total_width * 2;
            break;
        case RENDER_24BPP:
            offset_between_lines = vga.total_width * 3;
            break;
        case RENDER_32BPP:
            offset_between_lines = vga.total_width * 4;
            break;
    }
    if (!vga.memory_modified)
        return;
    vga.memory_modified &= ~(1 << (vga.current_scanline != 0));

    uint32_t total_scanlines_drawn = 0;
    while (scanlines_to_update--) {
        total_scanlines_drawn++;
        if ((vga.current_scanline & 1) && (vga.crt[9] & 0x80)) {
            memcpy(&vga.framebuffer[vga.framebuffer_offset], &vga.framebuffer[vga.framebuffer_offset - vga.total_width],
                   vga.total_width);
        } else {
            if (vga.current_scanline < vga.total_height) {
                uint32_t fboffset  = vga.framebuffer_offset;
                uint32_t vram_addr = vga.vram_addr;
                switch (vga.renderer) {
                    case BLANK_RENDERER:
                    case BLANK_RENDERER | 1:
                        for (unsigned int i = 0; i < vga.total_width; i++) {
                            vga.framebuffer[fboffset + i] = 255 << 24;
                        }
                        break;
                    case ALPHANUMERIC_RENDERER: {
                        for (unsigned int i = 0; i < vga.total_width; i += vga.char_width, vram_addr += 4) {
                            uint8_t  character = vga.vram[vram_addr << 1];
                            uint8_t  attribute = vga.vram[(vram_addr << 1) + 1];
                            uint8_t  font      = vga.vram[((vga.character_scanline + character * 32 +
                                                      vga.character_map[~attribute >> 3 & 1])
                                                     << 2) +
                                                    2];
                            uint32_t fg = attribute & 15, bg = attribute >> 4 & 15;
                            if (cursor_enabled && vram_addr == cursor_address) {
                                if ((vga.character_scanline >= cursor_scanline_start) &&
                                    (vga.character_scanline <= cursor_scanline_end)) {
                                    bg = fg;
                                }
                            }
                            if ((vga.attr[0x10] & 8) && (framectr >= 32)) {
                                bg &= 7;
                                if (attribute & 0x80)
                                    fg = bg;
                            }
                            if ((attribute & 0b01110111) == 1) {
                                if (vga.character_scanline == underline_location)
                                    bg = fg;
                            }
                            fg                            = vga.dac_palette[vga.dac_mask & vga.attr_palette[fg]];
                            bg                            = vga.dac_palette[vga.dac_mask & vga.attr_palette[bg]];
                            uint32_t xorvec               = fg ^ bg;
                            vga.framebuffer[fboffset + 0] = ((xorvec & -(font >> 7))) ^ bg;
                            vga.framebuffer[fboffset + 1] = ((xorvec & -(font >> 6 & 1))) ^ bg;
                            vga.framebuffer[fboffset + 2] = ((xorvec & -(font >> 5 & 1))) ^ bg;
                            vga.framebuffer[fboffset + 3] = ((xorvec & -(font >> 4 & 1))) ^ bg;
                            vga.framebuffer[fboffset + 4] = ((xorvec & -(font >> 3 & 1))) ^ bg;
                            vga.framebuffer[fboffset + 5] = ((xorvec & -(font >> 2 & 1))) ^ bg;
                            vga.framebuffer[fboffset + 6] = ((xorvec & -(font >> 1 & 1))) ^ bg;
                            vga.framebuffer[fboffset + 7] = ((xorvec & -(font >> 0 & 1))) ^ bg;
                            if ((character & line_graphics) == 0xC0) {
                                vga.framebuffer[fboffset + 8] = ((xorvec & -(font >> 0 & 1))) ^ bg;
                            } else if (vga.char_width == 9)
                                vga.framebuffer[fboffset + 8] = bg;
                            fboffset += vga.char_width;
                        }
                        break;
                    }
                    case MODE_13H_RENDERER: {
                        for (unsigned int i = 0; i < vga.total_width; i += 4, vram_addr += 16) {
                            for (int j = 0; j < 4; j++) {
                                vga.framebuffer[fboffset + j] = vga.dac_palette[vga.vram[vram_addr | j] & vga.dac_mask];
                            }
                            fboffset += 4;
                        }
                        break;
                    }
                    case MODE_13H_RENDERER | 1:
                        for (unsigned int i = 0; i < vga.total_width; i += 8, vram_addr += 4) {
                            for (int j = 0, k = 0; j < 4; j++, k += 2) {
                                vga.framebuffer[fboffset + k] = vga.framebuffer[fboffset + k + 1] =
                                    vga.dac_palette[vga.vram[vram_addr | j] & vga.dac_mask];
                            }
                            fboffset += 8;
                        }
                        break;
                    case RENDER_4BPP: {
                        uint32_t addr = vram_addr;
                        if (vga.character_scanline & address_bit_mapping)
                            addr |= 0x8000;
                        uint8_t p0 = vga.vram[addr | 0];
                        uint8_t p1 = vga.vram[addr | 1];
                        uint8_t p2 = vga.vram[addr | 2];
                        uint8_t p3 = vga.vram[addr | 3];
                        for (unsigned int x = 0, px = vga.current_pixel_panning; x < vga.total_width;
                             x++, fboffset++, px++) {
                            if (px > 7) {
                                px = 0;
                                addr += 4;
                                p0 = vga.vram[addr | 0];
                                p1 = vga.vram[addr | 1];
                                p2 = vga.vram[addr | 2];
                                p3 = vga.vram[addr | 3];
                            }
                            int pixel = bpp4_to_offset(p0, px, 0) | bpp4_to_offset(p1, px, 1) |
                                        bpp4_to_offset(p2, px, 2) | bpp4_to_offset(p3, px, 3);
                            pixel &= enableMask;
                            vga.framebuffer[fboffset] = vga.dac_palette[vga.dac_mask & vga.attr_palette[pixel]];
                        }
                        break;
                    }
                    case RENDER_4BPP | 1: {
                        uint32_t addr = vram_addr;
                        uint8_t  p0   = vga.vram[addr | 0];
                        uint8_t  p1   = vga.vram[addr | 1];
                        uint8_t  p2   = vga.vram[addr | 2];
                        uint8_t  p3   = vga.vram[addr | 3];
                        for (unsigned int x = 0, px = vga.current_pixel_panning; x < vga.total_width;
                             x += 2, fboffset += 2, px++) {
                            if (px > 7) {
                                px = 0;
                                addr += 4;
                                p0 = vga.vram[addr | 0];
                                p1 = vga.vram[addr | 1];
                                p2 = vga.vram[addr | 2];
                                p3 = vga.vram[addr | 3];
                            }
                            int pixel = bpp4_to_offset(p0, px, 0) | bpp4_to_offset(p1, px, 1) |
                                        bpp4_to_offset(p2, px, 2) | bpp4_to_offset(p3, px, 3);
                            pixel &= enableMask;
                            uint32_t result               = vga.dac_palette[vga.dac_mask & vga.attr_palette[pixel]];
                            vga.framebuffer[fboffset]     = result;
                            vga.framebuffer[fboffset + 1] = result;
                        }
                        break;
                    }
                    case RENDER_32BPP:
                        if (!vga.vbe_scanlines_modified[vga.current_scanline])
                            break;
                        for (unsigned int i = 0; i < vga.total_width; i++, vram_addr += 4) {
                            vga.framebuffer[fboffset++] = *((uint32_t *)&vga.vram[vram_addr]) | 0xFF000000;
                        }
                        vga.vbe_scanlines_modified[vga.current_scanline] = 0;
                        break;
                    case RENDER_8BPP:
                        if (!vga.vbe_scanlines_modified[vga.current_scanline])
                            break;
                        for (unsigned int i = 0; i < vga.total_width; i++, vram_addr++)
                            vga.framebuffer[fboffset++] = vga.dac_palette[vga.vram[vram_addr]];
                        vga.vbe_scanlines_modified[vga.current_scanline] = 0;
                        break;
                    case RENDER_16BPP:
                        if (!vga.vbe_scanlines_modified[vga.current_scanline])
                            break;
                        for (unsigned int i = 0; i < vga.total_width; i++, vram_addr += 2) {
                            uint16_t word = *((uint16_t *)&vga.vram[vram_addr]);
                            int      red = word >> 11 << 3, green = (word >> 5 & 63) << 2, blue = (word & 31) << 3;
                            vga.framebuffer[fboffset++] = red << 16 | green << 8 | blue << 0 | 0xFF000000;
                        }
                        vga.vbe_scanlines_modified[vga.current_scanline] = 0;
                        break;
                    case RENDER_24BPP:
                        if (!vga.vbe_scanlines_modified[vga.current_scanline])
                            break;
                        for (unsigned int i = 0; i < vga.total_width; i++, vram_addr += 3) {
                            uint8_t blue = vga.vram[vram_addr], green = vga.vram[vram_addr + 1],
                                    red = vga.vram[vram_addr + 2];

                            vga.framebuffer[fboffset++] = (blue) | (green << 8) | (red << 16) | 0xFF000000;
                        }
                        vga.vbe_scanlines_modified[vga.current_scanline] = 0;
                        break;
                }
                if ((vga.crt[9] & 0x1F) == vga.character_scanline) {
                    vga.character_scanline = 0;
                    vga.vram_addr += offset_between_lines;
                } else
                    vga.character_scanline++;
            }
        }
        vga.current_scanline = (vga.current_scanline + 1) & 0x0FFF;
        vga.framebuffer_offset += vga.total_width;
        if (vga.current_scanline >= vga.total_height) {
            display_update(0, vga.total_height);
            vga_complete_redraw();
            total_scanlines_drawn = 0;
        }
    }
}
static void vga_reset(void)
{
    vga.misc             = 0;
    vga.seq_index        = 0;
    vga.char_width       = 9;
    vga.character_map[0] = vga.character_map[1] = 0;
    vga_complete_redraw();
}
static void expand32_alt(uint8_t *ptr, int v4)
{
    ptr[0] = v4 & 1 ? 0xFF : 0;
    ptr[1] = v4 & 2 ? 0xFF : 0;
    ptr[2] = v4 & 4 ? 0xFF : 0;
    ptr[3] = v4 & 8 ? 0xFF : 0;
}
static uint32_t expand32(int v4)
{
    uint32_t r = v4 & 1 ? 0xFF : 0;
    r |= v4 & 2 ? 0xFF00 : 0;
    r |= v4 & 4 ? 0xFF0000 : 0;
    r |= v4 & 8 ? 0xFF000000 : 0;
    return r;
}
static uint32_t b8to32(uint8_t x)
{
    uint32_t y = x;
    y |= y << 8;
    return y | (y << 16);
}
static uint32_t vga_mem_readb(uint32_t addr)
{
    if (vga.vbe_enable & VBE_DISPI_ENABLED) {
        if (addr & 0x80000000)
            return vga.vram[addr - VBE_LFB_BASE];
        else
            return vga.vram[vga.vbe_regs[5] + (addr & 0x1FFFF)];
    }
    addr -= vga.vram_window_base;
    if (addr > vga.vram_window_size) {
        return -1;
    }
    vga.latch32   = ((uint32_t *)(vga.vram))[addr];
    int     plane = 0, plane_addr = -1;
    uint8_t color_dont_care[4], color_compare[4];
    switch (vga.read_access) {
        case CHAIN4:
            plane      = addr & 3;
            plane_addr = addr >> 2;
            break;
        case ODDEVEN:
            plane      = (addr & 1) | (vga.gfx[4] & 2);
            plane_addr = addr & ~1;
            break;
        case NORMAL:
            plane      = vga.gfx[4] & 3;
            plane_addr = addr;
            break;
        case READMODE_1:
            expand32_alt(color_dont_care, vga.gfx[7]);
            expand32_alt(color_compare, vga.gfx[2]);
            return ~(((vga.latch8[0] & color_dont_care[0]) ^ color_compare[0]) |
                     ((vga.latch8[1] & color_dont_care[1]) ^ color_compare[1]) |
                     ((vga.latch8[2] & color_dont_care[2]) ^ color_compare[2]) |
                     ((vga.latch8[3] & color_dont_care[3]) ^ color_compare[3]));
    }
    if (plane_addr > 65536)
        VGA_FATAL("Reading outside plane bounds\n");
    uint8_t data = vga.vram[plane | (plane_addr << 2)];

    return data;
}
static uint8_t alu_rotate(uint8_t value)
{
    uint8_t rotate_count = vga.gfx[3] & 7;
    return ((value >> rotate_count) | (value << (8 - rotate_count))) & 0xFF;
}
static inline uint32_t do_mask(uint32_t value, uint32_t mask, int mask_enabled)
{
    static uint32_t lut32[9] = {0, 0xFF, 0xFF00, 0, 0xFF0000, 0, 0, 0, 0xFF000000};
    uint32_t        _xor     = value ^ mask;
    DO_MASK(1);
    DO_MASK(2);
    DO_MASK(4);
    DO_MASK(8);
    return _xor;
}
static void vga_mem_writeb(uint32_t addr, uint32_t data)
{
    if (vga.vbe_enable & VBE_DISPI_ENABLED) {
        uint32_t vram_offset;
        if (addr & 0x80000000) {
            vram_offset = addr - VBE_LFB_BASE;
            if (vga.vbe_enable & VBE_DISPI_LFB_ENABLED)
                vga.vram[vram_offset] = data;
            else
                return;
        } else {
            vram_offset = vga.vbe_regs[5] + (addr & 0x1FFFF);
            if (vga.vbe_enable & VBE_DISPI_LFB_ENABLED)
                return;
            else
                vga.vram[vram_offset] = data;
        }
        uint32_t scanline = vram_offset / (vga.total_width * ((vga.vbe_regs[3] + 7) >> 3));
        if (scanline < vga.total_height)
            vga.vbe_scanlines_modified[scanline] = 1;
        vga.memory_modified = 1;
        return;
    }
    addr -= vga.vram_window_base;
    if (addr > vga.vram_window_size) {
        return;
    }
    int plane = 0, plane_addr = -1;
    switch (vga.write_access) {
        case CHAIN4:
            plane      = 1 << (addr & 3);
            plane_addr = addr >> 2;
            break;
        case ODDEVEN:
            plane      = 5 << (addr & 1);
            plane_addr = addr & ~1;
            break;
        case NORMAL:
            plane      = 15;
            plane_addr = addr;
            break;
    }
    uint32_t data32 = data, and_value = 0xFFFFFFFF;
    int      run_alu = 1;
    switch (vga.write_mode) {
        case 0:
            data32 = b8to32(alu_rotate(data));
            data32 = do_mask(data32, expand32(vga.gfx[0]), vga.gfx[1]);
            break;
        case 1:
            data32  = vga.latch32;
            run_alu = 0;
            break;
        case 2:
            data32 = expand32(data);
            break;
        case 3:
            and_value = b8to32(alu_rotate(data));
            data32    = expand32(vga.gfx[0]);
            break;
    }
    if (run_alu) {
        uint32_t mask = b8to32(vga.gfx[8]) & and_value;
        switch (vga.gfx[3] & 0x18) {
            case 0:
                data32 = (data32 & mask) | (vga.latch32 & ~mask);
                break;
            case 0x08:
                data32 = ((data32 & vga.latch32) & mask) | (vga.latch32 & ~mask);
                break;
            case 0x10:
                data32 = ((data32 | vga.latch32) & mask) | (vga.latch32 & ~mask);
                break;
            case 0x18:
                data32 = ((data32 ^ vga.latch32) & mask) | (vga.latch32 & ~mask);
                break;
        }
    }

    plane &= vga.seq[2];
    uint32_t *vram_ptr            = (uint32_t *)&vga.vram[plane_addr << 2];
    *vram_ptr                     = do_mask(*vram_ptr, data32, plane);
    uint32_t offs                 = (plane_addr << 2) - (((vga.crt[0x0C] << 8) | vga.crt[0x0D]) << 2),
             offset_between_lines = (((!vga.crt[0x13]) << 8 | vga.crt[0x13]) * 2) << 2;
    unsigned int scanline         = offs / offset_between_lines;
    if (vga.total_height > scanline) {
        switch (vga.renderer >> 1) {
            case MODE_13H_RENDERER >> 1:
                vga.vbe_scanlines_modified[scanline] = 1;
                break;
            case RENDER_4BPP >> 1:
                vga.vbe_scanlines_modified[scanline] = 1;
                break;
        }
    }
    vga.memory_modified = 3;
}
static int vga_pci_write(uint8_t *ptr, uint8_t addr, uint8_t data)
{
    switch (addr) {
        case 0x10:
            ptr[addr] = (ptr[addr] & 0x0F) | (data & 0xF0);
            return 0;
        case 0x13:
            break;
        case 0x33: {
            uint32_t new_mmio = ptr[0x30] | (ptr[0x31] << 8) | (ptr[0x32] << 16) | (data << 24);
            new_mmio &= ~1;
            uint32_t data;
            if (new_mmio == 0xFFFFFFFE) {
                data = -vga.rom_size;
            } else {
                data = 0xFEB00000;
            }
            ptr[0x30] = data;
            ptr[0x31] = data >> 8;
            ptr[0x32] = data >> 16;
            ptr[0x33] = data >> 24;
            vga.mem   = (uint8_t *)cpu.cpu_get_ram_ptr();
            io_remap_mmio_read(vga.vgabios_addr, new_mmio);
            vga.vgabios_addr = new_mmio;
            break;
        }
    }
    return 0;
}
static uint32_t vga_rom_readb(uint32_t addr)
{
    return vga.rom[(addr - vga.vgabios_addr) & 0xFFFF];
}
static void vga_rom_writeb(uint32_t addr, uint32_t data)
{
    UNUSED(addr | data);
}
static void vga_pci_init(struct loaded_file *vgabios)
{
    uint8_t *dev = (uint8_t *)pci_create_device(0, 2, 0, vga_pci_write);
    pci_copy_default_configuration(dev, (void *)pci_config_space, 16);
    dev[0x10] = 8;
    io_register_mmio_read(vga.vgabios_addr = 0xFEB00000, 0x20000, vga_rom_readb, NULL, NULL);
    io_register_mmio_write(vga.vgabios_addr, 0x20000, vga_rom_writeb, NULL, NULL);
    vga.rom = (uint8_t *)calloc(1, 65536);
    memcpy(vga.rom, vgabios->data, vgabios->length & 65535);
    vga.rom_size = vgabios->length;
    dev[0x30]    = vga.vgabios_addr;
    dev[0x31]    = vga.vgabios_addr >> 8;
    dev[0x32]    = vga.vgabios_addr >> 16;
    dev[0x33]    = vga.vgabios_addr >> 24;
}
void vga_init(struct pc_settings *pc)
{
    io_register_reset(vga_reset);
    io_register_read(0x3B0, 48, vga_read, NULL, NULL);
    io_register_write(0x3B0, 48, vga_write, NULL, NULL);
    if (pc->vbe_enabled) {
        io_register_read(0x1CE, 2, NULL, vga_read, NULL);
        io_register_write(0x1CE, 2, NULL, vga_write, NULL);
    }
    state_register(vga_state);
    io_register_mmio_read(0xA0000, 0x20000 - 1, vga_mem_readb, NULL, NULL);
    io_register_mmio_write(0xA0000, 0x20000 - 1, vga_mem_writeb, NULL, NULL);
    int memory_size = pc->vga_memory_size < (256 << 10) ? 256 << 10 : pc->vga_memory_size;
    io_register_mmio_read(VBE_LFB_BASE, memory_size, vga_mem_readb, NULL, NULL);
    io_register_mmio_write(VBE_LFB_BASE, memory_size, vga_mem_writeb, NULL, NULL);
    vga.vram_size = memory_size;
    vga_alloc_mem();
    if (pc->pci_vga_enabled) {
        vga_pci_init(&pc->vgabios);
    }
}
void *vga_get_raw_vram(void)
{
    return vga.vram;
}