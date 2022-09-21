#include "cpu.h"
#include <cstdint>

extern CPU cpu;



void CPU::reload_cs_base(void)
{
    uint32_t virt_eip = (m_phys_eip + m_eip_phys_bias), lin_eip = virt_eip + m_seg_base[1];
    uint32_t lin_page = lin_eip >> 12, shift = m_tlb_shift_read, tag = m_tlb_tags[lin_eip >> 12] >> shift;
    if (tag & 2) {
        m_last_phys_eip = m_phys_eip + 0x1000;
        return;
    }
    m_phys_eip      = (uint32_t)(uintptr_t)((uint8_t *)m_tlb[lin_page] + lin_eip - (uint8_t *)m_mem);
    m_last_phys_eip = m_phys_eip & ~0xFFF;
    m_eip_phys_bias = virt_eip - m_phys_eip;
}
void CPU::cpu_load_csip_real(uint16_t cs, uint32_t eip)
{
    m_phys_eip += (eip) - (m_phys_eip + m_eip_phys_bias);
    ;
    cpu_seg_load_real(1, cs);
    reload_cs_base();
}
void CPU::cpu_load_csip_virtual(uint16_t cs, uint32_t eip)
{
    m_phys_eip += (eip) - (m_phys_eip + m_eip_phys_bias);
    ;
    cpu_seg_load_virtual(1, cs);
    reload_cs_base();
}
int CPU::cpu_load_csip_protected(uint16_t cs, struct seg_desc *info, uint32_t eip)
{
    m_phys_eip += (eip) - (m_phys_eip + m_eip_phys_bias);
    ;
    if (cpu_seg_load_protected(1, cs, info))
        return 1;
    reload_cs_base();
    return 0;
}
void CPU::cpu_seg_load_virtual(int id, uint16_t sel)
{
    m_seg[id]       = sel;
    m_seg_base[id]  = sel << 4;
    m_seg_limit[id] = 0xFFFF;
    m_seg_access[id] &= ~(0x60 | 0x4000);
    switch (id) {
        case 1:
            m_state_hash = 0x0002 | 0x0001;
            break;
        case 2:
            m_esp_mask = 0xFFFF;
            break;
    }
}
void CPU::cpu_seg_load_real(int id, uint16_t sel)
{
    m_seg[id]       = sel;
    m_seg_base[id]  = sel << 4;
    m_seg_limit[id] = 0xFFFF;
    m_seg_access[id] &= ~(0x60 | 0x4000);
    switch (id) {
        case 1:
            m_state_hash = 0x0002 | 0x0001;
            break;
        case 2:
            m_esp_mask = 0xFFFF;
            break;
    }
}
int CPU::cpu_seg_load_protected(int id, uint16_t sel, struct seg_desc *info)
{
    m_seg[id]        = sel;
    m_seg_base[id]   = cpu_seg_get_base(info);
    m_seg_limit[id]  = cpu_seg_get_limit(info);
    m_seg_access[id] = ((info)->raw[1] >> 8 & 0xFFFF);
    uint32_t linaddr = cpu_seg_descriptor_address(-1, sel);
    if (linaddr == (uint32_t)-1)
        do {
            util_abort();
            abort();
        } while (0);
    info->raw[1] |= 0x100;
    do {
        uint32_t addr_ = linaddr + 5, shift_ = 2, data_ = info->raw[1] >> 8 & 0xFF, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> 2, 2))
                return 1;
        } else
            *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    switch (id) {
        case 1:
            if (m_seg_access[1] & 0x4000)
                m_state_hash = 0;
            else
                m_state_hash = 0x0002 | 0x0001;
            m_cpl = sel & 3;
            cpu_prot_update_cpl();
            break;
        case 2:
            if (m_seg_access[2] & 0x4000)
                m_esp_mask = -1;
            else
                m_esp_mask = 0xFFFF;
            break;
    }
    return 0;
}
int CPU::cpu_seg_load_descriptor2(int table, uint32_t selector, struct seg_desc *seg, int exception, int code)
{
    if ((selector | 7) > m_seg_limit[table]) {
        if (exception == -1)
            return -1;
        do {
            cpu_exception(exception, (code) | 0x10000);
            return 1;
        } while (0);
    }
    int addr = (selector & ~7) + m_seg_base[table];
    do {
        uint32_t addr_ = addr, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> 0, 0))
                seg->raw[0] = m_read_result;
            else
                return 1;
        } else
            seg->raw[0] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = addr + 4, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> 0, 0))
                seg->raw[1] = m_read_result;
            else
                return 1;
        } else
            seg->raw[1] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    return 0;
}
int CPU::cpu_seg_load_descriptor(uint32_t selector, struct seg_desc *seg, int exception, int code)
{
    if (cpu_seg_load_descriptor2(((selector & 4) != 0) ? 8 : 7, selector, seg, exception, code))
        return 1;
    return 0;
}
int CPU::cpu_seg_get_dpl(int seg)
{
    return (m_seg_access[seg] >> 5 & 3);
}
uint32_t CPU::cpu_seg_get_base(struct seg_desc *info)
{
    uint32_t base = info->raw[0] >> 16;
    base |= (info->raw[1] << 16 & 0xFF0000) | (info->raw[1] & 0xFF000000);
    return base;
}
uint32_t CPU::cpu_seg_get_limit(struct seg_desc *info)
{
    uint32_t limit = info->raw[0] & 0xFFFF;
    limit |= (info->raw[1] & 0xF0000);
    if (((info)->raw[1] >> 8 & 0xFFFF) & 0x8000) {
        limit <<= 12;
        limit |= 0xFFF;
    }
    return limit;
}
uint32_t CPU::cpu_seg_gate_target_segment(struct seg_desc *info)
{
    return info->raw[0] >> 16 & 0xFFFF;
}
uint32_t CPU::cpu_seg_gate_target_offset(struct seg_desc *info)
{
    uint32_t offset = info->raw[0] & 0xFFFF;
    int      access = ((info)->raw[1] >> 8 & 0xFFFF);
    switch (((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01))) {
        case CALL_GATE_386:
        case INTERRUPT_GATE_386:
        case TRAP_GATE_386:
            return offset | (info->raw[1] & ~0xFFFF);
        default:
            return offset;
    }
}
uint32_t CPU::cpu_seg_gate_parameter_count(struct seg_desc *info)
{
    return info->raw[1] & 0x1F;
}
uint32_t CPU::cpu_seg_descriptor_address(int tbl, uint16_t sel)
{
    if (tbl == -1) {
        if (((sel & 4) != 0))
            tbl = 8;
        else
            tbl = 7;
    }
    if ((sel | 7) > m_seg_limit[tbl])
        return (uint32_t)-1;
    return (sel & ~7) + m_seg_base[tbl];
}
int CPU::cpu_load_seg_value_mov(int seg, uint16_t val)
{
    if ((m_cr[0] & 1) == 0 || (m_eflags & 0x20000)) {
        if (!(m_cr[0] & 1))
            cpu_seg_load_real(seg, val);
        else
            cpu_seg_load_virtual(seg, val);
    } else {
        struct seg_desc info;
        info.raw[0]         = 0;
        info.raw[1]         = 0;
        uint16_t val_offset = val & 0xFFFC;
        int      rpl, dpl, type, access;
        switch (seg) {
            case 1:
                return cpu_seg_load_protected(seg, val, &info);
            case 2:
                if (!val_offset)
                    do {
                        cpu_exception(13, (0) | 0x10000);
                        return 1;
                    } while (0);
                if (cpu_seg_load_descriptor(val, &info, 13, val_offset))
                    return 1;
                access = ((&info)->raw[1] >> 8 & 0xFFFF);
                rpl    = (val & 3);
                dpl    = (access >> 5 & 3);
                if (m_cpl != rpl || m_cpl != dpl)
                    do {
                        cpu_exception(13, (val_offset) | 0x10000);
                        return 1;
                    } while (0);
                type = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                if (type == 0x12 || type == 0x13 || type == 0x16 || type == 0x17) {
                    if (!(access & 0x80))
                        do {
                            cpu_exception(13, (val_offset) | 0x10000);
                            return 1;
                        } while (0);
                    return cpu_seg_load_protected(seg, val, &info);
                } else
                    do {
                        cpu_exception(13, (val_offset) | 0x10000);
                        return 1;
                    } while (0);
                break;
            case 0:
            case 4:
            case 5:
            case 3:
                if (val_offset) {
                    if (cpu_seg_load_descriptor(val, &info, 13, val_offset))
                        return 1;
                    access = ((&info)->raw[1] >> 8 & 0xFFFF);
                    type   = ((access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
                    switch (type) {
                        default:
                            do {
                                cpu_exception(13, (val_offset) | 0x10000);
                                return 1;
                            } while (0);
                            break;
                        case 0x1A:
                        case 0x1B:
                        case 0x1E:
                        case 0x1F:
                            break;
                        case 0x10 ... 0x17:
                        case 0x18 ... 0x19:
                            dpl = (access >> 5 & 3);
                            rpl = (val & 3);
                            if (dpl < m_cpl || dpl < rpl)
                                do {
                                    cpu_exception(13, (val_offset) | 0x10000);
                                    return 1;
                                } while (0);
                            break;
                    }
                    if (!(access & 0x80))
                        do {
                            cpu_exception(11, (val_offset) | 0x10000);
                            return 1;
                        } while (0);
                    return cpu_seg_load_protected(seg, val, &info);
                } else {
                    m_seg[seg]        = val;
                    m_seg_base[seg]   = 0;
                    m_seg_limit[seg]  = 0;
                    m_seg_access[seg] = 0;
                }
                break;
        }
    }
    return 0;
}
