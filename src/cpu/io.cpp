#include "cpu.h"
#include "../io.h"
#include <cstdint>

extern CPU cpu;

int CPU::cpu_io_check_access(uint32_t port, int size)
{
    if ((m_cr[0] & 1) && ((m_eflags & 0x20000) || (unsigned int)m_cpl > (m_eflags >> 12 & 3))) {
        uint16_t        tss = m_seg[6];
        struct seg_desc tss_info;
        int             tss_access, tss_type;
        if (cpu_seg_load_descriptor(tss, &tss_info, 13, 0))
            return 1;
        tss_access = ((&tss_info)->raw[1] >> 8 & 0xFFFF);
        tss_type   = ((tss_access) & (0x10 | 0x08 | 0x04 | 0x02 | 0x01));
        if (tss_type != AVAILABLE_TSS_386 && tss_type != BUSY_TSS_386)
            do {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            } while (0);
        uint32_t base = m_seg_base[6], limit = m_seg_limit[6];
        if (limit < 0x67)
            do {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            } while (0);
        uint32_t io_offset;
        do {
            uint32_t addr_ = base + 0x66, shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    io_offset = m_read_result;
                else
                    return 1;
            } else
                io_offset = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        if (limit < (io_offset + ((port + size) >> 3)))
            do {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            } while (0);
        int      mask = ((size << 1) - 1) << (port & 7);
        uint16_t bitmask;
        do {
            uint32_t addr_ = base + io_offset + (port >> 3), shift_ = 0, tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 1)) {
                if (!cpu_access_read16(addr_, tag >> 0, 0))
                    bitmask = m_read_result;
                else
                    return 1;
            } else
                bitmask = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
        } while (0);
        int valid = (bitmask & mask) == 0;
        if (!valid)
            do {
                cpu_exception(13, (0) | 0x10000);
                return 1;
            } while (0);
    }
    return 0;
}
void CPU::cpu_outb(uint32_t port, uint32_t data)
{
    io_writeb(port, data);
}
void CPU::cpu_outw(uint32_t port, uint32_t data)
{
    io_writew(port, data);
}
void CPU::cpu_outd(uint32_t port, uint32_t data)
{
    io_writed(port, data);
}
uint32_t CPU::cpu_inb(uint32_t port)
{
    uint8_t result = io_readb(port);
    return result;
}
uint32_t CPU::cpu_inw(uint32_t port)
{
    uint16_t result = io_readw(port);
    return result;
}
uint32_t CPU::cpu_ind(uint32_t port)
{
    uint32_t result = io_readd(port);
    return result;
}
