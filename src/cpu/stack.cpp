#include "cpu.h"

extern CPU cpu;



int CPU::cpu_push16(uint32_t data)
{
    uint32_t esp = m_reg32[4], esp_mask = m_esp_mask, esp_minus_two = (esp - 2) & esp_mask;
    do {
        uint32_t addr_ = esp_minus_two + m_seg_base[2], shift_ = m_tlb_shift_write, data_ = data,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    m_reg32[4] = esp_minus_two | (esp & ~esp_mask);
    return 0;
}
int CPU::cpu_push32(uint32_t data)
{
    uint32_t esp = m_reg32[4], esp_mask = m_esp_mask, esp_minus_four = (esp - 4) & esp_mask;
    do {
        uint32_t addr_ = esp_minus_four + m_seg_base[2], shift_ = m_tlb_shift_write, data_ = data,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                return 1;
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    m_reg32[4] = esp_minus_four | (esp & ~esp_mask);
    return 0;
}
int CPU::cpu_pop16(uint16_t *dest)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4];
    do {
        uint32_t addr_ = (esp & esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                *dest = m_read_result;
            else
                return 1;
        } else
            *dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((esp + 2) & esp_mask) | (esp & ~esp_mask);
    return 0;
}
int CPU::cpu_pop16_dest32(uint32_t *dest)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4];
    do {
        uint32_t addr_ = (esp & esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                *dest = m_read_result;
            else
                return 1;
        } else
            *dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((esp + 2) & esp_mask) | (esp & ~esp_mask);
    return 0;
}
int CPU::cpu_pop32(uint32_t *dest)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4];
    do {
        uint32_t addr_ = (esp & esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                *dest = m_read_result;
            else
                return 1;
        } else
            *dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((esp + 4) & esp_mask) | (esp & ~esp_mask);
    return 0;
}
int CPU::cpu_pusha(void)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4];
    for (int i = 0; i < 16; i += 2) {
        esp = (esp - 2) & esp_mask;
        do {
            uint32_t addr_ = esp + m_seg_base[2], shift_ = m_tlb_shift_write, data_ = m_reg16[i],
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
    }
    m_reg32[4] = esp | (m_reg32[4] & ~esp_mask);
    return 0;
}
int CPU::cpu_pushad(void)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4];
    for (int i = 0; i < 8; i++) {
        esp = (esp - 4) & esp_mask;
        do {
            uint32_t addr_ = esp + m_seg_base[2], shift_ = m_tlb_shift_write, data_ = m_reg32[i],
                     tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    return 1;
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
    }
    m_reg32[4] = esp | (m_reg32[4] & ~esp_mask);
    return 0;
}
int CPU::cpu_popa(void)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4] & esp_mask;
    uint16_t temp16[8];
    for (int i = 7; i >= 0; i--) {
        do {
            uint32_t addr_ = esp + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp16[i] = m_read_result;
                else
                    return 1;
            } else
                temp16[i] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        esp = (esp + 2) & esp_mask;
    }
    for (int i = 0; i < 8; i++) {
        if (i == 4)
            continue;
        m_reg16[i << 1] = temp16[i];
    }
    m_reg32[4] = esp | (m_reg32[4] & ~esp_mask);
    return 0;
}
int CPU::cpu_popad(void)
{
    uint32_t esp_mask = m_esp_mask, esp = m_reg32[4] & esp_mask;
    uint32_t temp32[8];
    for (int i = 7; i >= 0; i--) {
        do {
            uint32_t addr_ = esp + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                    temp32[i] = m_read_result;
                else
                    return 1;
            } else
                temp32[i] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        esp = (esp + 4) & esp_mask;
    }
    for (int i = 0; i < 8; i++) {
        if (i == 4)
            continue;
        m_reg32[i] = temp32[i];
    }
    m_reg32[4] = esp | (m_reg32[4] & ~esp_mask);
    return 0;
}
