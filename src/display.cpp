#include "display.h"
#include "devices.h"
#include "util.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

#define DISPLAY_LOG(x, ...) LOG("DISPLAY", x, ##__VA_ARGS__)
#define DISPLAY_FATAL(x, ...)                                                                                          \
    do {                                                                                                               \
        DISPLAY_LOG(x, ##__VA_ARGS__);                                                                                 \
        ABORT();                                                                                                       \
    } while (0)
#define KEYMOD_INVALID -1

static SDL_Window  *window  = NULL;
static SDL_Surface *surface = NULL;
static SDL_Surface *screen  = NULL;

static void *surface_pixels;
static int   input_captured = 0;
static int   resized        = 0;
static int   h, w, mouse_enabled = 0, mhz_rating = -1;


void *display_get_pixels(void)
{
    return surface_pixels;
}
void display_update_cycles(int cycles_elapsed, int us)
{
    mhz_rating = (int)((double)cycles_elapsed / (double)us);
}
void display_set_resolution(int width, int height)
{
    if (width < 10 || height < 10)
        return;

    resized = 1;
    if (w != width || h != height) {
        if (surface_pixels)
            free(surface_pixels);
        surface_pixels = malloc(width * height * 4);
        if (surface)
            SDL_FreeSurface(surface);
        if (screen)
            SDL_FreeSurface(screen);
        if (window)
            SDL_DestroyWindow(window);
        window =
            SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
        screen  = SDL_GetWindowSurface(window);
        surface = SDL_CreateRGBSurfaceFrom(surface_pixels, width, height, 32, width * 4, 0x00ff0000, 0x0000ff00,
                                           0x000000ff, 0xff000000);

        w = width;
        h = height;
    }
}
void display_update(int scanline_start, int scanlines)
{
    if (!resized)
        return;
    if ((w == 0) || (h == 0))
        return;
    if ((scanline_start + scanlines) > h) {
        printf("%d x %d [%d %d]\n", w, h, scanline_start, scanlines);
        ABORT();
    } else {
        SDL_UpdateWindowSurface(window);
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = w;
        rect.h = h;
        SDL_BlitSurface(surface, &rect, screen, &rect);
    }
}
static void display_mouse_capture_update(int y)
{
    // input_captured = y;
    // SDL_WM_GrabInput(y);
    // SDL_ShowCursor(SDL_TRUE ^ y);
    // mouse_enabled = y;
}
void display_release_mouse(void)
{
    display_mouse_capture_update(0);
}
static int sdl_keysym_to_scancode(int sym)
{
    int n;
    switch (sym) {
        case SDLK_0 ... SDLK_9:
            n = sym - SDLK_0;
            if (!n)
                n = 10;
            return n + 1;
        case SDLK_ESCAPE:
            if (mouse_enabled) {
                display_mouse_capture_update(0);
                return KEYMOD_INVALID;
            }
            return 1;
        case SDLK_EQUALS:
            return 0x0D;
        case SDLK_RETURN:
            return 0x1C;
        case SDLK_a:
            return 0x1E;
        case SDLK_b:
            return 0x30;
        case SDLK_c:
            return 0x2E;
        case SDLK_d:
            return 0x20;
        case SDLK_e:
            return 0x12;
        case SDLK_f:
            return 0x21;
        case SDLK_g:
            return 0x22;
        case SDLK_h:
            return 0x23;
        case SDLK_i:
            return 0x17;
        case SDLK_j:
            return 0x24;
        case SDLK_k:
            return 0x25;
        case SDLK_l:
            return 0x26;
        case SDLK_m:
            return 0x32;
        case SDLK_n:
            return 0x31;
        case SDLK_o:
            return 0x18;
        case SDLK_p:
            return 0x19;
        case SDLK_q:
            return 0x10;
        case SDLK_r:
            return 0x13;
        case SDLK_s:
            return 0x1F;
        case SDLK_t:
            return 0x14;
        case SDLK_u:
            return 0x16;
        case SDLK_v:
            return 0x2F;
        case SDLK_w:
            return 0x11;
        case SDLK_x:
            return 0x2D;
        case SDLK_y:
            return 0x15;
        case SDLK_z:
            return 0x2C;
        case SDLK_BACKSPACE:
            return 0x0E;
        case SDLK_LEFT:
            return 0xE04B;
        case SDLK_DOWN:
            return 0xE050;
        case SDLK_RIGHT:
            return 0xE04D;
        case SDLK_UP:
            return 0xE048;
        case SDLK_SPACE:
            return 0x39;
        case SDLK_PAGEUP:
            return 0xE04F;
        case SDLK_PAGEDOWN:
            return 0xE051;
        case SDLK_DELETE:
            return 0xE053;
        case SDLK_F1 ... SDLK_F12:
            return 0x3B + (sym - SDLK_F1);
        case SDLK_SLASH:
            return 0x35;
        case SDLK_LALT:
            return 0x38;
        case SDLK_LCTRL:
            return 0x1D;
        case SDLK_LSHIFT:
            return 0x2A;
        case SDLK_RSHIFT:
            return 0x36;
        case SDLK_SEMICOLON:
            return 0x27;
        case SDLK_BACKSLASH:
            return 0x2B;
        case SDLK_COMMA:
            return 0x33;
        case SDLK_PERIOD:
            return 0x34;
        case SDLK_MINUS:
            return 0x0C;
        case SDLK_RIGHTBRACKET:
            return 0x1A;
        case SDLK_LEFTBRACKET:
            return 0x1B;
        case SDLK_QUOTE:
            return 0x28;
        case SDLK_BACKQUOTE:
            return 0x29;
        case SDLK_TAB:
            return 0x0F;
        default:
            printf("Unknown keysym: %d\n", sym);
            return KEYMOD_INVALID;
    }
}
static inline void display_kbd_send_key(int k)
{
    if (k == KEYMOD_INVALID)
        return;
    if (k & 0xFF00)
        kbd_add_key(k >> 8);
    kbd_add_key(k & 0xFF);
}
static inline void send_keymod_scancode(int k, int _or)
{
    if (k & KMOD_ALT) {
        display_kbd_send_key(0xE038 | _or);
    }
}
void display_handle_events(void)
{
    SDL_Event event;
    int       k;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                printf("QUIT\n");
                exit(0);
                break;
            case SDL_KEYDOWN: {
                send_keymod_scancode(event.key.keysym.mod, 0);
                display_kbd_send_key(sdl_keysym_to_scancode(event.key.keysym.sym));
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                k = event.type == SDL_MOUSEBUTTONDOWN ? MOUSE_STATUS_PRESSED : MOUSE_STATUS_RELEASED;
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        kbd_mouse_down(k, MOUSE_STATUS_NOCHANGE, MOUSE_STATUS_NOCHANGE);
                        break;
                    case SDL_BUTTON_MIDDLE:
                        kbd_mouse_down(MOUSE_STATUS_NOCHANGE, k, MOUSE_STATUS_NOCHANGE);
                        break;
                    case SDL_BUTTON_RIGHT:
                        if (k == MOUSE_STATUS_PRESSED && !mouse_enabled)
                            display_mouse_capture_update(1);
                        else
                            kbd_mouse_down(MOUSE_STATUS_NOCHANGE, MOUSE_STATUS_NOCHANGE, k);
                        break;
                }
                break;
            }
            case SDL_MOUSEMOTION: {
                if (input_captured)
                    kbd_send_mouse_move(event.motion.x, event.motion.y);
                break;
            }
            case SDL_KEYUP: {
                int c = sdl_keysym_to_scancode(event.key.keysym.sym);
                send_keymod_scancode(event.key.keysym.mod, 0x80);
                display_kbd_send_key(c | 0x80);
                break;
            }
        }
    }
}
void display_send_ctrl_alt_del(int down)
{
    down = down ? 0 : 0x80;
    display_kbd_send_key(0x1D | down);
    display_kbd_send_key(0xE038 | down);
    display_kbd_send_key(0xE053 | down);
}
void display_send_scancode(int key)
{
    display_kbd_send_key(key);
}
void display_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE))
        DISPLAY_FATAL("Unable to initialize SDL");
    display_set_resolution(640, 480);
    resized = 0;
}
void display_sleep(int ms)
{
    SDL_Delay(ms);
}
