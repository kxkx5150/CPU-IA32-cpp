#include "util.h"
#include "cpu/cpu.h"
#include "display.h"
#include "state.h"
#include <cstdint>
#include <stdlib.h>

#define QMALLOC_SIZE 1 << 20

extern CPU cpu;


struct aalloc_info
{
    void   *actual_ptr;
    uint8_t data[0];
};

uint32_t ticks_per_second = 50000000;

static void   *qmalloc_data;
static int     qmalloc_usage, qmalloc_size;
static void  **qmalloc_slabs      = NULL;
static int     qmalloc_slabs_size = 0;
static itick_t tick_base;


static void qmalloc_slabs_resize(void)
{
    qmalloc_slabs = (void **)realloc(qmalloc_slabs, qmalloc_slabs_size * sizeof(uint8_t *));
}
void qmalloc_init(void)
{
    if (qmalloc_slabs == NULL) {
        qmalloc_slabs_size = 1;
        qmalloc_slabs      = (void **)malloc(1);
        qmalloc_slabs_resize();
    }
    qmalloc_data                          = malloc(QMALLOC_SIZE);
    qmalloc_usage                         = 0;
    qmalloc_size                          = QMALLOC_SIZE;
    qmalloc_slabs[qmalloc_slabs_size - 1] = qmalloc_data;
}
void *qmalloc(int size, int align)
{
    if (!align)
        align = 4;
    align--;
    qmalloc_usage = (qmalloc_usage + align) & ~align;
    void *ptr     = qmalloc_usage + (uint8_t *)qmalloc_data;
    qmalloc_usage += size;
    if (qmalloc_usage >= qmalloc_size) {
        LOG("QMALLOC", "Creating additional slab\n");
        qmalloc_init();
        return qmalloc(size, align);
    }
    return ptr;
}
void qfree(void)
{
    for (int i = 0; i < qmalloc_slabs_size; i++) {
        free(qmalloc_slabs[i]);
    }
    free(qmalloc_slabs);
    qmalloc_slabs = NULL;
    qmalloc_init();
}
void set_ticks_per_second(uint32_t value)
{
    ticks_per_second = value;
}
void util_state(void)
{
    struct bjson_object *obj = state_obj((char *)"util", 1);
    state_field(obj, 8, (char *)"tick_base", &tick_base);
}
itick_t get_now(void)
{
    return tick_base + cpu.cpu_get_cycles();
}
void add_now(itick_t a)
{
    tick_base += a;
}
void util_debug(void)
{
    display_release_mouse();
    __asm__("int3");
}
void util_abort(void)
{
    display_release_mouse();
    abort();
}
void *aalloc(int size, int align)
{
    int                 adjusted = align - 1;
    uint8_t            *actual   = (uint8_t *)calloc(1, sizeof(uint8_t *) + size + adjusted);
    uint8_t            *ptem     = actual + sizeof(void *) + adjusted;
    uintptr_t           ptr      = (uintptr_t)(ptem) & ~adjusted;
    struct aalloc_info *ai       = (struct aalloc_info *)((uint8_t *)(ptr) - sizeof(void *));
    ai->actual_ptr               = actual;
    return ((uint8_t *)ai) + sizeof(uint8_t *);
}
void afree(void *ptr)
{
    struct aalloc_info *a = (struct aalloc_info *)((uint8_t *)ptr - sizeof(uint8_t *));
    free(a->actual_ptr);
}
