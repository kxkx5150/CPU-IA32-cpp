#include "cpu.h"

extern CPU cpu;


int CPU::cpu_smc_page_has_code(uint32_t phys)
{
    phys >>= 12;
    if (phys >= m_smc_has_code_length)
        return 0;
    return m_smc_has_code[phys];
}
int CPU::cpu_smc_has_code(uint32_t phys)
{
    phys >>= 7;
    if ((phys >> 5) >= m_smc_has_code_length)
        return 0;
    return m_smc_has_code[phys >> 5] & (1 << (phys & 31));
}
void CPU::cpu_smc_set_code(uint32_t phys)
{
    phys >>= 7;
    if ((phys >> 5) >= m_smc_has_code_length)
        return;
    m_smc_has_code[phys >> 5] |= 1 << (phys & 31);
}
void CPU::cpu_smc_invalidate(uint32_t lin, uint32_t phys)
{
    uint32_t pageid = phys >> 12, page_info, p128, invmask, pagebase = phys & ~0xFFF;
    int      start, end, quit = 0;
    if (pageid >= m_smc_has_code_length)
        return;
    page_info = m_smc_has_code[pageid];
    p128      = phys >> 7 & 31;
    start     = 0;
    end       = p128;
    {
        int endmask = 1 << end, startmask = 1 << start;
        if (start == 0)
            startmask = 0;
        endmask = endmask | (endmask - 1);
        if (startmask != 0)
            startmask = startmask | (startmask - 1);
        invmask = endmask ^ startmask;
        if (!(page_info & invmask))
            return;
    }
    for (int i = start; i <= end; i++) {
        uint32_t mask = 1 << i;
        if (page_info & mask) {
            uint32_t           physbase = pagebase + (i << 7);
            struct trace_info *info;
            for (int j = 0; j < 128; j++) {
                if ((info = cpu_trace_get_entry(physbase + j))) {
                    if (!quit && phys >= info->phys && phys <= (info->phys + (info->flags & 0x3FF)))
                        quit = 1;
                    info->phys = -1;
                }
            }
        }
    }
    page_info &= ~invmask;
    m_smc_has_code[pageid] = page_info;
    if (!page_info)
        cpu_mmu_tlb_invalidate(lin);
    if (quit)
        do {
            m_cycles += cpu_get_cycles() - m_cycles;
            m_refill_counter = m_cycles_to_run - 1;
            m_cycles_to_run  = 1;
            m_cycle_offset   = 1;
        } while (0);
}
void CPU::cpu_smc_invalidate_page(uint32_t phys)
{
    uint32_t pageid = phys >> 12, page_info = m_smc_has_code[pageid], pagebase = phys & ~0xFFF, quit = 1;
    for (int i = 0; i < 31; i++) {
        uint32_t mask = 1 << i;
        if (page_info & mask) {
            uint32_t           physbase = pagebase + (i << 7);
            struct trace_info *info;
            for (int j = 0; j < 128; j++) {
                if ((info = cpu_trace_get_entry(physbase + j))) {
                    if (!quit && phys >= info->phys && phys <= (info->phys + (info->flags & 0x3FF)))
                        quit = 1;
                    info->phys = -1;
                }
            }
        }
    }
    m_smc_has_code[pageid] = page_info;
    if (quit)
        do {
            m_cycles += cpu_get_cycles() - m_cycles;
            m_refill_counter = m_cycles_to_run - 1;
            m_cycles_to_run  = 1;
            m_cycle_offset   = 1;
        } while (0);
}
