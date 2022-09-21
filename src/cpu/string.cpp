#include "cpu.h"

extern CPU cpu;


int CPU::movsb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::movsb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::movsw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::movsw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::movsd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = ds_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[12] += add;
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::movsd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src, ds_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = ds_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[6] += add;
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::stosb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src = m_reg8[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::stosb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src = m_reg8[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::stosw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src = m_reg16[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::stosw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src = m_reg16[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::stosd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src = m_reg32[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::stosd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src = m_reg32[0];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::scasb16(int flags)
{
    int     count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1;
    uint8_t dest = m_reg8[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::scasb32(int flags)
{
    int     count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1;
    uint8_t dest = m_reg8[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::scasw16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2;
    uint16_t dest = m_reg16[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::scasw32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2;
    uint16_t dest = m_reg16[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::scasd16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4;
    uint32_t dest = m_reg32[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::scasd32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4;
    uint32_t dest = m_reg32[0], src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::insb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inb(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inb(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::insb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inb(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inb(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::insw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inw(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inw(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::insw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_inw(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_inw(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::insd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_ind(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_ind(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg16[14] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::insd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        src = cpu_ind(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        src = cpu_ind(m_reg16[4]);
        do {
            uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_write, data_ = src,
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        m_reg32[7] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::outsb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::outsb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 8 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outb(m_reg16[4], src);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::outsw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::outsw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 16 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outw(m_reg16[4], src);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::outsd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::outsd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, src, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (cpu_io_check_access(m_reg16[4], 32 >> 3))
        return -1;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    src = m_read_result;
                else
                    return 1;
            } else
                src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        cpu_outd(m_reg16[4], src);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::cmpsb16(int flags)
{
    int     count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    uint8_t dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_reg16[12] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::cmpsb32(int flags)
{
    int     count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    uint8_t dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if ((tag >> shift_ & 1)) {
                    if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_reg32[6] += add;
            m_lr   = (int8_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB8;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if ((tag >> shift_ & 1)) {
                        if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int8_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB8;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::cmpsw16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    uint16_t dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_reg16[12] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::cmpsw32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    uint16_t dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_reg32[6] += add;
            m_lr   = (int16_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB16;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 1)) {
                        if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int16_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB16;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::cmpsd16(int flags)
{
    int      count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    uint32_t dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg16[14] += add;
            m_reg16[12] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg16[2] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg16[14], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg16[14] += add;
                m_reg16[12] += add;
                m_reg16[2]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg16[2] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::cmpsd32(int flags)
{
    int      count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    uint32_t dest, src;
    if ((unsigned int)count > 65536)
        count = 65536;
    switch (flags >> 6 & 3) {
        case 0:
            do {
                uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        dest = m_read_result;
                    else
                        return 1;
                } else
                    dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            do {
                uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        src = m_read_result;
                    else
                        return 1;
                } else
                    src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            m_reg32[7] += add;
            m_reg32[6] += add;
            m_lr   = (int32_t)(dest - src);
            m_lop2 = src;
            m_laux = SUB32;
            return 0;
        case 1:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src != dest)
                    return 0;
            }
            return m_reg32[1] != 0;
        case 2:
            for (int i = 0; i < count; i++) {
                do {
                    uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            dest = m_read_result;
                        else
                            return 1;
                    } else
                        dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                do {
                    uint32_t addr_ = m_seg_base[0] + m_reg32[7], shift_ = m_tlb_shift_read,
                             tag = m_tlb_tags[addr_ >> 12];
                    if (((addr_ | tag >> shift_) & 3)) {
                        if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                            src = m_read_result;
                        else
                            return 1;
                    } else
                        src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                } while (0);
                m_reg32[7] += add;
                m_reg32[6] += add;
                m_reg32[1]--;
                m_lr   = (int32_t)(dest - src);
                m_lop2 = src;
                m_laux = SUB32;
                if (src == dest)
                    return 0;
            }
            return m_reg32[1] != 0;
    }
    do {
        util_abort();
        abort();
    } while (0);
}
int CPU::lodsb16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::lodsb32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -1 : 1, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if ((tag >> shift_ & 1)) {
                if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg8[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg8[0] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::lodsw16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::lodsw32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -2 : 2, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg16[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg16[0] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
int CPU::lodsd16(int flags)
{
    int count = m_reg16[2], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg16[12], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg16[12] += add;
        m_reg16[2]--;
    }
    return m_reg16[2] != 0;
}
int CPU::lodsd32(int flags)
{
    int count = m_reg32[1], add = m_eflags & 0x400 ? -4 : 4, seg_base = m_seg_base[flags >> 22 & 7];
    if ((unsigned int)count > 65536)
        count = 65536;
    if (!(flags & ((1 << 6) | (2 << 6)))) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        return 0;
    }
    for (int i = 0; i < count; i++) {
        do {
            uint32_t addr_ = seg_base + m_reg32[6], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    m_reg32[0] = m_read_result;
                else
                    return 1;
            } else
                m_reg32[0] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        m_reg32[6] += add;
        m_reg32[1]--;
    }
    return m_reg32[1] != 0;
}
