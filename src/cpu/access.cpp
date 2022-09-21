#include "cpu.h"
#include "../io.h"

extern CPU cpu;


int CPU::cpu_access_read8(uint32_t addr, uint32_t tag, int shift)
{
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    if ((phys >= 0xA0000 && phys < 0xC0000) || (phys >= m_memory_size)) {
        m_read_result = io_handle_mmio_read(phys, 0);
        return 0;
    }
    m_read_result = *(uint8_t *)host_ptr;
    return 0;
}
int CPU::cpu_access_read16(uint32_t addr, uint32_t tag, int shift)
{
    if (addr & 1) {
        uint32_t res = 0;
        for (int i = 0, j = 0; i < 2; i++, j += 8) {
            if (cpu_access_read8(addr + i, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
            res |= m_read_result << j;
        }
        m_read_result = res;
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    if ((phys >= 0xA0000 && phys < 0xC0000) || (phys >= m_memory_size)) {
        m_read_result = io_handle_mmio_read(phys, 1);
        return 0;
    }
    m_read_result = *(uint16_t *)host_ptr;
    return 0;
}
int CPU::cpu_access_read32(uint32_t addr, uint32_t tag, int shift)
{
    if (addr & 3) {
        uint32_t res = 0;
        for (int i = 0, j = 0; i < 4; i++, j += 8) {
            if (cpu_access_read8(addr + i, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
            res |= m_read_result << j;
        }
        m_read_result = res;
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    if ((phys >= 0xA0000 && phys < 0xC0000) || (phys >= m_memory_size)) {
        m_read_result = io_handle_mmio_read(phys, 2);
        return 0;
    }
    m_read_result = *(uint32_t *)host_ptr;
    return 0;
}
int CPU::cpu_access_write8(uint32_t addr, uint32_t data, uint32_t tag, int shift)
{
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    if ((phys >= 0xA0000 && phys < 0x100000) || (phys >= m_memory_size)) {
        io_handle_mmio_write(phys, data, 0);
        return 0;
    }
    if (cpu_smc_has_code(phys))
        cpu_smc_invalidate(addr, phys);
    *(uint8_t *)host_ptr = data;
    return 0;
}
int CPU::cpu_access_write16(uint32_t addr, uint32_t data, uint32_t tag, int shift)
{
    if (addr & 1) {
        for (int i = 0, j = 0; i < 2; i++, j += 8) {
            if (cpu_access_write8(addr + i, data >> j, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
        }
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    if ((phys >= 0xA0000 && phys < 0x100000) || (phys >= m_memory_size)) {
        io_handle_mmio_write(phys, data, 1);
        return 0;
    }
    if (cpu_smc_has_code(phys))
        cpu_smc_invalidate(addr, phys);
    *(uint16_t *)host_ptr = data;
    return 0;
}
int CPU::cpu_access_write32(uint32_t addr, uint32_t data, uint32_t tag, int shift)
{
    if (addr & 3) {
        for (int i = 0, j = 0; i < 4; i++, j += 8) {
            if (cpu_access_write8(addr + i, data >> j, m_tlb_tags[(addr + i) >> 12] >> shift, shift))
                return 1;
        }
        return 0;
    }
    if (tag & 2) {
        if (cpu_mmu_translate(addr, shift))
            return 1;
        tag = m_tlb_tags[addr >> 12] >> shift;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    if ((phys >= 0xA0000 && phys < 0x100000) || (phys >= m_memory_size)) {
        io_handle_mmio_write(phys, data, 2);
        return 0;
    }
    if (cpu_smc_has_code(phys))
        cpu_smc_invalidate(addr, phys);
    *(uint32_t *)host_ptr = data;
    return 0;
}
int CPU::cpu_access_verify(uint32_t addr, uint32_t end, int shift)
{
    uint32_t tag;
    if ((addr ^ end) & ~0xFFF) {
        tag = m_tlb_tags[addr >> 12];
        if (tag & 2) {
            if (cpu_mmu_translate(addr, shift))
                return 1;
        }
    } else
        end = addr;
    tag = m_tlb_tags[end >> 12];
    if (tag & 2) {
        if (cpu_mmu_translate(end, shift))
            return 1;
    }
    return 0;
}
uint8_t CPU::read8(uint32_t lin)
{
    uint8_t dest;
    do {
        uint32_t addr_ = lin, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    printf("Unable to read memory at address %08x\n", lin);
                    return 0;
                } while (0);
        } else
            dest = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    return dest;
}
uint16_t CPU::read16(uint32_t lin)
{
    uint16_t dest;
    do {
        uint32_t addr_ = lin, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    printf("Unable to read memory at address %08x\n", lin);
                    return 0;
                } while (0);
        } else
            dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    return dest;
}
uint32_t CPU::read32(uint32_t lin)
{
    uint32_t dest;
    do {
        uint32_t addr_ = lin, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    printf("Unable to read memory at address %08x\n", lin);
                    return 0;
                } while (0);
        } else
            dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    return dest;
}
void CPU::readmem(uint32_t lin, int bytes)
{
    for (int i = 0; i < bytes; i++) {
        printf("%02x ", read8(lin + i));
    }
    printf("\n");
}
void CPU::readphys(uint32_t lin, int bytes)
{
    for (int i = 0; i < bytes; i++) {
        printf("%02x ", m_mem8[lin + i]);
    }
    printf("\n");
}
uint32_t CPU::lin2phys(uint32_t addr)
{
    uint8_t tag = m_tlb_tags[addr >> 12];
    if (tag & 2) {
        if (cpu_mmu_translate(addr, 0)) {
            printf("ERROR TRANSLATING ADDRESS %08x\n", addr);
            return 1;
        }
        tag = m_tlb_tags[addr >> 12] >> 0;
    }
    void    *host_ptr = (uint8_t *)m_tlb[addr >> 12] + addr;
    uint32_t phys     = (uint32_t)(uintptr_t)((uint8_t *)host_ptr - (uint8_t *)m_mem);
    return phys;
}
