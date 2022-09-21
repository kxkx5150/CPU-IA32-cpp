#include "devices.h"
#include "io.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <stdlib.h>

#define PIT_LOG(fmt, ...) LOG("PIT", fmt, ##__VA_ARGS__)
#define PIT_CLOCK_SPEED   1193182

#define CHAN0   pit.chan[0]
#define CHAN1   pit.chan[1]
#define CHAN2   pit.chan[2]
#define CHAN(n) pit.chan[n]

#define RW_STATE_LSB    1
#define RW_STATE_MSB    2
#define RW_STATE_WORD   3
#define RW_STATE_WORD_2 4

#define MODE_INTERRUPT_ON_TERMINAL_COUNT     0
#define MODE_HARDWARE_RETRIGGERABLE_ONE_SHOT 1
#define MODE_RATE_GENERATOR                  2
#define MODE_SQUARE_WAVE                     3
#define MODE_SOFTWARE_TRIGGERED_STROBE       4
#define MODE_HARDWARE_TRIGGERED_STROBE       5
#define CONTROL_ADDRESS                      3
#define STATUS_LATCHED                       1
#define COUNTER_LATCHED                      2


struct pit_channel
{
    uint32_t count, interim_count;
    int      flipflop;
    int      mode, bcd, gate, rw_mode, rmode, wmode;
    uint8_t  status_latch;
    uint8_t  whats_latched;
    uint16_t counter_latch;
    itick_t  last_load_time, last_irq_time;
    uint32_t period;
    uint32_t pit_last_count;
    int      timer_flipflop;
    int      timer_running;
};
struct pit
{
    int                speaker;
    itick_t            last;
    struct pit_channel chan[3];
};
static struct pit pit;


static void pit_state(void)
{
    struct bjson_object *obj = state_obj((char *)"pit", (18 + 2) * 3);
    state_field(obj, 4, (char *)"pit.chan[0].count", &pit.chan[0].count);
    state_field(obj, 4, (char *)"pit.chan[1].count", &pit.chan[1].count);
    state_field(obj, 4, (char *)"pit.chan[2].count", &pit.chan[2].count);
    state_field(obj, 4, (char *)"pit.chan[0].interim_count", &pit.chan[0].interim_count);
    state_field(obj, 4, (char *)"pit.chan[1].interim_count", &pit.chan[1].interim_count);
    state_field(obj, 4, (char *)"pit.chan[2].interim_count", &pit.chan[2].interim_count);
    state_field(obj, 4, (char *)"pit.chan[0].flipflop", &pit.chan[0].flipflop);
    state_field(obj, 4, (char *)"pit.chan[1].flipflop", &pit.chan[1].flipflop);
    state_field(obj, 4, (char *)"pit.chan[2].flipflop", &pit.chan[2].flipflop);
    state_field(obj, 4, (char *)"pit.chan[0].mode", &pit.chan[0].mode);
    state_field(obj, 4, (char *)"pit.chan[1].mode", &pit.chan[1].mode);
    state_field(obj, 4, (char *)"pit.chan[2].mode", &pit.chan[2].mode);
    state_field(obj, 4, (char *)"pit.chan[0].bcd", &pit.chan[0].bcd);
    state_field(obj, 4, (char *)"pit.chan[1].bcd", &pit.chan[1].bcd);
    state_field(obj, 4, (char *)"pit.chan[2].bcd", &pit.chan[2].bcd);
    state_field(obj, 4, (char *)"pit.chan[0].gate", &pit.chan[0].gate);
    state_field(obj, 4, (char *)"pit.chan[1].gate", &pit.chan[1].gate);
    state_field(obj, 4, (char *)"pit.chan[2].gate", &pit.chan[2].gate);
    state_field(obj, 4, (char *)"pit.chan[0].rw_mode", &pit.chan[0].rw_mode);
    state_field(obj, 4, (char *)"pit.chan[1].rw_mode", &pit.chan[1].rw_mode);
    state_field(obj, 4, (char *)"pit.chan[2].rw_mode", &pit.chan[2].rw_mode);
    state_field(obj, 4, (char *)"pit.chan[0].rmode", &pit.chan[0].rmode);
    state_field(obj, 4, (char *)"pit.chan[1].rmode", &pit.chan[1].rmode);
    state_field(obj, 4, (char *)"pit.chan[2].rmode", &pit.chan[2].rmode);
    state_field(obj, 4, (char *)"pit.chan[0].wmode", &pit.chan[0].wmode);
    state_field(obj, 4, (char *)"pit.chan[1].wmode", &pit.chan[1].wmode);
    state_field(obj, 4, (char *)"pit.chan[2].wmode", &pit.chan[2].wmode);
    state_field(obj, 1, (char *)"pit.chan[0].status_latch", &pit.chan[0].status_latch);
    state_field(obj, 1, (char *)"pit.chan[1].status_latch", &pit.chan[1].status_latch);
    state_field(obj, 1, (char *)"pit.chan[2].status_latch", &pit.chan[2].status_latch);
    state_field(obj, 1, (char *)"pit.chan[0].whats_latched", &pit.chan[0].whats_latched);
    state_field(obj, 1, (char *)"pit.chan[1].whats_latched", &pit.chan[1].whats_latched);
    state_field(obj, 1, (char *)"pit.chan[2].whats_latched", &pit.chan[2].whats_latched);
    state_field(obj, 2, (char *)"pit.chan[0].counter_latch", &pit.chan[0].counter_latch);
    state_field(obj, 2, (char *)"pit.chan[1].counter_latch", &pit.chan[1].counter_latch);
    state_field(obj, 2, (char *)"pit.chan[2].counter_latch", &pit.chan[2].counter_latch);
    state_field(obj, 8, (char *)"pit.chan[0].last_load_time", &pit.chan[0].last_load_time);
    state_field(obj, 8, (char *)"pit.chan[1].last_load_time", &pit.chan[1].last_load_time);
    state_field(obj, 8, (char *)"pit.chan[2].last_load_time", &pit.chan[2].last_load_time);
    state_field(obj, 8, (char *)"pit.chan[0].last_irq_time", &pit.chan[0].last_irq_time);
    state_field(obj, 8, (char *)"pit.chan[1].last_irq_time", &pit.chan[1].last_irq_time);
    state_field(obj, 8, (char *)"pit.chan[2].last_irq_time", &pit.chan[2].last_irq_time);
    state_field(obj, 4, (char *)"pit.chan[0].period", &pit.chan[0].period);
    state_field(obj, 4, (char *)"pit.chan[1].period", &pit.chan[1].period);
    state_field(obj, 4, (char *)"pit.chan[2].period", &pit.chan[2].period);
    state_field(obj, 4, (char *)"pit.chan[0].pit_last_count", &pit.chan[0].pit_last_count);
    state_field(obj, 4, (char *)"pit.chan[1].pit_last_count", &pit.chan[1].pit_last_count);
    state_field(obj, 4, (char *)"pit.chan[2].pit_last_count", &pit.chan[2].pit_last_count);
    state_field(obj, 4, (char *)"pit.chan[0].timer_flipflop", &pit.chan[0].timer_flipflop);
    state_field(obj, 4, (char *)"pit.chan[1].timer_flipflop", &pit.chan[1].timer_flipflop);
    state_field(obj, 4, (char *)"pit.chan[2].timer_flipflop", &pit.chan[2].timer_flipflop);
    state_field(obj, 4, (char *)"pit.chan[0].timer_running", &pit.chan[0].timer_running);
    state_field(obj, 4, (char *)"pit.chan[1].timer_running", &pit.chan[1].timer_running);
    state_field(obj, 4, (char *)"pit.chan[2].timer_running", &pit.chan[2].timer_running);
    FIELD(pit.speaker);
    FIELD(pit.last);
}
static inline itick_t pit_counter_to_itick(uint32_t c)
{
    double time_scale = (double)ticks_per_second / (double)PIT_CLOCK_SPEED;
    return (itick_t)((double)c * time_scale);
}
static inline itick_t pit_itick_to_counter(itick_t i)
{
    double time_scale = (double)PIT_CLOCK_SPEED / (double)ticks_per_second;
    return (itick_t)((double)i * time_scale);
}
static int pit_get_out(struct pit_channel *pit)
{
    uint32_t elapsed = pit_itick_to_counter(get_now() - pit->last_load_time);
    if (pit->count == 0)
        return 0;
    uint32_t current_counter = elapsed % pit->count;
    switch (pit->mode) {
        case 0:
        case 1:
            return (pit->count >= current_counter) ^ pit->mode;
        case 2:
            return current_counter != 1;
        case 3:
            if (pit->count & 1)
                return current_counter >= ((pit->count + 1) >> 1);
            else
                return current_counter < ((pit->count - 1) >> 1);
        case 4:
        case 5:
            return current_counter != 0;
    }
    abort();
}
static int pit_get_count(struct pit_channel *pit)
{
    itick_t  elapsed       = get_now() - pit->last_load_time;
    uint32_t diff_in_ticks = (uint32_t)((double)elapsed * (double)PIT_CLOCK_SPEED / (double)ticks_per_second);
    uint32_t current       = pit->count - diff_in_ticks;
    if (pit->count == 0)
        return 0;
    current = (current % pit->count);
    return current;
}
static void pit_set_count(struct pit_channel *self, int v)
{
    self->last_irq_time = self->last_load_time = get_now();
    self->count                                = (!v) << 16 | v;
    self->period                               = pit_counter_to_itick(self->count);
    self->timer_running                        = 1;
    self->pit_last_count                       = pit_get_count(self);
}
static void pit_channel_latch_counter(struct pit_channel *self)
{
    if (!(self->whats_latched & COUNTER_LATCHED)) {
        uint16_t ct         = pit_get_count(self);
        int      mode       = self->rw_mode;
        self->whats_latched = (mode << 2) | COUNTER_LATCHED;
        switch (mode) {
            case 1:
            case 2:
                self->counter_latch = ct >> ((mode - 1) << 3) & 0xFF;
                break;
            case 3:
                self->counter_latch = ct;
                break;
        }
    }
}
static void pit_writeb(uint32_t port, uint32_t value)
{
    int channel = port & 3;
    switch (channel) {
        case 3: {
            channel        = value >> 6;
            uint8_t opmode = value >> 1 & 7, bcd = value & 1, access = value >> 4 & 3;
            switch (channel) {
                case 3:
                    for (int i = 0; i < 3; i++) {
                        if ((opmode >> i) & 1) {
                            struct pit_channel *chan = &pit.chan[i];
                            if (!(access & 2))
                                pit_channel_latch_counter(chan);
                            if (!(access & 1)) {
                                if (!(chan->whats_latched & STATUS_LATCHED)) {
                                    chan->status_latch =
                                        (pit_get_out(chan) << 7) | (chan->rw_mode << 4) | (chan->mode << 1) | chan->bcd;
                                    chan->whats_latched |= STATUS_LATCHED;
                                }
                            }
                        }
                    }
                    break;
                case 0 ... 2: {
                    struct pit_channel *chan = &pit.chan[channel];
                    if (!access) {
                        pit_channel_latch_counter(chan);
                    } else {
                        chan->rw_mode = access;
                        chan->wmode = chan->rmode = access - 1;
                        chan->mode                = opmode;
                        switch (chan->mode) {
                            case 2:
                                if (channel == 0)
                                    pic_raise_irq(0);
                                break;
                        }
                        chan->bcd = bcd;
                        if (bcd) {
                            PIT_LOG("BCD mode not supported\n");
                        }
                    }
                    break;
                }
            }
            break;
        }
        case 0 ... 2: {
            struct pit_channel *chan = &pit.chan[channel];
            switch (chan->wmode) {
                case 0:
                    pit_set_count(chan, value);
                    break;
                case 1:
                    pit_set_count(chan, value << 8);
                    break;
                case 2:
                    chan->interim_count = value;
                    chan->wmode ^= 1;
                    break;
                case 3:
                    pit_set_count(chan, value << 8 | chan->interim_count);
                    chan->wmode ^= 1;
                    break;
            }
            break;
        }
    }
}
static uint32_t pit_readb(uint32_t a)
{
    struct pit_channel *chan = &pit.chan[a & 3];
    uint8_t             retv = -1;
    if (chan->whats_latched & STATUS_LATCHED) {
        chan->whats_latched &= ~STATUS_LATCHED;
        retv = chan->status_latch;
    } else if (chan->whats_latched & COUNTER_LATCHED) {
        int whats_latched_temp = chan->whats_latched >> 2;
        switch (whats_latched_temp) {
            case 1:
            case 2:
                whats_latched_temp = 0;
                retv               = chan->counter_latch;
                break;
            case 3:
                whats_latched_temp = (2 << 2) | COUNTER_LATCHED;
                retv               = chan->counter_latch;
                chan->counter_latch >>= 8;
                break;
        }
        chan->whats_latched = whats_latched_temp;
    } else {
        uint32_t count = pit_get_count(chan);
        switch (chan->rmode) {
            case 0:
                retv = count;
                break;
            case 1:
                retv = count >> 8;
                break;
            case 2:
            case 3:
                retv = count >> ((chan->rmode & 1) << 3);
                chan->rmode ^= 1;
                break;
        }
    }
    return retv;
}
static void pit_channel_reset(struct pit_channel *self)
{
    self->count    = 0;
    self->flipflop = self->mode = self->bcd = self->gate = 0;
    self->last_load_time                                 = -1;
}
static void pit_reset(void)
{
    for (int i = 0; i < 3; i++) {
        pit_channel_reset(pit.chan + i);
        pit.chan[i].gate = i != 2;
    }
    pit.speaker = 0;
}
static void timer_cb(void)
{
    pic_lower_irq(0);
    pic_raise_irq(0);
}
int pit_next(itick_t now)
{
    UNUSED(now);
    uint32_t count = pit_get_count(&pit.chan[0]), raise_irq = 0;
    if (count > pit.chan[0].pit_last_count) {
        raise_irq = 1;
    }
    if (pit.chan[0].timer_running) {
        int refill_count = pit.chan[0].count;
        if (raise_irq) {
            timer_cb();
            if (pit.chan[0].mode != 2 && pit.chan[0].mode != 3) {
                pit.chan[0].timer_running = 0;
                return -1;
            }
        }
        pit.chan[0].pit_last_count = count;
        return pit_counter_to_itick(refill_count - count);
    }
    return -1;
}
static uint32_t pit_speaker_readb(uint32_t port)
{
    UNUSED(port);
    pit.chan[2].timer_flipflop ^= 1;
    return pit.chan[2].timer_flipflop << 4 | (pit_get_out(&pit.chan[2]) << 5);
}
static void pit_speaker_writeb(uint32_t port, uint32_t data)
{
    UNUSED(port | data);
    PIT_LOG("%sabled the pc speaker\n", data & 1 ? "En" : "Dis");
}
void pit_init(void)
{
    io_register_reset(pit_reset);
    io_register_read(0x40, 3, pit_readb, NULL, NULL);
    io_register_write(0x40, 4, pit_writeb, NULL, NULL);
    io_register_read(0x61, 1, pit_speaker_readb, NULL, NULL);
    io_register_write(0x61, 1, pit_speaker_writeb, NULL, NULL);
    state_register(pit_state);
}