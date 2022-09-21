
#include "devices.h"
#include "state.h"
#include <time.h>

#define FREQUENCY          32768
#define CMOS_LOG(x, ...)   LOG("CMOS", x, ##__VA_ARGS__)
#define CMOS_FATAL(x, ...) FATAL("CMOS", x, ##__VA_ARGS__)
#define ALARM_SEC          1
#define ALARM_MIN          3
#define ALARM_HOUR         5

#define is24hour() (cmos.ram[0x0B] & 2)
#define PERIODIC   0x40
#define ALARM      0x20
#define UPDATE     0x10


struct cmos
{
    uint8_t  ram[128];
    uint8_t  addr, nmi;
    time_t   now;
    int      periodic_ticks, periodic_ticks_max;
    uint32_t period;
    itick_t  last_called, uip_period, last_second_update;
};
static struct cmos cmos;


static void cmos_state(void)
{
    struct bjson_object *obj = state_obj((char *)"cmos", 10);
    state_field(obj, 128, (char *)"cmos.ram", &cmos.ram);
    state_field(obj, 1, (char *)"cmos.addr", &cmos.addr);
    state_field(obj, 1, (char *)"cmos.nmi", &cmos.nmi);
    state_field(obj, sizeof(time_t), (char *)"cmos.now", &cmos.now);
    state_field(obj, 4, (char *)"cmos.periodic_ticks", &cmos.periodic_ticks);
    state_field(obj, 4, (char *)"cmos.periodic_ticks_max", &cmos.periodic_ticks_max);
    state_field(obj, 4, (char *)"cmos.period", &cmos.period);
    state_field(obj, 8, (char *)"cmos.last_called", &cmos.last_called);
    state_field(obj, 8, (char *)"cmos.uip_period", &cmos.uip_period);
    state_field(obj, 8, (char *)"cmos.last_second_update", &cmos.last_second_update);
}
static void cmos_lower_irq(void)
{
    pic_lower_irq(8);
}
static uint8_t bcd_read(uint8_t val)
{
    if (cmos.ram[0x0B] & 4)
        return val;
    else
        return ((val / 10) << 4) | (val % 10);
}
static uint8_t cmos_ram_read(uint8_t addr)
{
    struct tm *now;
    itick_t    now_ticks, next_second;
    switch (addr) {
        case 0:
            now = localtime(&cmos.now);
            return bcd_read(now->tm_sec);
        case 2:
            now = localtime(&cmos.now);
            return bcd_read(now->tm_min);
        case 4:
            now = localtime(&cmos.now);
            if (is24hour())
                return bcd_read(now->tm_hour);
            else
                return bcd_read(now->tm_hour % 12) | (now->tm_hour > 12) << 7;
        case 6:
            now = localtime(&cmos.now);
            return bcd_read(now->tm_wday + 1);
        case 7:
            now = localtime(&cmos.now);
            return bcd_read(now->tm_mday);
        case 8:
            now = localtime(&cmos.now);
            return bcd_read(now->tm_mon + 1);
        case 9:
            now = localtime(&cmos.now);
            return bcd_read(now->tm_year % 100);
        case 1:
        case 3:
        case 5:
            return cmos.ram[cmos.addr];
        case 0x0A:
            now_ticks   = get_now();
            next_second = cmos.last_second_update + ticks_per_second;
            if (now_ticks >= (next_second - cmos.uip_period) && now_ticks < next_second) {
                return cmos.ram[0x0A] | 0x80;
            }
        case 0x0B:
            return cmos.ram[cmos.addr];
        case 0x0C: {
            cmos_lower_irq();
            int res        = cmos.ram[0x0C];
            cmos.ram[0x0C] = 0;
            return res;
        }
        case 0x0D:
            return 0x80;
    }
    CMOS_FATAL("should not be here\n");
}
static uint32_t cmos_readb(uint32_t port)
{
    switch (port & 1) {
        case 0:
            return 0xFF;
        case 1:
            if (cmos.addr <= 0x0D)
                return cmos_ram_read(cmos.addr);
            else
                return cmos.ram[cmos.addr];
    }
    CMOS_FATAL("should not be here\n");
}
static inline void cmos_update_timer(void)
{
    if ((cmos.ram[0x0A] >> 4 & 7) != 2)
        CMOS_LOG("22-step divider set to strange value: %d\n", cmos.ram[0x0A] >> 4 & 7);
    int period = cmos.ram[0x0A] & 0x0F;
    if (!period)
        return;
    if (period < 3)
        period += 7;
    int freq = FREQUENCY >> (period - 1);
    if (cmos.ram[0x0B] & 0x40) {
        cmos.period             = ticks_per_second / freq;
        cmos.periodic_ticks     = 0;
        cmos.periodic_ticks_max = freq;
    } else {
        cmos.period = ticks_per_second;
    }
    cmos.last_called = get_now();
}
static inline int bcd(int data)
{
    if (cmos.ram[0x0B] & 4)
        return data;
    return ((data & 0xf0) >> 1) + ((data & 0xf0) >> 3) + (data & 0x0f);
}
static inline void cmos_ram_write(uint8_t data)
{
    struct tm *now = localtime(&cmos.now);
    switch (cmos.addr) {
        case 1:
        case 3:
        case 5:
            cmos.ram[cmos.addr] = data;
            break;
        case 0:
            now->tm_sec = bcd(data);
            break;
        case 2:
            now->tm_min = bcd(data);
            break;
        case 4:
            now->tm_hour = bcd(data & 0x7F);
            if (!is24hour())
                if (data & 0x80)
                    now->tm_hour += 12;
            break;
        case 6:
            now->tm_wday = bcd(data);
            break;
        case 7:
            now->tm_mday = bcd(data);
            break;
        case 8:
            now->tm_mon = bcd(data);
            break;
        case 9:
            now->tm_year = bcd(data) + (bcd(cmos.ram[0x32]) - 19) * 100;
            if (now->tm_year < 70)
                now->tm_year = 70;
            break;
        case 0x0A:
            cmos.ram[0x0A] = (data & 0x7F) | (cmos.ram[0x0A] & 0x80);
            cmos_update_timer();
            break;
        case 0x0B:
            cmos.ram[0x0B] = data;
            cmos_update_timer();
            break;
        case 0x0C ... 0x0D:
            break;
        default:
            CMOS_FATAL("?? writeb ??");
    }
    cmos.now = mktime(now);
}
static void cmos_writeb(uint32_t port, uint32_t data)
{
    switch (port & 1) {
        case 0:
            cmos.nmi  = data >> 7;
            cmos.addr = data & 0x7F;
            break;
        case 1:
            if (cmos.addr <= 0x0D)
                cmos_ram_write(data);
            else
                cmos.ram[cmos.addr] = data;
    }
}
static void cmos_raise_irq(int why)
{
    cmos.ram[0x0C] = 0x80 | why;
    pic_raise_irq(8);
}
int cmos_clock(itick_t now)
{
    itick_t next = cmos.last_called + cmos.period;
    if (now >= next) {
        int why = 0;
        if (cmos.ram[0x0B] & 0x40) {
            why |= PERIODIC;
            cmos.periodic_ticks++;
            if (cmos.periodic_ticks != cmos.periodic_ticks_max)
                goto done;
            cmos.periodic_ticks = 0;
        }
        cmos.now++;
        if (cmos.ram[0x0B] & 0x20) {
            int ok = 1;
            ok &= cmos_ram_read(ALARM_SEC) == cmos_ram_read(0);
            ok &= cmos_ram_read(ALARM_MIN) == cmos_ram_read(2);
            ok &= cmos_ram_read(ALARM_HOUR) == cmos_ram_read(4);
            if (ok)
                why |= ALARM;
        }
        if (cmos.ram[0x0B] & 0x10) {
            why |= UPDATE;
        }
        cmos.last_second_update = now;
    done:
        cmos.last_called = get_now();
        if (why) {
            cmos_raise_irq(why);
            return 1;
        }
    }
    return 0;
}
int cmos_next(itick_t now)
{
    cmos_clock(now);
    return cmos.last_called + cmos.period - now;
}
void cmos_set(uint8_t where, uint8_t data)
{
    cmos.ram[where] = data;
}
uint8_t cmos_get(uint8_t where)
{
    return cmos.ram[where];
}
static void cmos_reset(void)
{
    cmos.ram[0x0A] = 0x26;
    cmos.ram[0x0B] = 0x02;
    cmos.ram[0x0C] = 0x00;
    cmos.ram[0x0D] = 0x80;
}
void cmos_init(uint64_t now)
{
    io_register_read(0x70, 2, cmos_readb, NULL, NULL);
    io_register_write(0x70, 2, cmos_writeb, NULL, NULL);
    state_register(cmos_state);
    io_register_reset(cmos_reset);
    if (now == 0)
        now = time(NULL);
    cmos.now                = now;
    cmos.last_second_update = get_now();
    cmos.uip_period         = 244;
    cmos.last_called        = get_now();
    cmos.period             = ticks_per_second;
}