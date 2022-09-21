#include "cpu.h"
#include "../devices.h"
#include <math.h>


extern CPU cpu;


void CPU::fpu_set_control_word(uint16_t control_word)
{
    control_word |= 0x40;
    m_fpu.control_word = control_word;
    int rounding       = m_fpu.control_word >> 10 & 3;
    switch (rounding) {
        case FPU_ROUND_NEAREST:
            m_fpu.status.float_rounding_mode = float_round_nearest_even;
            break;
        case FPU_ROUND_DOWN:
            m_fpu.status.float_rounding_mode = float_round_down;
            break;
        case FPU_ROUND_UP:
            m_fpu.status.float_rounding_mode = float_round_up;
            break;
        case FPU_ROUND_TRUNCATE:
            m_fpu.status.float_rounding_mode = float_round_to_zero;
            break;
    }
    int precision = m_fpu.control_word >> 8 & 3;
    switch (precision) {
        case FPU_PRECISION_24:
            m_fpu.status.float_rounding_precision = 32;
            break;
        case FPU_PRECISION_53:
            m_fpu.status.float_rounding_precision = 64;
            break;
        case FPU_PRECISION_64:
            m_fpu.status.float_rounding_precision = 80;
            break;
    }
    m_fpu.status.float_exception_flags    = 0;
    m_fpu.status.float_nan_handling_mode  = float_first_operand_nan;
    m_fpu.status.flush_underflow_to_zero  = 0;
    m_fpu.status.float_suppress_exception = 0;
    m_fpu.status.float_exception_masks    = control_word & 0x3F;
    m_fpu.status.denormals_are_zeros      = 0;
}
void CPU::fpu_state(void)
{
    struct bjson_object *obj = state_obj((char *)"fpu", 9 + 16);
    state_field(obj, 4, (char *)"m_fpu.ftop", &m_fpu.ftop);
    state_field(obj, 2, (char *)"m_fpu.control_word", &m_fpu.control_word);
    state_field(obj, 2, (char *)"m_fpu.status_word", &m_fpu.status_word);
    state_field(obj, 2, (char *)"m_fpu.tag_word", &m_fpu.tag_word);
    state_field(obj, 4, (char *)"m_fpu.fpu_eip", &m_fpu.fpu_eip);
    state_field(obj, 4, (char *)"m_fpu.fpu_data_ptr", &m_fpu.fpu_data_ptr);
    state_field(obj, 2, (char *)"m_fpu.fpu_cs", &m_fpu.fpu_cs);
    state_field(obj, 2, (char *)"m_fpu.fpu_opcode", &m_fpu.fpu_opcode);
    state_field(obj, 2, (char *)"m_fpu.fpu_data_seg", &m_fpu.fpu_data_seg);
    char name[32];
    for (int i = 0; i < 8; i++) {
        sprintf(name, "m_fpu.st[%d].mantissa", i);
        state_field(obj, 8, name, &m_fpu.st[i].fraction);
        sprintf(name, "m_fpu.st[%d].exponent", i);
        state_field(obj, 2, name, &m_fpu.st[i].exp);
    }
    if (state_is_reading())
        fpu_set_control_word(m_fpu.control_word);
}
uint16_t CPU::fpu_get_status_word(void)
{
    return m_fpu.status_word | (m_fpu.ftop << 11);
}
int CPU::is_denormal(uint16_t exponent, uint64_t mantissa)
{
    return !(exponent & 0x7FFF) && mantissa;
}
int CPU::is_pseudo_denormal(uint16_t exponent, uint64_t mantissa)
{
    return is_denormal(exponent, mantissa) && !(mantissa & 0x8000000000000000ULL);
}
int CPU::is_zero(uint16_t exponent, uint64_t mantissa)
{
    return ((exponent & 0x7FFF) | mantissa) == 0;
}
int CPU::is_zero_any_sign(uint16_t exponent, uint64_t mantissa)
{
    if (is_zero(exponent, mantissa)) {
        if (exponent & 0x8000)
            return -1;
        else
            return 1;
    } else
        return 0;
}
int CPU::is_negative(uint16_t exponent, uint64_t mantissa)
{
    return !is_zero_any_sign(exponent, mantissa) && (exponent & 0x8000) != 0;
}
int CPU::is_invalid(uint16_t exponent, uint64_t mantissa)
{
    uint16_t exponent_without_sign = exponent & 0x7FFF;
    if (exponent_without_sign != 0)
        return (mantissa & 0x8000000000000000ULL) == 0;
    return 0;
}
int CPU::is_infinity(uint16_t exponent, uint64_t mantissa)
{
    if (((exponent & 0x7FFF) == 0x7FFF) && (mantissa == 0x8000000000000000ULL))
        return mantissa >> 15 ? -1 : 1;
    return 0;
}
int CPU::is_nan(uint16_t exponent, uint64_t mantissa)
{
    if (((exponent & 0x7FFF) == 0x7FFF) && (mantissa != 0x8000000000000000ULL))
        return 1 + ((mantissa & 0x4000000000000000ULL) != 0);
    return 0;
}
int CPU::fpu_get_tag_from_value(floatx80 *f)
{
    uint16_t exponent;
    uint64_t mantissa;
    do {
        exponent = (f)->exp;
        mantissa = (f)->fraction;
    } while (0);
    if ((exponent | mantissa) == 0)
        return FPU_TAG_ZERO;
    int x = 0;
    x |= is_infinity(exponent, mantissa);
    x |= is_denormal(exponent, mantissa);
    x |= is_pseudo_denormal(exponent, mantissa);
    x |= is_invalid(exponent, mantissa);
    x |= is_nan(exponent, mantissa);
    if (x)
        return FPU_TAG_SPECIAL;
    return FPU_TAG_VALID;
}
int CPU::fpu_get_tag(int st)
{
    return m_fpu.tag_word >> (((st + m_fpu.ftop) & 7) << 1) & 3;
}
void CPU::fpu_set_tag(int st, int v)
{
    int shift = ((st + m_fpu.ftop) & 7) << 1;
    m_fpu.tag_word &= ~(3 << shift);
    m_fpu.tag_word |= v << shift;
}
int CPU::fpu_exception_raised(int flags)
{
    return (m_fpu.status.float_exception_flags & ~m_fpu.status.float_exception_masks) & flags;
}
void CPU::fpu_stack_fault(void)
{
    m_fpu.status.float_exception_flags = (1 << 0) | (1 << 6);
}
void CPU::fpu_commit_sw(void)
{
    m_fpu.status_word |= m_partial_sw;
    m_fpu.status_word &= ~m_bits_to_clear | m_partial_sw;
    m_bits_to_clear = 0;
    m_partial_sw    = 0;
}
int CPU::fpu_check_exceptions2(int commit_sw)
{
    int flags               = m_fpu.status.float_exception_flags;
    int unmasked_exceptions = (flags & ~m_fpu.status.float_exception_masks) & 0x3F;
    if (flags & (1 << 5) && (flags & ((1 << 4) | (1 << 3)))) {
        flags &= ~(1 << 5);
        unmasked_exceptions &= ~(1 << 5);
    }
    if (flags & 0x10000) {
        flags &= ~(1 << 9);
    }
    if (flags & ((1 << 0) | (1 << 2) | (1 << 1))) {
        unmasked_exceptions &= (1 << 0) | (1 << 2) | (1 << 1);
        flags &= (1 << 0) | (1 << 2) | (1 << 1) | (1 << 6);
    }
    if (commit_sw)
        m_fpu.status_word |= flags;
    else
        m_partial_sw |= flags;
    if (unmasked_exceptions) {
        m_fpu.status_word |= 0x8080;
        if (unmasked_exceptions & ~(1 << 5))
            return 1;
        return 0;
    }
    return 0;
}
int CPU::fpu_check_exceptions(void)
{
    return fpu_check_exceptions2(1);
}
void CPU::fninit(void)
{
    fpu_set_control_word(0x37F);
    m_fpu.status_word  = 0;
    m_fpu.tag_word     = 0xFFFF;
    m_fpu.ftop         = 0;
    m_fpu.fpu_data_ptr = 0;
    m_fpu.fpu_data_seg = 0;
    m_fpu.fpu_eip      = 0;
    m_fpu.fpu_cs       = 0;
    m_fpu.fpu_opcode   = 0;
}
int CPU::fpu_nm_check(void)
{
    if (m_cr[0] & (4 | 8))
        do {
            cpu_exception(7, 0);
            return 1;
        } while (0);
    return 0;
}
floatx80 *CPU::fpu_get_st_ptr(int st)
{
    return &m_fpu.st[(m_fpu.ftop + st) & 7];
}
floatx80 CPU::fpu_get_st(int st)
{
    return m_fpu.st[(m_fpu.ftop + st) & 7];
}
void CPU::fpu_set_st(int st, floatx80 data)
{
    fpu_set_tag(st, fpu_get_tag_from_value(&data));
    m_fpu.st[(m_fpu.ftop + st) & 7] = data;
}
int CPU::fpu_check_stack_overflow(int st)
{
    int tag = fpu_get_tag(st);
    if (tag != FPU_TAG_EMPTY) {
        m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (1) << 9;
        fpu_stack_fault();
        return 1;
    }
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
    return 0;
}
int CPU::fpu_check_stack_underflow(int st, int commit_sw)
{
    int tag = fpu_get_tag(st);
    if (tag == FPU_TAG_EMPTY) {
        fpu_stack_fault();
        if (commit_sw)
            m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (1) << 9;
        else
            m_partial_sw = 1 << 9;
        return 1;
    }
    if (commit_sw)
        m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
    else
        m_bits_to_clear = 1 << 9;
    return 0;
}
int CPU::fpu_exception_masked(int excep)
{
    if (excep == (1 << 6))
        excep = (1 << 0);
    return (m_fpu.control_word & excep);
}
int CPU::fpu_push(floatx80 data)
{
    m_fpu.ftop = (m_fpu.ftop - 1) & 7;
    fpu_set_st(0, data);
    return 0;
}
void CPU::fpu_pop()
{
    fpu_set_tag(0, FPU_TAG_EMPTY);
    m_fpu.ftop = (m_fpu.ftop + 1) & 7;
}
void CPU::fpu_update_pointers(uint32_t opcode)
{
    m_fpu.fpu_cs     = m_seg[1];
    m_fpu.fpu_eip    = (m_phys_eip + m_eip_phys_bias);
    m_fpu.fpu_opcode = opcode;
}
void CPU::fpu_update_pointers2(uint32_t opcode, uint32_t virtaddr, uint32_t seg)
{
    m_fpu.fpu_cs       = m_seg[1];
    m_fpu.fpu_eip      = (m_phys_eip + m_eip_phys_bias);
    m_fpu.fpu_opcode   = opcode;
    m_fpu.fpu_data_ptr = virtaddr;
    m_fpu.fpu_data_seg = m_seg[seg];
}
int CPU::write_float32(uint32_t linaddr, float32 src)
{
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = src, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    return 0;
}
int CPU::write_float64(uint32_t linaddr, float64 dest)
{
    uint64_t x = dest;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = (uint32_t)x, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = (uint32_t)(x >> 32),
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    return 0;
}
int CPU::fpu_check_push(void)
{
    if (fpu_check_stack_overflow(-1)) {
        fpu_check_exceptions();
        if (m_fpu.control_word & (1 << 0)) {
            fpu_push(IndefiniteNaN);
        } else
            m_fpu.status_word |= 0x80;
        return 1;
    }
    return 0;
}
int CPU::fpu_store_f80(uint32_t linaddr, floatx80 *data)
{
    uint16_t exponent;
    uint64_t mantissa;
    do {
        exponent = (data)->exp;
        mantissa = (data)->fraction;
    } while (0);
    int shift = m_tlb_shift_write;
    do {
        uint32_t addr_ = linaddr, shift_ = shift, data_ = (uint32_t)mantissa, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> shift, shift))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = shift, data_ = (uint32_t)(mantissa >> 32), tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> shift, shift))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 8, shift_ = shift, data_ = exponent, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> shift, shift))
                return 1;
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    return 0;
}
int CPU::fpu_read_f80(uint32_t linaddr, floatx80 *data)
{
    uint16_t exponent;
    uint32_t low, hi;
    int      shift = m_tlb_shift_read;
    do {
        uint32_t addr_ = linaddr, shift_ = shift, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> shift, shift))
                low = m_read_result;
            else
                return 1;
        } else
            low = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = shift, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> shift, shift))
                hi = m_read_result;
            else
                return 1;
        } else
            hi = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 8, shift_ = shift, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> shift, shift))
                exponent = m_read_result;
            else
                return 1;
        } else
            exponent = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        (data)->exp      = exponent;
        (data)->fraction = (uint64_t)low | (uint64_t)hi << 32;
    } while (0);
    return 0;
}
int CPU::fpu_fcom(floatx80 op1, floatx80 op2, int unordered)
{
    int relation = floatx80_compare_internal(op1, op2, unordered, &m_fpu.status);
    if (fpu_check_exceptions())
        return 1;
    int bad           = relation == float_relation_unordered;
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (bad | (relation == float_relation_less)) << 8;
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (bad) << 10;
    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (bad | (relation == float_relation_equal)) << 14;
    return 0;
}
int CPU::fpu_fcomi(floatx80 op1, floatx80 op2, int unordered)
{
    int relation = floatx80_compare_internal(op1, op2, unordered, &m_fpu.status);
    if (fpu_check_exceptions())
        return 1;
    int bad = relation == float_relation_unordered;
    int cf  = bad | (relation == float_relation_less);
    int pf  = bad;
    int zf  = bad | (relation == float_relation_equal);
    cpu_set_cf(cf);
    cpu_set_pf(pf);
    cpu_set_zf(zf);
    return 0;
}
int CPU::fstenv(uint32_t linaddr, int code16)
{
    for (int i = 0; i < 8; i++) {
        if (fpu_get_tag(i) != FPU_TAG_EMPTY)
            fpu_set_tag(i, fpu_get_tag_from_value(&m_fpu.st[(m_fpu.ftop + i) & 7]));
    }
    int x = m_tlb_shift_write;
    if (!code16) {
        do {
            uint32_t addr_ = linaddr, shift_ = x, data_ = 0xFFFF0000 | m_fpu.control_word,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> x, x))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = linaddr + 4, shift_ = x, data_ = 0xFFFF0000 | fpu_get_status_word(),
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> x, x))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = linaddr + 8, shift_ = x, data_ = 0xFFFF0000 | m_fpu.tag_word,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> x, x))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        if (m_cr[0] & 1) {
            do {
                uint32_t addr_ = linaddr + 12, shift_ = x, data_ = m_fpu.fpu_eip, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 16, shift_ = x, data_ = m_fpu.fpu_cs | (m_fpu.fpu_opcode << 16),
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 20, shift_ = x, data_ = m_fpu.fpu_data_ptr, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 24, shift_ = x, data_ = 0xFFFF0000 | m_fpu.fpu_data_seg,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
        } else {
            uint32_t linear_fpu_eip  = m_fpu.fpu_eip + (m_fpu.fpu_cs << 4);
            uint32_t linear_fpu_data = m_fpu.fpu_data_ptr + (m_fpu.fpu_data_seg << 4);
            do {
                uint32_t addr_ = linaddr + 12, shift_ = x, data_ = linear_fpu_eip | 0xFFFF0000,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 16, shift_ = x,
                         data_ = (m_fpu.fpu_opcode & 0x7FF) | (linear_fpu_eip >> 4 & 0x0FFFF000),
                         tag   = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 20, shift_ = x, data_ = linear_fpu_data | 0xFFFF0000,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 24, shift_ = x, data_ = linear_fpu_data >> 4 & 0x0FFFF000,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
        }
    } else {
        do {
            uint32_t addr_ = linaddr, shift_ = x, data_ = m_fpu.control_word, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> x, x))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = linaddr + 2, shift_ = x, data_ = fpu_get_status_word(), tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> x, x))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = linaddr + 4, shift_ = x, data_ = m_fpu.tag_word, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> x, x))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        if (m_cr[0] & 1) {
            do {
                uint32_t addr_ = linaddr + 6, shift_ = x, data_ = m_fpu.fpu_eip, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 8, shift_ = x, data_ = m_fpu.fpu_cs, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 10, shift_ = x, data_ = m_fpu.fpu_data_ptr, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 12, shift_ = x, data_ = m_fpu.fpu_data_seg, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
        } else {
            uint32_t linear_fpu_eip  = m_fpu.fpu_eip + (m_fpu.fpu_cs << 4);
            uint32_t linear_fpu_data = m_fpu.fpu_data_ptr + (m_fpu.fpu_data_seg << 4);
            do {
                uint32_t addr_ = linaddr + 6, shift_ = x, data_ = linear_fpu_eip, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 8, shift_ = x,
                         data_ = (m_fpu.fpu_opcode & 0x7FF) | (linear_fpu_eip >> 4 & 0xF000),
                         tag   = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 10, shift_ = x, data_ = linear_fpu_data, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 12, shift_ = x, data_ = linear_fpu_data >> 4 & 0xF000,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> x, x))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
        }
    }
    return 0;
}
int CPU::fldenv(uint32_t linaddr, int code16)
{
    uint32_t temp32;
    if (!code16) {
        do {
            uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp32 = m_read_result;
                else
                    return 1;
            } else
                temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        fpu_set_control_word(temp32);
        do {
            uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.status_word = m_read_result;
                else
                    return 1;
            } else
                m_fpu.status_word = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_fpu.ftop = m_fpu.status_word >> 11 & 7;
        m_fpu.status_word &= ~(7 << 11);
        do {
            uint32_t addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.tag_word = m_read_result;
                else
                    return 1;
            } else
                m_fpu.tag_word = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        if (m_cr[0] & 1) {
            do {
                uint32_t addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_eip = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 16, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_cs     = temp32 & 0xFFFF;
            m_fpu.fpu_opcode = temp32 >> 16 & 0x7FF;
            do {
                uint32_t addr_ = linaddr + 20, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_ptr = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_data_ptr = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 24, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_seg = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_data_seg = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
        } else {
            m_fpu.fpu_cs  = 0;
            m_fpu.fpu_eip = 0;
            do {
                uint32_t addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_eip = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 16, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_opcode = temp32 & 0x7FF;
            m_fpu.fpu_eip |= temp32 << 4 & 0xFFFF0000;
            do {
                uint32_t addr_ = linaddr + 20, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_data_ptr = temp32 & 0xFFFF;
            do {
                uint32_t addr_ = linaddr + 24, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_eip |= temp32 << 4 & 0xFFFF0000;
        }
    } else {
        do {
            uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp32 = m_read_result;
                else
                    return 1;
            } else
                temp32 = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        fpu_set_control_word(temp32);
        do {
            uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.status_word = m_read_result;
                else
                    return 1;
            } else
                m_fpu.status_word = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_fpu.ftop = m_fpu.status_word >> 11 & 7;
        m_fpu.status_word &= ~(7 << 11);
        do {
            uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_fpu.tag_word = m_read_result;
                else
                    return 1;
            } else
                m_fpu.tag_word = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        if (m_cr[0] & 1) {
            do {
                uint32_t addr_ = linaddr + 6, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_eip = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_cs = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_cs = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 10, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_ptr = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_data_ptr = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_data_seg = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_data_seg = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
        } else {
            m_fpu.fpu_cs  = 0;
            m_fpu.fpu_eip = 0;
            do {
                uint32_t addr_ = linaddr + 6, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        m_fpu.fpu_eip = m_read_result;
                    else
                        return 1;
                } else
                    m_fpu.fpu_eip = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_opcode = temp32 & 0x7FF;
            m_fpu.fpu_eip |= temp32 << 4 & 0xF0000;
            do {
                uint32_t addr_ = linaddr + 10, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_data_ptr = temp32 & 0xFFFF;
            do {
                uint32_t addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        temp32 = m_read_result;
                    else
                        return 1;
                } else
                    temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_fpu.fpu_eip |= temp32 << 4 & 0xF0000;
        }
    }
    if (m_fpu.status_word & ~m_fpu.control_word & 0x3F)
        m_fpu.status_word |= 0x8080;
    else
        m_fpu.status_word &= ~0x8080;
    return 0;
}
void CPU::fpu_watchpoint(void)
{
}
void CPU::fpu_watchpoint2(void)
{
}
int CPU::fpu_reg_op(struct decoded_instruction *i, uint32_t flags)
{
    uint32_t opcode = i->imm32;
    floatx80 temp80;
    if (fpu_nm_check())
        return 1;
    fpu_watchpoint();
    m_fpu.status.float_exception_flags = 0;
    int smaller_opcode                 = (opcode >> 5 & 0x38) | (opcode >> 3 & 7);
    switch (smaller_opcode) {
        case (0xD8 & 7) << 3 | 0:
        case (0xD8 & 7) << 3 | 1:
        case (0xD8 & 7) << 3 | 4:
        case (0xD8 & 7) << 3 | 5:
        case (0xD8 & 7) << 3 | 6:
        case (0xD8 & 7) << 3 | 7:
        case (0xDC & 7) << 3 | 0:
        case (0xDC & 7) << 3 | 1:
        case (0xDC & 7) << 3 | 4:
        case (0xDC & 7) << 3 | 5:
        case (0xDC & 7) << 3 | 6:
        case (0xDC & 7) << 3 | 7:
        case (0xDE & 7) << 3 | 0:
        case (0xDE & 7) << 3 | 1:
        case (0xDE & 7) << 3 | 4:
        case (0xDE & 7) << 3 | 5:
        case (0xDE & 7) << 3 | 6:
        case (0xDE & 7) << 3 | 7: {
            int      st_index = opcode & 7;
            floatx80 dst;
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(st_index, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            switch (smaller_opcode & 7) {
                case 0:
                    dst = floatx80_add(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 1:
                    dst = floatx80_mul(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 4:
                    dst = floatx80_sub(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 5:
                    dst = floatx80_sub(fpu_get_st(st_index), fpu_get_st(0), &m_fpu.status);
                    break;
                case 6:
                    dst = floatx80_div(fpu_get_st(0), fpu_get_st(st_index), &m_fpu.status);
                    break;
                case 7:
                    dst = floatx80_div(fpu_get_st(st_index), fpu_get_st(0), &m_fpu.status);
                    break;
            }
            if (!fpu_check_exceptions()) {
                if (smaller_opcode & 32) {
                    fpu_set_st(st_index, dst);
                    if (smaller_opcode & 16)
                        fpu_pop();
                } else
                    fpu_set_st(0, dst);
            }
            break;
        }
        case (0xD8 & 7) << 3 | 2:
        case (0xD8 & 7) << 3 | 3:
        case (0xDC & 7) << 3 | 2:
        case (0xDC & 7) << 3 | 3:
        case (0xDE & 7) << 3 | 2: {
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
            }
            fpu_update_pointers(opcode);
            if (!fpu_fcom(fpu_get_st(0), fpu_get_st(opcode & 7), 0)) {
                if (smaller_opcode & 1 || smaller_opcode == ((0xDE & 7) << 3 | 2))
                    fpu_pop();
            }
            break;
        }
        case (0xD9 & 7) << 3 | 0:
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(opcode & 7, 1) || fpu_check_push())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            temp80 = fpu_get_st(opcode & 7);
            fpu_push(temp80);
            break;
        case (0xD9 & 7) << 3 | 1:
        case (0xDD & 7) << 3 | 1:
        case (0xDF & 7) << 3 | 1:
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            temp80 = fpu_get_st(0);
            fpu_set_st(0, fpu_get_st(opcode & 7));
            fpu_set_st(opcode & 7, temp80);
            break;
        case (0xD9 & 7) << 3 | 2:
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            break;
        case (0xD9 & 7) << 3 | 4:
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            if ((opcode & 7) != 5)
                if (fpu_check_stack_underflow(0, 1))
                    do {
                        fpu_watchpoint2();
                        return 0;
                    } while (0);
            temp80 = fpu_get_st(0);
            switch (opcode & 7) {
                case 0:
                    floatx80_chs(&temp80);
                    break;
                case 1:
                    floatx80_abs(&temp80);
                    break;
                case 4:
                    if (fpu_fcom(temp80, Zero, 0))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    return 0;
                case 5: {
                    int      unordered = 0;
                    uint16_t exponent;
                    uint64_t mantissa;
                    do {
                        exponent = (&temp80)->exp;
                        mantissa = (&temp80)->fraction;
                    } while (0);
                    if (fpu_get_tag(0) == FPU_TAG_EMPTY)
                        unordered = 5;
                    else {
                        if (is_invalid(exponent, mantissa))
                            unordered = 0;
                        else if (is_nan(exponent, mantissa))
                            unordered = 1;
                        else if (is_infinity(exponent, mantissa))
                            unordered = 3;
                        else if (is_zero_any_sign(exponent, mantissa))
                            unordered = 4;
                        else if (is_denormal(exponent, mantissa))
                            unordered = 6;
                        else
                            unordered = 2;
                    }
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (unordered & 1) << 8;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (exponent >> 15 & 1) << 9;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (unordered >> 1 & 1) << 10;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (unordered >> 2 & 1) << 14;
                    return 0;
                }
            }
            fpu_set_st(0, temp80);
            break;
        case (0xD9 & 7) << 3 | 5:
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            if (fpu_check_push())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_push(*Constants[opcode & 7]);
            break;
        case (0xD9 & 7) << 3 | 6: {
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            floatx80 res, temp;
            int      temp2, old_rounding;
            switch (opcode & 7) {
                case 0:
                    if (fpu_check_stack_underflow(0, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    res = f2xm1(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions())
                        fpu_set_st(0, res);
                    break;
                case 1:
                    if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    old_rounding                          = m_fpu.status.float_rounding_precision;
                    m_fpu.status.float_rounding_precision = 80;
                    res                                   = fyl2x(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    m_fpu.status.float_rounding_precision = old_rounding;
                    if (!fpu_check_exceptions()) {
                        fpu_set_st(1, res);
                        fpu_pop();
                    }
                    break;
                case 2:
                    if (fpu_check_stack_underflow(0, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    res = fpu_get_st(0);
                    if (!ftan(&res, &m_fpu.status))
                        fpu_set_st(0, res);
                    break;
                case 3:
                    if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    res = fpatan(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        fpu_pop();
                        fpu_set_st(0, res);
                    }
                    break;
                case 4:
                    if (fpu_check_stack_underflow(0, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    if (fpu_check_stack_overflow(-1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    temp = fpu_get_st(0);
                    res  = floatx80_extract(&temp, &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        fpu_set_st(0, res);
                        fpu_push(temp);
                    }
                    break;
                case 5: {
                    floatx80 st0 = fpu_get_st(0), st1 = fpu_get_st(1);
                    uint64_t quo;
                    temp2 = floatx80_ieee754_remainder(st0, st1, &temp, &quo, &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        if (!(temp2 < 0)) {
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (0) << 8;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (0) << 10;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (0) << 14;
                            if (temp2 > 0) {
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                            } else {
                                if (quo & 1)
                                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (1) << 9;
                                if (quo & 2)
                                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                                if (quo & 4)
                                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                            }
                        }
                        fpu_set_st(0, temp);
                    }
                } break;
                case 6:
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                    m_fpu.ftop        = (m_fpu.ftop - 1) & 7;
                    break;
                case 7:
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                    m_fpu.ftop        = (m_fpu.ftop + 1) & 7;
                    break;
            }
        } break;

        case (0xD9 & 7) << 3 | 7: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            int      flags, pop = 0;
            floatx80 dest;
            uint64_t quotient;
            switch (opcode & 7) {
                case 0:
                    if (fpu_check_stack_underflow(1, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    flags = floatx80_remainder(fpu_get_st(0), fpu_get_st(1), &dest, &quotient, &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        if (flags < 0) {
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (0) << 8;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (0) << 10;
                            m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (0) << 14;
                        } else {
                            if (flags != 0) {
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (0) << 8;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (0) << 9;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (0) << 14;
                            } else {
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (quotient >> 2 & 1) << 8;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 9)) | (quotient & 1) << 9;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (0) << 10;
                                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (quotient >> 1 & 1) << 14;
                            }
                        }
                        fpu_set_st(0, dest);
                    }
                    break;
                case 1:
                    if (fpu_check_stack_underflow(1, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    dest = fyl2xp1(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    if (!fpu_check_exceptions()) {
                        fpu_pop();
                        fpu_set_st(0, dest);
                    }
                    return 0;
                case 2:
                    dest = floatx80_sqrt(fpu_get_st(0), &m_fpu.status);
                    break;
                case 3: {
                    if (fpu_check_stack_overflow(-1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    floatx80 sinres, cosres;
                    if (fsincos(fpu_get_st(0), &sinres, &cosres, &m_fpu.status) == -1) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                    } else if (!fpu_check_exceptions()) {
                        fpu_set_st(0, sinres);
                        fpu_push(cosres);
                    }
                    return 0;
                }
                case 4:
                    dest = floatx80_round_to_int(fpu_get_st(0), &m_fpu.status);
                    break;
                case 5:
                    if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1))
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    dest = floatx80_scale(fpu_get_st(0), fpu_get_st(1), &m_fpu.status);
                    break;
                case 6:
                    dest = fpu_get_st(0);
                    if (fsin(&dest, &m_fpu.status) == -1) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    }
                    break;
                case 7:
                    dest = fpu_get_st(0);
                    if (fcos(&dest, &m_fpu.status) == -1) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                        do {
                            fpu_watchpoint2();
                            return 0;
                        } while (0);
                    }
                    break;
            }
            if (!fpu_check_exceptions()) {
                fpu_set_st(0, dest);
                if (pop)
                    fpu_pop();
            }
            break;
        }
        case (0xDA & 7) << 3 | 0:
        case (0xDB & 7) << 3 | 0:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if (cpu_get_cf() ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDB & 7) << 3 | 1:
        case (0xDA & 7) << 3 | 1:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if ((m_lr == 0) ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDB & 7) << 3 | 2:
        case (0xDA & 7) << 3 | 2:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if (((m_lr == 0) || cpu_get_cf()) ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDB & 7) << 3 | 3:
        case (0xDA & 7) << 3 | 3:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) && fpu_check_stack_underflow(opcode & 7, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if (cpu_get_pf() ^ (smaller_opcode >> 3 & 1))
                fpu_set_st(0, fpu_get_st(opcode & 7));
            break;
        case (0xDA & 7) << 3 | 5:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if ((opcode & 7) == 1) {
                if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(1, 1)) {
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                }
                if (fpu_fcom(fpu_get_st(0), fpu_get_st(1), 1))
                    do {
                        fpu_watchpoint2();
                        return 0;
                    } while (0);
                if (!fpu_check_exceptions()) {
                    fpu_pop();
                    fpu_pop();
                }
            }
            break;
        case (0xDB & 7) << 3 | 4:
            switch (opcode & 7) {
                case 0 ... 1:
                case 4:
                    return 0;
                case 2:
                    m_fpu.status_word &= ~(0x80FF);
                    break;
                case 3:
                    fninit();
                    break;
                default:
                    do {
                        cpu_exception(6, 0);
                        return 1;
                    } while (0);
            }
            break;
        case (0xDB & 7) << 3 | 5:
        case (0xDB & 7) << 3 | 6:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            cpu_set_eflags(cpu_get_eflags() & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01));
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                cpu_set_zf(1);
                cpu_set_pf(1);
                cpu_set_cf(1);
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            }
            if (fpu_fcomi(fpu_get_st(0), fpu_get_st(opcode & 7), smaller_opcode & 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            break;
        case (0xD9 & 7) << 3 | 3:
        case (0xDD & 7) << 3 | 2:
        case (0xDD & 7) << 3 | 3:
        case (0xDF & 7) << 3 | 2:
        case (0xDF & 7) << 3 | 3: {
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1)) {
                if (fpu_exception_masked((1 << 6)))
                    fpu_pop();
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            }
            fpu_set_st(opcode & 7, fpu_get_st(0));
            if (smaller_opcode & 1 || (smaller_opcode & ~1) == ((0xDF & 7) << 3 | 2))
                fpu_pop();
            break;
        }
        case (0xDD & 7) << 3 | 0:
        case (0xDF & 7) << 3 | 0: {
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            fpu_set_tag(opcode & 7, FPU_TAG_EMPTY);
            if (smaller_opcode == ((0xDF & 7) << 3 | 0))
                fpu_pop();
            break;
        }
        case (0xDD & 7) << 3 | 4:
        case (0xDD & 7) << 3 | 5:
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
            }
            if (fpu_fcom(fpu_get_st(0), fpu_get_st(opcode & 7), 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if (!fpu_check_exceptions()) {
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            break;
        case (0xDE & 7) << 3 | 3:
            if (fpu_fwait())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_update_pointers(opcode);
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                if (!fpu_check_exceptions()) {
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                    m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                }
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            }
            if (fpu_fcom(fpu_get_st(0), fpu_get_st(1), 0))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_pop();
            fpu_pop();
            break;
        case (0xDF & 7) << 3 | 4:
            if ((opcode & 7) != 0)
                do {
                    cpu_exception(6, 0);
                    return 1;
                } while (0);
            m_reg16[0] = fpu_get_status_word();
            break;
        case (0xDF & 7) << 3 | 5:
        case (0xDF & 7) << 3 | 6: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers(opcode);
            cpu_set_eflags(cpu_get_eflags() & ~(0x800 | 0x80 | 0x40 | 0x10 | 0x04 | 0x01));
            if (fpu_check_stack_underflow(0, 1) || fpu_check_stack_underflow(opcode & 7, 1)) {
                cpu_set_zf(1);
                cpu_set_pf(1);
                cpu_set_cf(1);
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            }
            if (fpu_fcomi(fpu_get_st(0), fpu_get_st(opcode & 7), smaller_opcode & 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_pop();
            break;
        }
        case (0xDA & 7) << 3 | 4:
        case (0xDA & 7) << 3 | 6:
        case (0xDA & 7) << 3 | 7:
        case (0xDD & 7) << 3 | 6:
        case (0xDD & 7) << 3 | 7:
        case (0xDF & 7) << 3 | 7: {
            int major_opcode = opcode >> 8 | 0xD8;
            (void)(major_opcode);

            do {
                cpu_exception(6, 0);
                return 1;
            } while (0);
            break;
        }
        default: {
            int major_opcode = opcode >> 8 | 0xD8;
            do {

                util_abort();
                abort();
            } while (0);
        }
    }
    fpu_watchpoint2();
    return 0;
}
int CPU::fpu_mem_op(struct decoded_instruction *i, uint32_t virtaddr, uint32_t seg)
{
    uint32_t opcode = i->imm32, linaddr = virtaddr + m_seg_base[seg];
    floatx80 temp80;
    float64  temp64;
    float32  temp32;
    if (fpu_nm_check())
        return 1;
    fpu_watchpoint();
    m_fpu.status.float_exception_flags = 0;
    int smaller_opcode                 = (opcode >> 5 & 0x38) | (opcode >> 3 & 7);
    switch (smaller_opcode) {
        case (0xD8 & 7) << 3 | 0:
        case (0xD8 & 7) << 3 | 1:
        case (0xD8 & 7) << 3 | 2:
        case (0xD8 & 7) << 3 | 3:
        case (0xD8 & 7) << 3 | 4:
        case (0xD8 & 7) << 3 | 5:
        case (0xD8 & 7) << 3 | 6:
        case (0xD8 & 7) << 3 | 7:
        case (0xD9 & 7) << 3 | 0:
        case (0xDA & 7) << 3 | 0:
        case (0xDA & 7) << 3 | 1:
        case (0xDA & 7) << 3 | 2:
        case (0xDA & 7) << 3 | 3:
        case (0xDA & 7) << 3 | 4:
        case (0xDA & 7) << 3 | 5:
        case (0xDA & 7) << 3 | 6:
        case (0xDA & 7) << 3 | 7:
        case (0xDB & 7) << 3 | 0:
        case (0xDC & 7) << 3 | 0:
        case (0xDC & 7) << 3 | 1:
        case (0xDC & 7) << 3 | 2:
        case (0xDC & 7) << 3 | 3:
        case (0xDC & 7) << 3 | 4:
        case (0xDC & 7) << 3 | 5:
        case (0xDC & 7) << 3 | 6:
        case (0xDC & 7) << 3 | 7:
        case (0xDD & 7) << 3 | 0:
        case (0xDE & 7) << 3 | 0:
        case (0xDE & 7) << 3 | 1:
        case (0xDE & 7) << 3 | 2:
        case (0xDE & 7) << 3 | 3:
        case (0xDE & 7) << 3 | 4:
        case (0xDE & 7) << 3 | 5:
        case (0xDE & 7) << 3 | 6:
        case (0xDE & 7) << 3 | 7:
        case (0xDF & 7) << 3 | 0: {
            if (fpu_fwait())
                return 1;
            switch (opcode >> 9 & 3) {
                case 0:
                    do {
                        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                temp32 = m_read_result;
                            else
                                return 1;
                        } else
                            temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                    } while (0);
                    temp80 = float32_to_floatx80(temp32, &m_fpu.status);
                    break;
                case 1:
                    do {
                        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                temp32 = m_read_result;
                            else
                                return 1;
                        } else
                            temp32 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                    } while (0);
                    temp80 = int32_to_floatx80(temp32);
                    break;
                case 2: {
                    uint32_t low, hi;
                    uint64_t res;
                    do {
                        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                low = m_read_result;
                            else
                                return 1;
                        } else
                            low = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                    } while (0);
                    do {
                        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 3)) {
                            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                hi = m_read_result;
                            else
                                return 1;
                        } else
                            hi = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                    } while (0);
                    res    = (uint64_t)low | (uint64_t)hi << 32;
                    temp80 = float64_to_floatx80(res, &m_fpu.status);
                    break;
                }
                case 3: {
                    do {
                        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if (((addr_ | tag >> shift_) & 1)) {
                            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                temp32 = m_read_result;
                            else
                                return 1;
                        } else
                            temp32 = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                    } while (0);
                    temp80 = int32_to_floatx80((int16_t)temp32);
                    break;
                }
            }
            fpu_update_pointers2(opcode, virtaddr, seg);
            int op = smaller_opcode & 15;
            if ((op & 8) == 0) {
                if (fpu_check_stack_underflow(0, 1)) {
                    if ((smaller_opcode & 14) == 2) {
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 8)) | (1) << 8;
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 10)) | (1) << 10;
                        m_fpu.status_word = (m_fpu.status_word & ~(1 << 14)) | (1) << 14;
                    }
                    return 0;
                }
            } else {
                if (fpu_check_push())
                    do {
                        fpu_watchpoint2();
                        return 0;
                    } while (0);
            }
            floatx80 st0 = fpu_get_st(0);
            switch (op) {
                case 0:
                    st0 = floatx80_add(st0, temp80, &m_fpu.status);
                    break;
                case 1:
                    st0 = floatx80_mul(st0, temp80, &m_fpu.status);
                    break;
                case 2:
                case 3:
                    if (!fpu_fcom(st0, temp80, 0)) {
                        if (smaller_opcode & 1)
                            fpu_pop();
                    }
                    return 0;
                case 4:
                    st0 = floatx80_sub(st0, temp80, &m_fpu.status);
                    break;
                case 5:
                    st0 = floatx80_sub(temp80, st0, &m_fpu.status);
                    break;
                case 6:
                    st0 = floatx80_div(st0, temp80, &m_fpu.status);
                    break;
                case 7:
                    st0 = floatx80_div(temp80, st0, &m_fpu.status);
                    break;
                default:
                    if (!fpu_check_exceptions())
                        fpu_push(temp80);
                    return 0;
            }
            if (!fpu_check_exceptions())
                fpu_set_st(0, st0);
            break;
        }
        case (0xD9 & 7) << 3 | 2:
        case (0xD9 & 7) << 3 | 3: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            temp32 = floatx80_to_float32(fpu_get_st(0), &m_fpu.status);
            if (!fpu_check_exceptions2(0)) {
                if (write_float32(linaddr, temp32))
                    return 1;
                fpu_commit_sw();
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            break;
        }
        case (0xD9 & 7) << 3 | 5: {
            uint16_t cw;
            do {
                uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        cw = m_read_result;
                    else
                        return 1;
                } else
                    cw = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            fpu_set_control_word(cw);
            break;
        }
        case (0xD9 & 7) << 3 | 6: {
            int is16 = (i->flags & (1 << 25));
            if (fstenv(linaddr, is16))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            break;
        }
        case (0xD9 & 7) << 3 | 7:
            do {
                uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_fpu.control_word,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            break;
        case (0xDB & 7) << 3 | 1:
        case (0xDB & 7) << 3 | 2:
        case (0xDB & 7) << 3 | 3:
        case (0xDD & 7) << 3 | 1:
        case (0xDF & 7) << 3 | 1:
        case (0xDF & 7) << 3 | 2:
        case (0xDF & 7) << 3 | 3: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            switch (smaller_opcode >> 4 & 3) {
                case 1: {
                    uint32_t res;
                    if (smaller_opcode & 2)
                        res = floatx80_to_int32(fpu_get_st(0), &m_fpu.status);
                    else
                        res = floatx80_to_int32_round_to_zero(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions2(0))
                        do {
                            uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = res,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else
                                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
                        } while (0);
                    break;
                }
                case 2: {
                    uint64_t res;
                    if (smaller_opcode & 2)
                        res = floatx80_to_int64(fpu_get_st(0), &m_fpu.status);
                    else
                        res = floatx80_to_int64_round_to_zero(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions2(0)) {
                        do {
                            uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = res,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else
                                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
                        } while (0);
                        do {
                            uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = res >> 32,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 3)) {
                                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else
                                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
                        } while (0);
                    }
                    break;
                }
                case 3: {
                    uint16_t res;
                    if (smaller_opcode & 2)
                        res = floatx80_to_int16(fpu_get_st(0), &m_fpu.status);
                    else
                        res = floatx80_to_int16_round_to_zero(fpu_get_st(0), &m_fpu.status);
                    if (!fpu_check_exceptions2(0))
                        do {
                            uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = res,
                                     tag = m_tlb_tags[addr_ >> 12];
                            if (((addr_ | tag >> shift_) & 1)) {
                                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                                    return 1;
                            } else
                                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
                        } while (0);
                    break;
                }
            }
            if (!fpu_check_exceptions2(0)) {
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            fpu_commit_sw();
            break;
        }
        case (0xD9 & 7) << 3 | 4: {
            int is16 = (i->flags & (1 << 25));
            if (fldenv(linaddr, is16))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            break;
        }
        case (0xDB & 7) << 3 | 5: {
            if (fpu_fwait())
                return 1;
            if (fpu_read_f80(linaddr, &temp80))
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_overflow(-1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_push(temp80);
            break;
        }
        case (0xDB & 7) << 3 | 7: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 1))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            if (fpu_store_f80(linaddr, fpu_get_st_ptr(0)))
                return 1;
            fpu_pop();
            break;
        }
        case (0xDD & 7) << 3 | 2:
        case (0xDD & 7) << 3 | 3: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            temp64 = floatx80_to_float64(fpu_get_st(0), &m_fpu.status);
            if (!fpu_check_exceptions2(0)) {
                if (write_float64(linaddr, temp64))
                    return 1;
                fpu_commit_sw();
                if (smaller_opcode & 1)
                    fpu_pop();
            }
            break;
        }
        case (0xDD & 7) << 3 | 4: {
            int is16 = (i->flags & (1 << 25));
            if (fldenv(linaddr, is16))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            int offset = 14 << !is16;
            for (int i = 0; i < 8; i++) {
                if (fpu_read_f80(offset + linaddr, &m_fpu.st[(m_fpu.ftop + i) & 7]))
                    return 1;
                offset += 10;
            }
            break;
        }
        case (0xDD & 7) << 3 | 6: {
            int is16 = (i->flags & (1 << 25));
            if (fstenv(linaddr, is16))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            int offset = 14 << !is16;
            for (int i = 0; i < 8; i++) {
                if (fpu_store_f80(offset + linaddr, &m_fpu.st[(m_fpu.ftop + i) & 7]))
                    return 1;
                offset += 10;
            }
            fninit();
            break;
        }
        case (0xDD & 7) << 3 | 7:
            do {
                uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = fpu_get_status_word(),
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else
                    *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            break;
        case (0xDF & 7) << 3 | 4: {
            uint32_t low, high, higher;
            if (fpu_fwait())
                return 1;
            do {
                uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        low = m_read_result;
                    else
                        return 1;
                } else
                    low = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        high = m_read_result;
                    else
                        return 1;
                } else
                    high = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        higher = m_read_result;
                    else
                        return 1;
                } else
                    higher = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            fpu_update_pointers2(opcode, virtaddr, seg);
            uint64_t result = 0;
            int      sign   = higher & 0x8000;
            higher &= 0x7FFF;
            for (int i = 0; i < 4; i++) {
                result *= 100;
                uint8_t temp = low & 0xFF;
                result += temp - (6 * (temp >> 4));
                low >>= 8;
            }
            for (int i = 0; i < 4; i++) {
                result *= 100;
                uint8_t temp = high & 0xFF;
                result += temp - (6 * (temp >> 4));
                high >>= 8;
            }
            for (int i = 0; i < 2; i++) {
                result *= 100;
                uint8_t temp = higher & 0xFF;
                result += temp - (6 * (temp >> 4));
                higher >>= 8;
            }
            temp80 = int64_to_floatx80((uint64_t)sign << (64LL - 16LL) | result);
            if (fpu_check_push())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_push(temp80);
            break;
        }
        case (0xDF & 7) << 3 | 5: {
            uint32_t hi, low;
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            do {
                uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        low = m_read_result;
                    else
                        return 1;
                } else
                    low = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        hi = m_read_result;
                    else
                        return 1;
                } else
                    hi = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            temp80 = int64_to_floatx80((uint64_t)low | (uint64_t)hi << 32);
            if (fpu_check_push())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            fpu_push(temp80);
            break;
        }
        case (0xDF & 7) << 3 | 6: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            floatx80 st0 = fpu_get_st(0);
            uint64_t bcd = floatx80_to_int64(st0, &m_fpu.status);
            if (fpu_check_exceptions())
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            for (int i = 0; i < 9; i++) {
                int result = bcd % 10;
                bcd /= 10;
                result |= (bcd % 10) << 4;
                bcd /= 10;
                do {
                    uint32_t addr_ = linaddr + i, shift_ = m_tlb_shift_write, data_ = result,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                            return 1;
                    } else
                        *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
                } while (0);
            }
            int result = bcd % 10;
            bcd /= 10;
            result |= (bcd % 10) << 4;
            do {
                uint32_t addr_ = linaddr + 9, shift_ = m_tlb_shift_write, data_ = result | (st0.exp >> 8 & 0x80),
                         tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else
                    *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            fpu_pop();
            break;
        }
        case (0xDF & 7) << 3 | 7: {
            if (fpu_fwait())
                return 1;
            fpu_update_pointers2(opcode, virtaddr, seg);
            if (fpu_check_stack_underflow(0, 0))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            uint64_t i64 = floatx80_to_int64(fpu_get_st(0), &m_fpu.status);
            if (fpu_check_exceptions2(0))
                do {
                    fpu_watchpoint2();
                    return 0;
                } while (0);
            do {
                uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = (uint32_t)i64,
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            do {
                uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = (uint32_t)(i64 >> 32),
                         tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                        return 1;
                } else
                    *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
            } while (0);
            fpu_commit_sw();
            fpu_pop();
            break;
        }
        case (0xD9 & 7) << 3 | 1:
        case (0xDB & 7) << 3 | 4:
        case (0xDB & 7) << 3 | 6:
        case (0xDD & 7) << 3 | 5: {
            int major_opcode = opcode >> 8 | 0xD8;
            (void)(major_opcode);
            break;
        }
        default: {
            int major_opcode = opcode >> 8 | 0xD8;
            do {
                util_abort();
                abort();
            } while (0);
        }
    }
    fpu_watchpoint2();
    return 0;
}
int CPU::fpu_fxsave(uint32_t linaddr)
{
    if (linaddr & 15)
        do {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        } while (0);
    if (fpu_nm_check())
        return 1;
    if (cpu_access_verify(linaddr, linaddr + 288 - 1, m_tlb_shift_write))
        return 1;
    do {
        uint32_t addr_ = linaddr + 0, shift_ = m_tlb_shift_write, data_ = m_fpu.control_word,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_write, data_ = fpu_get_status_word(),
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    uint8_t tag = 0;
    for (int i = 0; i < 8; i++)
        if ((m_fpu.tag_word >> (i * 2) & 3) != FPU_TAG_EMPTY)
            tag |= 1 << i;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = tag, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 6, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_opcode,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 8, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_eip, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 12, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_cs, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 16, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_data_ptr,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 20, shift_ = m_tlb_shift_write, data_ = m_fpu.fpu_data_seg,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 24, shift_ = m_tlb_shift_write, data_ = m_mxcsr, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 28, shift_ = m_tlb_shift_write, data_ = 0xFFFF, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    uint32_t tempaddr = linaddr + 32;
    for (int i = 0; i < 8; i++) {
        fpu_store_f80(tempaddr, fpu_get_st_ptr(i));
        do {
            uint32_t addr_ = tempaddr + 10, shift_ = m_tlb_shift_write, data_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 12, shift_ = m_tlb_shift_write, data_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        tempaddr += 16;
    }
    tempaddr = linaddr + 160;
    for (int i = 0; i < 8; i++) {
        do {
            uint32_t addr_ = tempaddr, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4)],
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 4, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4) + 1],
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 8, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4) + 2],
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 12, shift_ = m_tlb_shift_write, data_ = m_xmm32[(i * 4) + 3],
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        tempaddr += 16;
    }
    return 0;
}
int CPU::fpu_fxrstor(uint32_t linaddr)
{
    if (linaddr & 15)
        do {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        } while (0);
    if (fpu_nm_check())
        return 1;
    if (cpu_access_verify(linaddr, linaddr + 288 - 1, m_tlb_shift_read))
        return 1;
    uint32_t _mxcsr;
    do {
        uint32_t addr_ = linaddr + 24, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                _mxcsr = m_read_result;
            else
                return 1;
        } else
            _mxcsr = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (_mxcsr & ~0xFFFF)
        do {
            cpu_exception(13, (0) | 0x10000);
            return 1;
        } while (0);
    m_mxcsr = _mxcsr;
    cpu_update_mxcsr();
    uint32_t temp = 0;
    do {
        uint32_t addr_ = linaddr + 0, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                return 1;
        } else
            temp = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    fpu_set_control_word(temp);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                return 1;
        } else
            temp = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_fpu.status_word = temp;
    m_fpu.ftop        = m_fpu.status_word >> 11 & 7;
    m_fpu.status_word &= ~(7 << 11);
    uint8_t small_tag_word;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                small_tag_word = m_read_result;
            else
                return 1;
        } else
            small_tag_word = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 6, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_opcode = m_read_result;
            else
                return 1;
        } else
            m_fpu.fpu_opcode = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_fpu.fpu_opcode &= 0x7FF;
    do {
        uint32_t addr_ = linaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_eip = m_read_result;
            else
                return 1;
        } else
            m_fpu.fpu_eip = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_cs = m_read_result;
            else
                return 1;
        } else
            m_fpu.fpu_cs = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 16, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_data_ptr = m_read_result;
            else
                return 1;
        } else
            m_fpu.fpu_data_ptr = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 20, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_fpu.fpu_data_seg = m_read_result;
            else
                return 1;
        } else
            m_fpu.fpu_data_seg = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    uint32_t tempaddr = linaddr + 32;
    for (int i = 0; i < 8; i++) {
        if (fpu_read_f80(tempaddr, fpu_get_st_ptr(i)))
            return 1;
        tempaddr += 16;
    }
    tempaddr = linaddr + 160;
    for (int i = 0; i < 8; i++) {
        do {
            uint32_t addr_ = tempaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4)] = m_read_result;
                else
                    return 1;
            } else
                m_xmm32[(i * 4)] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4) + 1] = m_read_result;
                else
                    return 1;
            } else
                m_xmm32[(i * 4) + 1] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 8, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4) + 2] = m_read_result;
                else
                    return 1;
            } else
                m_xmm32[(i * 4) + 2] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = tempaddr + 12, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_xmm32[(i * 4) + 3] = m_read_result;
                else
                    return 1;
            } else
                m_xmm32[(i * 4) + 3] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        tempaddr += 16;
    }
    uint16_t tag_word = 0;
    for (int i = 0; i < 8; i++) {
        int tagid = FPU_TAG_EMPTY;
        if (small_tag_word & (1 << i))
            tagid = fpu_get_tag_from_value(&m_fpu.st[i]);
        tag_word |= tagid << (i * 2);
    }
    m_fpu.tag_word = tag_word;
    return 0;
}
int CPU::fpu_fwait(void)
{
    if (m_fpu.status_word & 0x80) {
        if (m_cr[0] & 32)
            do {
                cpu_exception(16, 0);
                return 1;
            } while (0);
        else {
            pic_lower_irq(13);
            pic_raise_irq(13);
        }
    }
    return 0;
}
void _fpu_state()
{
    cpu.fpu_state();
}
void CPU::fpu_init(void)
{
    state_register(_fpu_state);
}
double f80_to_double(floatx80 *f80)
{
    double   scale_factor = pow(2.0, -63.0);
    uint16_t exponent;
    uint64_t fraction;
    do {
        exponent = (f80)->exp;
        fraction = (f80)->fraction;
    } while (0);
    double f = pow(2.0, ((exponent & 0x7FFF) - 0x3FFF));
    if (exponent > 0x8000)
        f = -f;
    f *= fraction * scale_factor;
    return f;
}
void CPU::fpu_debug(void)
{
}
void CPU::printFloat80(floatx80 *arg)
{
    uint16_t exponent;
    uint64_t fraction;
    do {
        exponent = (arg)->exp;
        fraction = (arg)->fraction;
    } while (0);
    printf("%04x %08x%08x\n", exponent, (uint32_t)(fraction >> 32), (uint32_t)fraction);
}
void *CPU::fpu_get_st_ptr1(void)
{
    return &m_fpu.st[0];
}



//

void CPU::float_raise(float_status_t *status, int flags)
{
    status->float_exception_flags |= flags;
}
int CPU::get_exception_flags(const float_status_t *status)
{
    return status->float_exception_flags & ~status->float_suppress_exception;
}
int CPU::float_exception_masked(const float_status_t *status, int flag)
{
    return status->float_exception_masks & flag;
}
int CPU::get_float_rounding_mode(const float_status_t *status)
{
    return status->float_rounding_mode;
}
int CPU::get_float_rounding_precision(const float_status_t *status)
{
    return status->float_rounding_precision;
}
int CPU::get_float_nan_handling_mode(const float_status_t *status)
{
    return status->float_nan_handling_mode;
}
void CPU::set_float_rounding_up(float_status_t *status)
{
    status->float_exception_flags |= RAISE_SW_C1;
}
int CPU::get_denormals_are_zeros(const float_status_t *status)
{
    return status->denormals_are_zeros;
}
int CPU::get_flush_underflow_to_zero(const float_status_t *status)
{
    return status->flush_underflow_to_zero;
}
float32 CPU::float32_round_to_int(float32 a, float_status_t *status)
{
    return float32_round_to_int_with_scale(a, 0, status);
}
float32 CPU::float32_fmadd(float32 a, float32 b, float32 c, float_status_t *status)
{
    return float32_muladd(a, b, c, 0, status);
}
float32 CPU::float32_fmsub(float32 a, float32 b, float32 c, float_status_t *status)
{
    return float32_muladd(a, b, c, float_muladd_negate_c, status);
}
float32 CPU::float32_fnmadd(float32 a, float32 b, float32 c, float_status_t *status)
{
    return float32_muladd(a, b, c, float_muladd_negate_product, status);
}
float32 CPU::float32_fnmsub(float32 a, float32 b, float32 c, float_status_t *status)
{
    return float32_muladd(a, b, c, float_muladd_negate_result, status);
}
int CPU::float32_compare(float32 a, float32 b, float_status_t *status)
{
    return float32_compare_internal(a, b, 0, status);
}
int CPU::float32_compare_quiet(float32 a, float32 b, float_status_t *status)
{
    return float32_compare_internal(a, b, 1, status);
}
float64 CPU::float64_round_to_int(float64 a, float_status_t *status)
{
    return float64_round_to_int_with_scale(a, 0, status);
}
float64 CPU::float64_fmadd(float64 a, float64 b, float64 c, float_status_t *status)
{
    return float64_muladd(a, b, c, 0, status);
}
float64 CPU::float64_fmsub(float64 a, float64 b, float64 c, float_status_t *status)
{
    return float64_muladd(a, b, c, float_muladd_negate_c, status);
}
float64 CPU::float64_fnmadd(float64 a, float64 b, float64 c, float_status_t *status)
{
    return float64_muladd(a, b, c, float_muladd_negate_product, status);
}
float64 CPU::float64_fnmsub(float64 a, float64 b, float64 c, float_status_t *status)
{
    return float64_muladd(a, b, c, float_muladd_negate_result, status);
}
int CPU::float64_compare(float64 a, float64 b, float_status_t *status)
{
    return float64_compare_internal(a, b, 0, status);
}
int CPU::float64_compare_quiet(float64 a, float64 b, float_status_t *status)
{
    return float64_compare_internal(a, b, 1, status);
}
int CPU::floatx80_compare(floatx80 a, floatx80 b, float_status_t *status)
{
    return floatx80_compare_internal(a, b, 0, status);
}
int CPU::floatx80_compare_quiet(floatx80 a, floatx80 b, float_status_t *status)
{
    return floatx80_compare_internal(a, b, 1, status);
}
void CPU::floatx80_abs(floatx80 *reg)
{
    reg->exp &= 0x7FFF;
}
void CPU::floatx80_chs(floatx80 *reg)
{
    reg->exp ^= 0x8000;
}
