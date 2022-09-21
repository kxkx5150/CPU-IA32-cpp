#include "cpu/cpu.h"
#include "devices.h"
#include "state.h"


extern CPU cpu;


#define PIC_LOG(x, ...) LOG("PIC", x, ##__VA_ARGS__)
#define PIC_FATAL(x, ...)                                                                                              \
    do {                                                                                                               \
        ABORT();                                                                                                       \
    } while (0)


struct pic_controller
{
    uint8_t vector_offset;
    uint8_t imr, irr, isr;
    uint8_t pin_state;
    uint8_t icw[5], icw_index;
    uint8_t ocw[4];
    uint8_t read_isr;
    uint8_t autoeoi, rotate_on_autoeoi;
    uint8_t priority_base;
    uint8_t in_initialization;
    uint8_t highest_priority_irq_to_send;
    uint8_t raised_intr_line;
    uint8_t elcr;
};
struct
{
    int                   irq_bus_value;
    struct pic_controller ctrl[2];
} pic;


static inline uint8_t rol(uint8_t value, uint8_t priority_base)
{
    uint8_t count = priority_base ^ 7;
    return (value << count) | (value >> (8 - count));
}
static inline int is_master(struct pic_controller *self)
{
    return self == &pic.ctrl[0];
}
static void pic_state(void)
{
    struct bjson_object *obj = state_obj((char *)"pic", (16 + 1) * 2);
    state_field(obj, 1, (char *)"pic.ctrl[0].vector_offset", &pic.ctrl[0].vector_offset);
    state_field(obj, 1, (char *)"pic.ctrl[1].vector_offset", &pic.ctrl[1].vector_offset);
    state_field(obj, 1, (char *)"pic.ctrl[0].imr", &pic.ctrl[0].imr);
    state_field(obj, 1, (char *)"pic.ctrl[1].imr", &pic.ctrl[1].imr);
    state_field(obj, 1, (char *)"pic.ctrl[0].irr", &pic.ctrl[0].irr);
    state_field(obj, 1, (char *)"pic.ctrl[1].irr", &pic.ctrl[1].irr);
    state_field(obj, 1, (char *)"pic.ctrl[0].isr", &pic.ctrl[0].isr);
    state_field(obj, 1, (char *)"pic.ctrl[1].isr", &pic.ctrl[1].isr);
    state_field(obj, 1, (char *)"pic.ctrl[0].pin_state", &pic.ctrl[0].pin_state);
    state_field(obj, 1, (char *)"pic.ctrl[1].pin_state", &pic.ctrl[1].pin_state);
    state_field(obj, 5, (char *)"pic.ctrl[0].icw", &pic.ctrl[0].icw);
    state_field(obj, 5, (char *)"pic.ctrl[1].icw", &pic.ctrl[1].icw);
    state_field(obj, 1, (char *)"pic.ctrl[0].icw_index", &pic.ctrl[0].icw_index);
    state_field(obj, 1, (char *)"pic.ctrl[1].icw_index", &pic.ctrl[1].icw_index);
    state_field(obj, 4, (char *)"pic.ctrl[0].ocw", &pic.ctrl[0].ocw);
    state_field(obj, 4, (char *)"pic.ctrl[1].ocw", &pic.ctrl[1].ocw);
    state_field(obj, 1, (char *)"pic.ctrl[0].read_isr", &pic.ctrl[0].read_isr);
    state_field(obj, 1, (char *)"pic.ctrl[1].read_isr", &pic.ctrl[1].read_isr);
    state_field(obj, 1, (char *)"pic.ctrl[0].autoeoi", &pic.ctrl[0].autoeoi);
    state_field(obj, 1, (char *)"pic.ctrl[1].autoeoi", &pic.ctrl[1].autoeoi);
    state_field(obj, 1, (char *)"pic.ctrl[0].rotate_on_autoeoi", &pic.ctrl[0].rotate_on_autoeoi);
    state_field(obj, 1, (char *)"pic.ctrl[1].rotate_on_autoeoi", &pic.ctrl[1].rotate_on_autoeoi);
    state_field(obj, 1, (char *)"pic.ctrl[0].priority_base", &pic.ctrl[0].priority_base);
    state_field(obj, 1, (char *)"pic.ctrl[1].priority_base", &pic.ctrl[1].priority_base);
    state_field(obj, 1, (char *)"pic.ctrl[0].in_initialization", &pic.ctrl[0].in_initialization);
    state_field(obj, 1, (char *)"pic.ctrl[1].in_initialization", &pic.ctrl[1].in_initialization);
    state_field(obj, 1, (char *)"pic.ctrl[0].highest_priority_irq_to_send", &pic.ctrl[0].highest_priority_irq_to_send);
    state_field(obj, 1, (char *)"pic.ctrl[1].highest_priority_irq_to_send", &pic.ctrl[1].highest_priority_irq_to_send);
    state_field(obj, 1, (char *)"pic.ctrl[0].raised_intr_line", &pic.ctrl[0].raised_intr_line);
    state_field(obj, 1, (char *)"pic.ctrl[1].raised_intr_line", &pic.ctrl[1].raised_intr_line);
    state_field(obj, 1, (char *)"pic.ctrl[0].elcr", &pic.ctrl[0].elcr);
    state_field(obj, 1, (char *)"pic.ctrl[1].elcr", &pic.ctrl[1].elcr);
    FIELD(pic.irq_bus_value);
}
static void pic_reset(void)
{
    for (int i = 0; i < 2; i++) {
        struct pic_controller *ctrl = &pic.ctrl[i];
        ctrl->vector_offset         = 0;
        ctrl->imr                   = 0xFF;
        ctrl->irr                   = 0;
        ctrl->isr                   = 0;
        ctrl->in_initialization     = 0;
        ctrl->read_isr              = 0;
        ctrl->elcr                  = 0;
    }
}
static void pic_elcr_write(uint32_t addr, uint32_t data)
{
    pic.ctrl[addr & 1].elcr = data;
}
static uint32_t pic_elcr_read(uint32_t addr)
{
    return pic.ctrl[addr & 1].elcr;
}
static void pic_internal_update(struct pic_controller *self)
{
    int unmasked, isr;
    if (!(unmasked = self->irr & ~self->imr)) {
        return;
    }
    unmasked = rol(unmasked, self->priority_base);

    isr = rol(self->isr, self->priority_base);
    if ((self->ocw[3] & 0x60) == 0x60) {
        unmasked &= ~isr;
        for (int i = 0; i < 8; i++) {
            if (unmasked & (1 << i)) {
                self->highest_priority_irq_to_send = (self->priority_base + 1 + i) & 7;
                if (is_master(self)) {
                    cpu.cpu_raise_intr_line();
                    cpu.cpu_request_fast_return(EXIT_STATUS_IRQ);
                } else {
                    pic_lower_irq(2);
                    pic_raise_irq(2);
                }
                return;
            }
        }
    } else {
        for (int i = 0; i < 8; i++) {
            int mask = 1 << i;
            if (isr & mask)
                return;
            if (unmasked & (1 << i)) {
                self->highest_priority_irq_to_send = (self->priority_base + 1 + i) & 7;
                if (is_master(self)) {
                    cpu.cpu_raise_intr_line();
                    cpu.cpu_request_fast_return(EXIT_STATUS_IRQ);
                } else {
                    pic_lower_irq(2);
                    pic_raise_irq(2);
                }
                return;
            }
        }
    }
    return;
}
static uint8_t pic_internal_get_interrupt(struct pic_controller *self)
{
    int irq = self->highest_priority_irq_to_send, irq_mask = 1 << irq;
    if (!(self->irr & irq_mask)) {
        return self->vector_offset | 7;
    }

    self->irr ^= irq_mask;
    if (self->autoeoi) {
        if (self->rotate_on_autoeoi)
            self->priority_base = irq;
    } else
        self->isr |= irq_mask;
    if (is_master(self) && irq == 2)
        return pic_internal_get_interrupt(&pic.ctrl[1]);
    else
        return self->vector_offset + irq;
}
uint8_t pic_get_interrupt(void)
{
    if (apic_has_interrupt())
        return apic_get_interrupt();
    cpu.cpu_lower_intr_line();
    int x = pic_internal_get_interrupt(&pic.ctrl[0]);
    return x;
}
static void pic_internal_raise_irq(struct pic_controller *self, int irq)
{
    int mask = 1 << irq;
    if (!(self->pin_state & mask)) {
        self->pin_state |= mask;
        self->irr |= mask;
        pic_internal_update(self);
    }
}
static void pic_internal_lower_irq(struct pic_controller *self, int irq)
{
    int mask = 1 << irq;
    self->irr &= ~mask;
    self->pin_state &= ~mask;
    if (!is_master(self) && !self->irr)
        pic_lower_irq(2);
}
void pic_raise_irq(int a)
{
    ioapic_raise_irq(a);
    pic_internal_raise_irq(&pic.ctrl[a > 7], a & 7);
}
void pic_lower_irq(int a)
{
    ioapic_lower_irq(a);
    pic_internal_lower_irq(&pic.ctrl[a > 7], a & 7);
}
static inline void pic_clear_specific(struct pic_controller *self, int irq)
{
    self->isr &= ~(1 << irq);
}
static inline void pic_set_priority(struct pic_controller *self, int irq)
{
    self->priority_base = irq;
}
static inline void pic_clear_highest_priority(struct pic_controller *self)
{
    uint8_t highest = (self->priority_base + 1) & 7;
    for (int i = 0; i < 8; i++) {
        uint8_t mask = 1 << ((highest + i) & 7);
        if (self->isr & mask) {
            self->isr ^= mask;
            return;
        }
    }
}
static void pic_write_icw(struct pic_controller *self, int id, uint8_t value)
{
    switch (id) {
        case 1:
            self->icw_index     = 2;
            self->icw[1]        = value;
            self->imr           = 0;
            self->isr           = 0;
            self->irr           = 0;
            self->priority_base = 7;
            break;
        case 2:
            self->vector_offset = value & ~7;
            self->icw[2]        = value;
            if (self->icw[1] & 2) {
                if (self->icw[1] & 1)
                    self->icw_index = 4;
                else
                    self->icw_index = 5;
            } else
                self->icw_index = 3;
            break;
        case 3:
            self->icw[3]    = value;
            self->icw_index = 5 ^ (self->icw[1] & 1);
            break;
        case 4:
            self->icw[4]    = value;
            self->autoeoi   = value & 2;
            self->icw_index = 5;
    }
    self->in_initialization = self->icw_index != 5;
}
static void pic_write_ocw(struct pic_controller *self, int index, int data)
{
    self->ocw[index] = data;
    switch (index) {
        case 1:
            self->imr = data;
            pic_internal_update(self);
            break;
        case 2: {
            int rotate = data & 0x80, specific = data & 0x40, eoi = data & 0x20, l = data & 7;
            if (eoi) {
                if (specific) {
                    pic_clear_specific(self, l);
                    if (rotate)
                        pic_set_priority(self, l);
                } else {
                    pic_clear_highest_priority(self);
                    if (rotate)
                        pic_set_priority(self, l);
                }
                pic_internal_update(self);
            } else {
                if (specific) {
                    if (rotate)
                        pic_set_priority(self, l);
                } else
                    self->rotate_on_autoeoi = rotate > 0;
            }
        } break;
        case 3: {
            if (data & 2)
                self->read_isr = data & 1;
        }
    }
}
static void pic_writeb(uint32_t addr, uint32_t data)
{
    struct pic_controller *self = &pic.ctrl[addr >> 7 & 1];
    if ((addr & 1) == 0) {
        switch (data >> 3 & 3) {
            case 0:
                pic_write_ocw(self, 2, data);
                break;
            case 1:
                pic_write_ocw(self, 3, data);
                break;
            default:
                self->in_initialization = 1;
                self->imr = self->isr = self->irr = 0;
                self->priority_base               = 7;
                self->autoeoi = self->rotate_on_autoeoi = 0;
                cpu.cpu_lower_intr_line();
                pic_write_icw(self, 1, data);
                break;
        }
    } else {
        if (self->in_initialization)
            pic_write_icw(self, self->icw_index, data);
        else
            pic_write_ocw(self, 1, data);
    }
}
static uint32_t pic_readb(uint32_t port)
{
    struct pic_controller *ctrl = &pic.ctrl[port >> 7 & 1];
    if (port & 1)
        return ctrl->imr;
    else
        return ctrl->read_isr ? ctrl->isr : ctrl->irr;
}
void pic_init(struct pc_settings *pc)
{
    io_register_write(0x20, 2, pic_writeb, NULL, NULL);
    io_register_read(0x20, 2, pic_readb, NULL, NULL);
    io_register_write(0xA0, 2, pic_writeb, NULL, NULL);
    io_register_read(0xA0, 2, pic_readb, NULL, NULL);
    if (pc->pci_enabled) {
        io_register_write(0x4D0, 2, pic_elcr_write, NULL, NULL);
        io_register_read(0x4D0, 2, pic_elcr_read, NULL, NULL);
    }
    io_register_reset(pic_reset);
    state_register(pic_state);
    pic.irq_bus_value = -1;
}