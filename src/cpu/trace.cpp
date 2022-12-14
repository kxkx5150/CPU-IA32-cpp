#include "cpu.h"
#include <cstdint>
#include <string.h>

extern CPU cpu;


uint32_t CPU::hash_eip(uint32_t phys)
{
    return phys & ((64 * 1024) - 1);
}
void CPU::cpu_trace_flush(void)
{
    memset(trace_info, 0, sizeof(struct trace_info) * (64 * 1024));
    m_trace_cache_usage = 0;
}
struct CPU::trace_info *CPU::cpu_trace_get_entry(uint32_t phys)
{
    struct trace_info *i = &trace_info[hash_eip(phys)];
    if (i->phys != phys)
        return (struct trace_info *)((void *)0);
    return i;
}
optype CPU::cpu_get_trace(void)
{
    if ((m_phys_eip ^ m_last_phys_eip) > 4095) {
        uint32_t virt_eip = (m_phys_eip + m_eip_phys_bias), lin_eip = virt_eip + m_seg_base[1];
        uint8_t  tlb_tag = m_tlb_tags[lin_eip >> 12];
        if ((tlb_tag >> m_tlb_shift_read & 1) || m_tlb_attrs[lin_eip >> 12] & 1) {
            if (cpu_mmu_translate(lin_eip, m_tlb_shift_read | 8))
                return &temporary_placeholder;
        }
        m_phys_eip      = (uint32_t)(uintptr_t)((uint8_t *)m_tlb[lin_eip >> 12] + lin_eip - (uint8_t *)m_mem);
        m_eip_phys_bias = virt_eip - m_phys_eip;
        m_last_phys_eip = m_phys_eip & ~0xFFF;
    }
    struct trace_info *trace = &trace_info[hash_eip(m_phys_eip)];
    if (trace->phys == m_phys_eip && trace->state_hash == m_state_hash) {
        if (trace->ptr == ((void *)0)) {
            do {
                util_abort();
                abort();
            } while (0);
        }
        return trace->ptr;
    }
    if ((m_trace_cache_usage + 32) >= ((64 * 1024) * 8)) {
        cpu_trace_flush();
    }
    struct decoded_instruction *i = &trace_cache[m_trace_cache_usage];
    m_trace_cache_usage += cpu_decode(trace, i);
    if (i == ((void *)0)) {
        do {

            util_abort();
            abort();
        } while (0);
    }
    return i;
}
