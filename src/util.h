#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x)     (void)(x)
#define MAP_ANONYMOUS 0x20
#define FATAL(component, x, ...)                                                                                       \
    do {                                                                                                               \
        fprintf(stderr, "[" component "] " x, ##__VA_ARGS__);                                                          \
        ABORT();                                                                                                       \
        abort();                                                                                                       \
    } while (0)

#define NOP()                                                                                                          \
    do {                                                                                                               \
    } while (0)

#define ABORT()                util_abort()
#define debugger               util_debug()
#define LOG(component, x, ...) fprintf(stderr, "[" component "] " x, ##__VA_ARGS__)

extern uint32_t  ticks_per_second;
typedef uint64_t itick_t;


void *aalloc(int size, int align);
void  afree(void *ptr);



itick_t get_now(void);
void    add_now(itick_t a);
void    qmalloc_init(void);
void   *qmalloc(int size, int align);
void    qfree(void);
void    util_debug(void);
void    util_abort(void);
void    util_state(void);

#ifdef __cplusplus
}
#endif
#endif