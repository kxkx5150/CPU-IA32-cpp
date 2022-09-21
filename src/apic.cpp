#include "cpu/cpu.h"
#include "devices.h"
#include "io.h"
#include "pc.h"

#define APIC_LOG(x, ...)             LOG("APIC", x, ##__VA_ARGS__)
#define APIC_FATAL(x, ...)           FATAL("APIC", x, ##__VA_ARGS__)
#define APIC_ERROR_SEND_CHECKSUM     1
#define APIC_ERROR_RECV_CHECKSUM     2
#define APIC_SEND_ACCEPT_ERROR       4
#define APIC_RECV_ACCEPT_ERROR       8
#define APIC_REDIRECT_IPI            16
#define APIC_SEND_INVALID_VECTOR     32
#define APIC_RECV_INVALID_VECTOR     64
#define APIC_ILLEGAL_REGISTER_ACCESS 128
#define LVT_DISABLED                 (1 << 16)
#define EDGE_TRIGGERED               0
#define LEVEL_TRIGGERED              1

extern CPU cpu;


enum
{
    LVT_INDEX_CMCI = 0,
    LVT_INDEX_TIMER,
    LVT_INDEX_THERMAL,
    LVT_INDEX_PERFORMANCE_COUNTER,
    LVT_LINT0,
    LVT_LINT1,
    LVT_ERROR,
    LVT_END
};
enum
{
    LVT_DELIVERY_FIXED           = 0,
    LVT_DELIVERY_SMI             = 2,
    LVT_DELIVERY_LOWEST_PRIORITY = 3,
    LVT_DELIVERY_NMI             = 4,
    LVT_DELIVERY_INIT            = 5,
    LVT_DELIVERY_EXT_INT         = 7
};
static struct apic_info
{
    uint32_t base;
    uint32_t spurious_interrupt_vector;
    uint32_t lvt[7];
    uint32_t isr[8];
    uint32_t tmr[8];
    uint32_t irr[8];
    uint32_t icr[2];
    uint32_t id;

    uint32_t error, cached_error;
    uint32_t timer_divide, timer_initial_count;
    itick_t  timer_reload_time, timer_next;
    uint32_t destination_format, logical_destination;
    int      dest_format_physical;
    int      intr_line_state;
    uint32_t task_priority, processor_priority;
    int      enabled;
    uint32_t temp_data;
} apic;


static void apic_state(void)
{
    struct bjson_object *obj = state_obj((char *)"apic", 22 + 0);

    state_field(obj, 4, (char *)"apic.base", &apic.base);
    state_field(obj, 4, (char *)"apic.spurious_interrupt_vector", &apic.spurious_interrupt_vector);
    state_field(obj, 28, (char *)"apic.lvt", &apic.lvt);
    state_field(obj, 32, (char *)"apic.isr", &apic.isr);
    state_field(obj, 32, (char *)"apic.tmr", &apic.tmr);
    state_field(obj, 32, (char *)"apic.irr", &apic.irr);
    state_field(obj, 8, (char *)"apic.icr", &apic.icr);
    state_field(obj, 4, (char *)"apic.id", &apic.id);
    state_field(obj, 4, (char *)"apic.error", &apic.error);
    state_field(obj, 4, (char *)"apic.cached_error", &apic.cached_error);
    state_field(obj, 4, (char *)"apic.timer_divide", &apic.timer_divide);
    state_field(obj, 4, (char *)"apic.timer_initial_count", &apic.timer_initial_count);
    state_field(obj, 8, (char *)"apic.timer_reload_time", &apic.timer_reload_time);
    state_field(obj, 8, (char *)"apic.timer_next", &apic.timer_next);
    state_field(obj, 4, (char *)"apic.destination_format", &apic.destination_format);
    state_field(obj, 4, (char *)"apic.logical_destination", &apic.logical_destination);
    state_field(obj, 4, (char *)"apic.dest_format_physical", &apic.dest_format_physical);
    state_field(obj, 4, (char *)"apic.intr_line_state", &apic.intr_line_state);
    state_field(obj, 4, (char *)"apic.task_priority", &apic.task_priority);
    state_field(obj, 4, (char *)"apic.processor_priority", &apic.processor_priority);
    state_field(obj, 4, (char *)"apic.enabled", &apic.enabled);
    state_field(obj, 4, (char *)"apic.temp_data", &apic.temp_data);
}
static inline void set_bit(uint32_t *ptr, int bitpos, int bit)
{
    int ptr_loc = bitpos >> 5 & 7, bit_loc = bitpos & 0x1F;
    if (!bit)
        ptr[ptr_loc] &= ~(1 << bit_loc);
    else
        ptr[ptr_loc] |= 1 << bit_loc;
}
static inline int get_bit(uint32_t *ptr, int bit)
{
    return (ptr[bit >> 5 & 7] & 1 << (bit & 0x1F)) != 0;
}
static inline int highest_set_bit(uint32_t *ptr)
{
    int x = 256 - 32;
    for (int i = 7; i >= 0; i--) {
        if (!ptr[i])
            x -= 32;
        else {
            return (31 - __builtin_clz(ptr[i])) + x;
        }
    }
    return -1;
}
static inline int vector_invalid(int vector)
{
    return !(vector & 0xF0) || vector >= 0xFF;
}
static void apic_error(void)
{
}
static void apic_send_highest_priority_interrupt(void)
{
    if (apic.intr_line_state == 1)
        return;

    int highest_interrupt_requested  = highest_set_bit(apic.irr),
        highest_interrupt_in_service = highest_set_bit(apic.isr);

    if (highest_interrupt_requested == -1)
        return;

    if (highest_interrupt_in_service < highest_interrupt_requested) {
        if ((highest_interrupt_requested & 0xF0) > (apic.task_priority & 0xF0)) {
            apic.processor_priority = highest_interrupt_requested & 0xF0;
            apic.intr_line_state    = 1;
            cpu.cpu_raise_intr_line();
            cpu.cpu_request_fast_return(EXIT_STATUS_IRQ);
        } else
            return;
    } else
        return;
}
int apic_get_interrupt(void)
{
    int highest_irr = highest_set_bit(apic.irr);
    if (highest_irr == -1) {
        APIC_FATAL("TODO: spurious interrupts\n");
    }

    set_bit(apic.irr, highest_irr, 0);
    set_bit(apic.isr, highest_irr, 1);

    apic.intr_line_state = 0;
    cpu.cpu_lower_intr_line();
    return highest_irr;
}
int apic_has_interrupt(void)
{
    return apic.intr_line_state;
}
void apic_receive_bus_message(int vector, int type, int trigger_mode)
{
    switch (type) {
        case LVT_DELIVERY_INIT:
            APIC_FATAL("TODO: INIT delivery\n");
            break;
        case LVT_DELIVERY_NMI:
            APIC_FATAL("TODO: NMI delivery\n");
            break;
        case LVT_DELIVERY_SMI:
            APIC_FATAL("TODO: SMI delivery\n");
            break;
        case LVT_DELIVERY_EXT_INT:
            set_bit(apic.irr, vector, 1);
            apic_send_highest_priority_interrupt();
            break;
        case LVT_DELIVERY_FIXED:
        case LVT_DELIVERY_LOWEST_PRIORITY:
            if (vector_invalid(vector)) {
                apic.error |= APIC_RECV_INVALID_VECTOR;
                apic_error();
            }
            if (get_bit(apic.irr, vector))
                return;
            set_bit(apic.irr, vector, 1);
            set_bit(apic.tmr, vector, trigger_mode);
            apic_send_highest_priority_interrupt();
            break;
    }
}
static void apic_send_ipi2(uint32_t vector, int mode, int trigger, uint32_t destination)
{
    if (vector_invalid(vector)) {
        apic.error |= APIC_SEND_INVALID_VECTOR;
        apic_error();
    }
    if (destination == apic.id) {
        apic_receive_bus_message(vector, mode, trigger);
    }
}
static void apic_send_ipi(uint32_t vector, int mode, int trigger)
{
    if (vector_invalid(vector)) {
        apic.error |= APIC_SEND_INVALID_VECTOR;
        apic_error();
    }
    apic_receive_bus_message(vector, mode, trigger);
}
static uint32_t *get_lvt_ptr(int idx)
{
    switch (idx) {
        case 0x2F:
            return &apic.lvt[LVT_INDEX_CMCI];
        case 0x32:
            return &apic.lvt[LVT_INDEX_TIMER];
        case 0x33:
            return &apic.lvt[LVT_INDEX_THERMAL];
        case 0x34:
            return &apic.lvt[LVT_INDEX_PERFORMANCE_COUNTER];
        case 0x35:
            return &apic.lvt[LVT_LINT0];
        case 0x36:
            return &apic.lvt[LVT_LINT1];
        case 0x37:
            return &apic.lvt[LVT_ERROR];
    }
    return NULL;
}
static int apic_get_clock_divide(void)
{
    return ((((apic.timer_divide >> 1 & 4) | (apic.timer_divide & 3)) + 1) & 7);
}
static uint32_t apic_get_count(void)
{
    return apic.timer_initial_count -
           ((uint32_t)(cpu.cpu_get_cycles() - apic.timer_reload_time) >> apic_get_clock_divide()) %
               apic.timer_initial_count;
}
static itick_t apic_get_period(void)
{
    return (itick_t)apic.timer_initial_count << apic_get_clock_divide();
}
static uint32_t apic_read(uint32_t addr)
{
    addr -= apic.base;
    addr >>= 4;
    switch (addr) {
        case 0x02:
            return apic.id;
        case 0x03:
            return 0x14 | (5 << 16) | (0 << 24);
        case 0x08:
            return apic.task_priority;
        case 0x0B:
            return 0;
        case 0x0D:
            return apic.logical_destination;
        case 0x0E:
            return apic.destination_format;
        case 0x0F:
            return apic.spurious_interrupt_vector;
        case 0x10 ... 0x17:
            return apic.isr[addr & 7];
        case 0x18 ... 0x1F:
            return apic.tmr[addr & 7];
        case 0x20 ... 0x27:
            return apic.irr[addr & 7];
        case 0x28: {
            return apic.cached_error;
        }
        case 0x2F:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
            return *get_lvt_ptr(addr);
        case 0x30 ... 0x31:
            return apic.icr[addr & 1];
        case 0x38:
            return apic.timer_initial_count;
        case 0x39:
            return apic_get_count();
        case 0x3E:
            return apic.timer_divide;
        default:
            APIC_FATAL("TODO: APIC read %08x\n", addr);
    }
}
static void apic_write(uint32_t addr, uint32_t data)
{
    addr -= apic.base;
    addr >>= 4;
    switch (addr) {
        case 0x03:
            apic.error |= APIC_ILLEGAL_REGISTER_ACCESS;
            break;
        case 2:
            apic.id = data;
            break;
        case 0x08: {
            apic.task_priority = data & 0xFF;
            int highest_isr    = highest_set_bit(apic.isr);
            if (highest_isr == -1)
                apic.processor_priority = apic.task_priority;
            else {
                int ndiff = (apic.task_priority & 0xF0) - (highest_isr & 0xF0);
                if (ndiff > 0)
                    apic.processor_priority = apic.task_priority;
                else
                    apic.processor_priority = highest_isr & 0xF0;
            }
            apic_send_highest_priority_interrupt();
            break;
        }
        case 0x0B: {
            int current_isr = highest_set_bit(apic.isr);
            if (current_isr != -1) {
                set_bit(apic.isr, current_isr, 0);
                if (get_bit(apic.tmr, current_isr)) {
                    ioapic_remote_eoi(current_isr);
                }
                apic_send_highest_priority_interrupt();
            }
            break;
        }
        case 0x0D:
            apic.logical_destination = data & 0xFF000000;
            break;
        case 0x0E:
            apic.destination_format &= ~0xF0000000;
            apic.destination_format |= data & 0xF0000000;
            apic.dest_format_physical = apic.destination_format == 0xFFFFFFFF;
            break;
        case 0x0F:
            apic.spurious_interrupt_vector = data;
            if (data & 0x100) {
                for (int i = 0; i < 7; i++)
                    apic.lvt[i] |= LVT_DISABLED;
            }
            break;
        case 0x10 ... 0x17:
            apic.isr[addr & 7] = data;
            break;
        case 0x18 ... 0x1F:
            apic.tmr[addr & 7] = data;
            break;
        case 0x20 ... 0x27:
            apic.irr[addr & 7] = data;
            break;
        case 0x28:
            apic.cached_error = apic.error;
            apic.error        = 0;
            break;
        case 0x2F:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
            *get_lvt_ptr(addr) = data;
            break;
        case 0x30: {
            apic.icr[0] = data;
            int vector = data & 0xFF, delivery_mode = data >> 8 & 7, level = data >> 14 & 1, trigger = data >> 15 & 1,
                destination_shorthand = data >> 18 & 3, apic_destination = apic.icr[1] >> (56 - 32);
            if (delivery_mode == 5 && level == 0 && trigger == 1) {
                return;
            }
            switch (destination_shorthand) {
                case 0:
                    apic_send_ipi2(vector, delivery_mode, trigger, apic_destination);
                    break;
                case 1:
                    apic_send_ipi(vector, LVT_DELIVERY_FIXED, trigger);
                    break;
                case 2:
                    apic_send_ipi(vector, delivery_mode, trigger);
                    break;
                case 3:
                    break;
            }
            break;
        }
        case 0x31:
            apic.icr[1] = data;
            break;
        case 0x38:
            apic.timer_initial_count = data;
            apic.timer_reload_time   = get_now();
            apic.timer_next          = apic.timer_reload_time + apic_get_period();
            cpu.cpu_cancel_execution_cycle(EXIT_STATUS_NORMAL);
            break;
        case 0x39:
            break;
        case 0x3E:
            apic.timer_divide = data;
            cpu.cpu_cancel_execution_cycle(EXIT_STATUS_NORMAL);
            break;
        default:
            APIC_FATAL("TODO: APIC write %08x data=%08x\n", addr, data);
    }
}
static uint32_t apic_readb(uint32_t addr)
{
    return apic_read(addr & ~3) >> ((addr & 3) * 8) & 0xFF;
}
static void apic_writeb(uint32_t addr, uint32_t data)
{
    int offset = addr & 3, byte_offset = offset << 3;
    apic.temp_data &= ~(0xFF << byte_offset);
    apic.temp_data |= data << byte_offset;
    if (offset == 3) {
        apic_write(addr & ~3, apic.temp_data);
    }
}
static void apic_reset(void)
{
    apic.spurious_interrupt_vector = 0xFF;
    apic.base                      = 0xFEE00000;
    apic.id                        = 0;
    apic.error                     = 0;
    apic.destination_format        = -1;
    apic.dest_format_physical      = 1;
    for (int i = 0; i < LVT_END; i++)
        apic.lvt[i] = LVT_DISABLED;
    io_register_mmio_read(apic.base, 4096, apic_readb, NULL, apic_read);
    io_register_mmio_write(apic.base, 4096, apic_writeb, NULL, apic_write);
}
int apic_next(itick_t now)
{
    if (!apic.enabled)
        return -1;
    if (apic.timer_initial_count == 0)
        return -1;
    int apic_timer_enabled = 1;
    int info               = apic.lvt[LVT_INDEX_TIMER] >> 16;
    if (apic.timer_next <= now) {
        if (!(info & 1)) {
            apic_receive_bus_message(apic.lvt[LVT_INDEX_TIMER] & 0xFF, LVT_DELIVERY_FIXED, 0);
        } else
            apic_timer_enabled = 0;
        switch (info >> 1 & 3) {
            case 2:
                APIC_FATAL("TODO: TSC Deadline\n");
                break;
            case 1:
                apic.timer_next += apic_get_period();
                break;
            case 0:
                apic.timer_next = -1;
                return -1;
            case 3:
                return -1;
        }
        if (apic_timer_enabled)
            return -1;
    }
    itick_t next = apic.timer_next - now;
    if (next > 0xFFFFFFFF)
        return -1;
    return (uint32_t)next;
}
void apic_init(struct pc_settings *pc)
{
    apic.enabled = pc->apic_enabled;
    if (!apic.enabled)
        return;
    io_register_reset(apic_reset);
    state_register(apic_state);
}
int apic_is_enabled(void)
{
    return apic.enabled;
}