#include "cpu.h"
#include "../devices.h"
#include <cstdint>
#include <string.h>

extern CPU cpu;



CPU::CPU()
{
    create_ftable();
    temporary_placeholder.handler = &CPU::op_trace_end;
}
void CPU::create_ftable()
{
}
void CPU::cpu_set_a20(int a20_enabled)
{
    uint32_t old_a20_mask = m_a20_mask;
    m_a20_mask            = -1 ^ (!a20_enabled << 20);
    if (old_a20_mask != m_a20_mask)
        cpu_mmu_tlb_flush();
}
int CPU::cpu_init_mem(int size)
{
    m_mem = calloc(1, size);
    memset((uint8_t *)m_mem + 0xC0000, -1, 0x40000);
    m_memory_size         = size;
    m_smc_has_code_length = (size + 4095) >> 12;
    m_smc_has_code        = (uint32_t *)calloc(4, m_smc_has_code_length);
    return 0;
}
int CPU::cpu_interrupts_masked(void)
{
    return m_eflags & 0x200;
}
itick_t CPU::cpu_get_cycles(void)
{
    return m_cycles + (m_cycle_offset - m_cycles_to_run);
}
int CPU::cpu_run(int cycles)
{
    m_cycle_offset   = cycles;
    m_cycles_to_run  = cycles;
    m_refill_counter = 0;
    m_hlt_counter    = 0;
    uint64_t begin   = cpu_get_cycles();
    while (1) {
        if (m_intr_line_state) {
            if (m_eflags & 0x200 && !m_interrupts_blocked) {
                int interrupt_id = pic_get_interrupt();
                cpu_interrupt(interrupt_id, 0, INTERRUPT_TYPE_HARDWARE, (m_phys_eip + m_eip_phys_bias));
                m_exit_reason = 0;
            }
        }
        if (m_exit_reason == 3)
            return 0;
        if (m_interrupts_blocked) {
            m_refill_counter = cycles;
            m_cycles += cpu_get_cycles() - m_cycles;
            m_cycles_to_run      = 1;
            m_cycle_offset       = 1;
            m_interrupts_blocked = 0;
        }
        cpu_execute();
        m_cycles += cpu_get_cycles() - m_cycles;
        m_cycles_to_run  = m_refill_counter;
        m_refill_counter = 0;
        m_cycle_offset   = m_cycles_to_run;
        if (!m_cycles_to_run)
            break;
    }
    int cycles_run = cpu_get_cycles() - begin;
    m_cycle_offset = 0;
    return cycles_run;
}
void CPU::cpu_raise_intr_line(void)
{
    m_intr_line_state = 1;
}
void CPU::cpu_lower_intr_line(void)
{
    m_intr_line_state = 0;
}
void CPU::cpu_request_fast_return(int reason)
{
    (void)(reason);
    do {
        m_cycles += cpu_get_cycles() - m_cycles;
        m_refill_counter = m_cycles_to_run - 1;
        m_cycles_to_run  = 1;
        m_cycle_offset   = 1;
    } while (0);
}
void CPU::cpu_cancel_execution_cycle(int reason)
{
    m_exit_reason = reason;
    m_cycles += cpu_get_cycles() - m_cycles;
    m_cycles_to_run  = 1;
    m_cycle_offset   = 1;
    m_refill_counter = 0;
}
void *CPU::cpu_get_ram_ptr(void)
{
    return m_mem;
}
int CPU::cpu_add_rom(int addr, int size, void *data)
{
    if ((uint32_t)addr > m_memory_size || (uint32_t)(addr + size) > m_memory_size)
        return 0;
    memcpy((uint8_t *)m_mem + addr, data, size);
    return 0;
}
int CPU::cpu_get_exit_reason(void)
{
    return m_exit_reason;
}
void CPU::cpu_set_break(void)
{
}
void CPU::cpu_reset(void)
{
    for (int i = 0; i < 8; i++) {
        m_reg32[i] = 0;
        if (i == 0)
            m_cr[0] = 0x60000010;
        else
            m_cr[i] = 0;
        if (i == 1)
            cpu_seg_load_real(1, 0xF000);
        else
            cpu_seg_load_real(i, 0);
        if (i >= 6)
            m_dr[i] = (i == 6) ? 0xFFFF0FF0 : 0x400;
        else
            m_dr[i] = 0;
    }
    m_phys_eip += (0xFFF0) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_cpl = 0;
    cpu_prot_update_cpl();
    m_eflags                = 2;
    m_page_attribute_tables = 0x0007040600070406LL;
    if (apic_is_enabled())
        m_apic_base = 0xFEE00900;
    else
        m_apic_base = 0;
    m_mxcsr = 0x1F80;
    cpu_update_mxcsr();
    memset(m_tlb, 0, sizeof(uint8_t *) * (1 << 20));
    memset(m_tlb_tags, 0xFF, 1 << 20);
    memset(m_tlb_attrs, 0xFF, 1 << 20);
    cpu_mmu_tlb_flush();
}
int CPU::cpu_apic_connected(void)
{
    return apic_is_enabled() && (m_apic_base & 0x100);
}
void CPU::cpu_state(void)
{
    struct bjson_object *obj = state_obj((char *)"cpu", 44);
    state_field(obj, 64, (char *)"m_reg32", &m_reg32);
    state_field(obj, 128, (char *)"m_xmm32", &m_xmm32);
    state_field(obj, 4, (char *)"m_mxcsr", &m_mxcsr);
    state_field(obj, 4, (char *)"m_esp_mask", &m_esp_mask);
    state_field(obj, 4, (char *)"m_memory_size", &m_memory_size);
    state_field(obj, 4, (char *)"m_eflags", &m_eflags);
    state_field(obj, 4, (char *)"m_laux", &m_laux);
    state_field(obj, 4, (char *)"m_lop1", &m_lop1);
    state_field(obj, 4, (char *)"m_lop2", &m_lop2);
    state_field(obj, 4, (char *)"m_lr", &m_lr);
    state_field(obj, 4, (char *)"m_phys_eip", &m_phys_eip);
    state_field(obj, 4, (char *)"m_last_phys_eip", &m_last_phys_eip);
    state_field(obj, 4, (char *)"m_eip_phys_bias", &m_eip_phys_bias);
    state_field(obj, 4, (char *)"m_state_hash", &m_state_hash);
    state_field(obj, 8, (char *)"m_cycles", &m_cycles);
    state_field(obj, 8, (char *)"m_cycle_frame_end", &m_cycle_frame_end);
    state_field(obj, 4, (char *)"m_cycles_to_run", &m_cycles_to_run);
    state_field(obj, 4, (char *)"m_refill_counter", &m_refill_counter);
    state_field(obj, 4, (char *)"m_hlt_counter", &m_hlt_counter);
    state_field(obj, 4, (char *)"m_cycle_offset", &m_cycle_offset);
    state_field(obj, 32, (char *)"m_cr", &m_cr);
    state_field(obj, 32, (char *)"m_dr", &m_dr);
    state_field(obj, 4, (char *)"m_cpl", &m_cpl);
    state_field(obj, 32, (char *)"m_seg", &m_seg);
    state_field(obj, 64, (char *)"m_seg_base", &m_seg_base);
    state_field(obj, 64, (char *)"m_seg_limit", &m_seg_limit);
    state_field(obj, 64, (char *)"m_seg_access", &m_seg_access);
    state_field(obj, 64, (char *)"m_seg_valid", &m_seg_valid);
    state_field(obj, 4, (char *)"m_trace_cache_usage", &m_trace_cache_usage);
    state_field(obj, 4, (char *)"m_tlb_shift_read", &m_tlb_shift_read);
    state_field(obj, 4, (char *)"m_tlb_shift_write", &m_tlb_shift_write);
    state_field(obj, 256, (char *)"m_mtrr_fixed", &m_mtrr_fixed);
    state_field(obj, 128, (char *)"m_mtrr_variable_addr_mask", &m_mtrr_variable_addr_mask);
    state_field(obj, 8, (char *)"m_mtrr_deftype", &m_mtrr_deftype);
    state_field(obj, 8, (char *)"m_page_attribute_tables", &m_page_attribute_tables);
    state_field(obj, 4, (char *)"m_a20_mask", &m_a20_mask);
    state_field(obj, 8, (char *)"m_apic_base", &m_apic_base);
    state_field(obj, 8, (char *)"m_tsc_fudge", &m_tsc_fudge);
    state_field(obj, 4, (char *)"m_read_result", &m_read_result);
    state_field(obj, 4, (char *)"m_intr_line_state", &m_intr_line_state);
    state_field(obj, 4, (char *)"m_interrupts_blocked", &m_interrupts_blocked);
    state_field(obj, 4, (char *)"m_exit_reason", &m_exit_reason);
    state_field(obj, 8, (char *)"m_ia32_efer", &m_ia32_efer);
    state_field(obj, 12, (char *)"m_sysenter", &m_sysenter);
    state_file(m_memory_size, (char *)"ram", m_mem);
    if (state_is_reading()) {
        cpu_trace_flush();
        cpu_mmu_tlb_flush();
        cpu_prot_update_cpl();
        cpu_update_mxcsr();
    }
}
void _cpu_state()
{
    cpu.cpu_state();
}
void _cpu_reset()
{
    cpu.cpu_reset();
}
int CPU::cpu_init(void)
{
    state_register(_cpu_state);
    io_register_reset(_cpu_reset);
    fpu_init();
    return 0;
}
void CPU::cpu_init_dma(uint32_t page)
{
    cpu_smc_invalidate_page(page);
}
void CPU::cpu_write_mem(uint32_t addr, void *data, uint32_t length)
{
    if (length <= 4) {
        switch (length) {
            case 1:
                m_mem8[addr] = *(uint8_t *)data;
                return;
            case 2:
                m_mem16[addr >> 1] = *(uint16_t *)data;
                return;
            case 4:
                m_mem32[addr >> 2] = *(uint32_t *)data;
                return;
        }
    }
    memcpy((uint8_t *)m_mem + addr, data, length);
}
void CPU::cpu_debug(void)
{
    printf("EAX: %08x ECX: %08x EDX: %08x EBX: %08x\n", m_reg32[0], m_reg32[1], m_reg32[2], m_reg32[3]);
    printf("ESP: %08x EBP: %08x ESI: %08x EDI: %08x\n", m_reg32[4], m_reg32[5], m_reg32[6], m_reg32[7]);
    printf("EFLAGS: %08x\n", cpu_get_eflags());
    printf("CS:EIP: %04x:%08x (lin: %08x) Physical EIP: %08x\n", m_seg[1], (m_phys_eip + m_eip_phys_bias),
           (m_phys_eip + m_eip_phys_bias + m_seg_base[1]), m_phys_eip);
    printf("Translation mode: %d-bit\n", m_state_hash ? 16 : 32);
    printf("Physical RAM base: %p Cycles to run: %d Cycles executed: %d\n", m_mem, m_cycles_to_run,
           (uint32_t)cpu_get_cycles());
}
