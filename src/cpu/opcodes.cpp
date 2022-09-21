#include "cpu.h"
#include "../devices.h"
#include "../display.h"
#include <alloca.h>
#include "../io.h"


extern CPU cpu;
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))


void CPU::interrupt_guard(void)
{
    m_cycles += cpu_get_cycles() - m_cycles;
    if (m_cycles_to_run != 1) {
        m_refill_counter = m_cycles_to_run - 2;
        m_cycles_to_run  = 2;
        m_cycle_offset   = 2;
    } else {
        m_cycles_to_run      = 1;
        m_cycle_offset       = 1;
        m_refill_counter     = 0;
        m_interrupts_blocked = 1;
    }
}
uint32_t CPU::cpu_get_linaddr(uint32_t i, optype j)
{
    uint32_t addr = m_reg32[i >> 8 & 15];
    addr += m_reg32[i >> 16 & 15] << (i >> 20 & 3);
    addr += j->disp32;
    return (addr & ((i << 12 & 65536) - 1)) + m_seg_base[i >> 22 & 7];
}
uint32_t CPU::cpu_get_virtaddr(uint32_t i, optype j)
{
    uint32_t addr = m_reg32[i >> 8 & 15];
    addr += m_reg32[i >> 16 & 15] << (i >> 20 & 3);
    addr += j->disp32;
    return (addr & ((i << 12 & 65536) - 1));
}
void CPU::cpu_execute(void)
{
    optype i = cpu_get_trace();
    do {
        i = CALL_MEMBER_FN(*this, i->handler)(i);

        if (!--m_cycles_to_run)
            break;
    } while (1);
}
optype CPU::op_ud_exception(optype i)
{
    (void)(i);
    do {
        cpu_exception(6, 0);
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    } while (0);
}
optype CPU::op_trace_end(optype i)
{
    (void)(i);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_nop(optype i)
{
    (void)(i);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_jmp_r16(optype i)
{
    uint32_t dest = m_reg16[i->flags >> 8 & 15];
    if (dest >= m_seg_limit[1])
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_phys_eip += (dest) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmp_r32(optype i)
{
    uint32_t dest = m_reg32[i->flags >> 8 & 15];
    if (dest >= m_seg_limit[1])
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_phys_eip += (dest) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmp_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmp_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_call_r16(optype i)
{
    uint32_t flags = i->flags;
    if (cpu_push16(((flags & 15) + (m_phys_eip + m_eip_phys_bias))))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (m_reg16[flags >> 8 & 15]) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_call_r32(optype i)
{
    uint32_t flags = i->flags;
    if (cpu_push32(((flags & 15) + (m_phys_eip + m_eip_phys_bias))))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (m_reg32[flags >> 8 & 15]) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_call_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push16(((m_phys_eip + m_eip_phys_bias) + (flags & 15))))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_call_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push32(((m_phys_eip + m_eip_phys_bias) + (flags & 15))))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (src) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmp_rel32(optype i)
{
    m_phys_eip += i->flags + i->imm32;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmp_rel16(optype i)
{
    uint32_t virt = (m_phys_eip + m_eip_phys_bias);
    m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmpf(optype i)
{
    if (jmpf(i->imm32, i->disp16, (m_phys_eip + m_eip_phys_bias) + i->flags))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmpf_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip = 0, cs = 0;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (jmpf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_jmpf_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip, cs;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (jmpf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_callf16_ap(optype i)
{
    uint32_t eip = i->imm32, cs = i->disp16, eip_after = (m_phys_eip + m_eip_phys_bias) + i->flags;
    if (callf(eip, cs, eip_after, 0))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_callf32_ap(optype i)
{
    uint32_t eip = i->imm32, cs = i->disp32, eip_after = (m_phys_eip + m_eip_phys_bias) + i->flags;
    if (callf(eip, cs, eip_after, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_callf_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip = 0, cs = 0;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (callf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15), 0))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_callf_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), eip, cs;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                eip = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            eip = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                cs = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            cs = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (callf(eip, cs, (m_phys_eip + m_eip_phys_bias) + (flags & 15), 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_retf16(optype i)
{
    if (retf(i->imm16, 0))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_retf32(optype i)
{
    if (retf(i->imm16, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_iret16(optype i)
{
    if (iret((m_phys_eip + m_eip_phys_bias) + i->flags, 0))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_iret32(optype i)
{
    if (iret((m_phys_eip + m_eip_phys_bias) + i->flags, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_loop_rel16(optype i)
{
    uint32_t mask = i->disp32, cond = (m_reg32[1] - 1) & mask, virt = (m_phys_eip + m_eip_phys_bias);
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_loop_rel32(optype i)
{
    uint32_t mask = i->disp32, cond = (m_reg32[1] - 1) & mask;
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond) {
        m_phys_eip += i->flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_loopz_rel16(optype i)
{
    uint32_t mask = i->disp32, cond = (m_reg32[1] - 1) & mask, virt = (m_phys_eip + m_eip_phys_bias);
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && (m_lr == 0)) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_loopz_rel32(optype i)
{
    uint32_t mask = i->disp32, cond = (m_reg32[1] - 1) & mask;
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && (m_lr == 0)) {
        m_phys_eip += i->flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_loopnz_rel16(optype i)
{
    uint32_t mask = i->disp32, cond = (m_reg32[1] - 1) & mask, virt = (m_phys_eip + m_eip_phys_bias);
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && !(m_lr == 0)) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_loopnz_rel32(optype i)
{
    uint32_t mask = i->disp32, cond = (m_reg32[1] - 1) & mask;
    m_reg32[1] = cond | (m_reg32[1] & ~mask);
    if (cond && !(m_lr == 0)) {
        m_phys_eip += i->flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_jecxz_rel16(optype i)
{
    uint32_t virt = (m_phys_eip + m_eip_phys_bias);
    if (!(m_reg32[1] & i->disp32)) {
        m_phys_eip += ((virt + i->flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_jecxz_rel32(optype i)
{
    if (!(m_reg32[1] & i->disp32)) {
        m_phys_eip += i->flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_jo16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jo32(optype i)
{
    int flags = i->flags;
    if (cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jno16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jno32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jb16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_cf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jb32(optype i)
{
    int flags = i->flags;
    if (cpu_get_cf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnb16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_cf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnb32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_cf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jz16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (m_lr == 0) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jz32(optype i)
{
    int flags = i->flags;
    if (m_lr == 0) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnz16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!(m_lr == 0)) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnz32(optype i)
{
    int flags = i->flags;
    if (!(m_lr == 0)) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jbe16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if ((m_lr == 0) || cpu_get_cf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jbe32(optype i)
{
    int flags = i->flags;
    if ((m_lr == 0) || cpu_get_cf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnbe16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!((m_lr == 0) || cpu_get_cf())) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnbe32(optype i)
{
    int flags = i->flags;
    if (!((m_lr == 0) || cpu_get_cf())) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_js16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_sf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_js32(optype i)
{
    int flags = i->flags;
    if (cpu_get_sf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jns16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_sf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jns32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_sf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jp16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_pf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jp32(optype i)
{
    int flags = i->flags;
    if (cpu_get_pf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnp16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!cpu_get_pf()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnp32(optype i)
{
    int flags = i->flags;
    if (!cpu_get_pf()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jl16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_sf() != cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jl32(optype i)
{
    int flags = i->flags;
    if (cpu_get_sf() != cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnl16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (cpu_get_sf() == cpu_get_of()) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnl32(optype i)
{
    int flags = i->flags;
    if (cpu_get_sf() == cpu_get_of()) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jle16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if ((m_lr == 0) || (cpu_get_sf() != cpu_get_of())) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jle32(optype i)
{
    int flags = i->flags;
    if ((m_lr == 0) || (cpu_get_sf() != cpu_get_of())) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnle16(optype i)
{
    int      flags = i->flags;
    uint32_t virt  = (m_phys_eip + m_eip_phys_bias);
    if (!(m_lr == 0) && (cpu_get_sf() == cpu_get_of())) {
        m_phys_eip += ((virt + flags + i->imm32) & 0xFFFF) - virt;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_jnle32(optype i)
{
    int flags = i->flags;
    if (!(m_lr == 0) && (cpu_get_sf() == cpu_get_of())) {
        m_phys_eip += flags + i->imm32;
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    } else
        do {
            m_phys_eip += flags;
            do {
            } while (0);
            return i + 1;
        } while (1);
    ;
}
optype CPU::op_call_j16(optype i)
{
    uint32_t virt_base = (m_phys_eip + m_eip_phys_bias), virt = virt_base + i->flags;
    if (cpu_push16((virt)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += ((virt + i->imm32) & 0xFFFF) - virt_base;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_call_j32(optype i)
{
    uint32_t flags = i->flags, virt = (m_phys_eip + m_eip_phys_bias) + flags;
    if (cpu_push32((virt)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += flags + i->imm32;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_ret16(optype i)
{
    (void)(i);
    if (cpu_pop16((&utemp.d16)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (utemp.d16) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_ret32(optype i)
{
    (void)(i);
    if (cpu_pop32((&utemp.d32)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (utemp.d32) - (m_phys_eip + m_eip_phys_bias);
    ;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_ret16_iw(optype i)
{
    (void)(i);
    if (cpu_pop16((&utemp.d16)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (utemp.d16) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_reg32[4] = ((m_reg32[4] + i->imm16) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_ret32_iw(optype i)
{
    (void)(i);
    if (cpu_pop32((&utemp.d32)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_phys_eip += (utemp.d32) - (m_phys_eip + m_eip_phys_bias);
    ;
    m_reg32[4] = ((m_reg32[4] + i->imm16) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_int(optype i)
{
    if (cpu_interrupt(i->imm8, 0, INTERRUPT_TYPE_SOFTWARE, (m_phys_eip + m_eip_phys_bias) + i->flags))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_into(optype i)
{
    __asm__("int3");
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
    if (cpu_get_of()) {
        if (cpu_interrupt(4, 0, INTERRUPT_TYPE_SOFTWARE, (m_phys_eip + m_eip_phys_bias) + i->flags))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        do {
            do {
            } while (0);
            return cpu_get_trace();
        } while (0);
    }
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_r16(optype i)
{
    int flags = i->flags;
    if (cpu_push16((m_reg16[flags >> 8 & 15])))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_i16(optype i)
{
    int flags = i->flags;
    if (cpu_push16((i->imm16)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_e16(optype i)
{
    int      flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push16((src)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_r32(optype i)
{
    int flags = i->flags;
    if (cpu_push32((m_reg32[flags >> 8 & 15])))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_i32(optype i)
{
    int flags = i->flags;
    if (cpu_push32((i->imm32)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_e32(optype i)
{
    int      flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint32_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_push32((src)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pop_r16(optype i)
{
    int flags = i->flags;
    if (cpu_pop16((&utemp.d16)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_reg16[flags >> 8 & 15] = utemp.d16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pop_e16(optype i)
{
    uint32_t prev_esp = m_reg32[4], linaddr, flags = i->flags, temp_esp;
    if (cpu_pop16((&utemp.d16)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    linaddr    = cpu_get_linaddr(flags, i);
    temp_esp   = m_reg32[4];
    m_reg32[4] = prev_esp;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = utemp.d16, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    m_reg32[4] = temp_esp;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pop_e32(optype i)
{
    uint32_t prev_esp = m_reg32[4], linaddr, flags = i->flags, temp_esp;
    if (cpu_pop32((&temp.d32)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    linaddr    = cpu_get_linaddr(flags, i);
    temp_esp   = m_reg32[4];
    m_reg32[4] = prev_esp;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = temp.d32, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    m_reg32[4] = temp_esp;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pop_r32(optype i)
{
    int flags = i->flags;
    if (cpu_pop32((&temp.d32)))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    m_reg32[flags >> 8 & 15] = temp.d32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_s16(optype i)
{
    int flags = i->flags;
    if (cpu_push16((m_seg[flags >> 8 & 15])))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_push_s32(optype i)
{
    int      flags = i->flags;
    uint32_t esp = m_reg32[4], esp_mask = m_esp_mask, esp_minus_four = (esp - 4) & esp_mask;
    do {
        uint32_t addr_ = esp_minus_four + m_seg_base[2], shift_ = m_tlb_shift_write, data_ = m_seg[flags >> 8 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    m_reg32[4] = esp_minus_four | (esp & ~esp_mask);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pop_s16(optype i)
{
    int      flags = i->flags, seg_dest = flags >> 8 & 15;
    uint16_t dest;
    do {
        uint32_t addr_ = (m_reg32[4] & m_esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(seg_dest, dest))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg32[4] = ((m_reg32[4] + 2) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    if (seg_dest == 2)
        interrupt_guard();
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pop_s32(optype i)
{
    int      flags = i->flags, seg_dest = flags >> 8 & 15;
    uint16_t dest;
    do {
        uint32_t addr_ = (m_reg32[4] & m_esp_mask) + m_seg_base[2], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(seg_dest, dest))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg32[4] = ((m_reg32[4] + 4) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    if (seg_dest == 2)
        interrupt_guard();
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pusha(optype i)
{
    if (cpu_pusha())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pushad(optype i)
{
    if (cpu_pushad())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_popa(optype i)
{
    if (cpu_popa())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_popad(optype i)
{
    if (cpu_popad())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r8r8(optype i)
{
    int flags = i->flags;
    cpu_arith8(flags >> 25 & 7, &m_reg8[flags >> 8 & 15], m_reg8[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r8i8(optype i)
{
    int flags = i->flags;
    cpu_arith8(flags >> 25 & 7, &m_reg8[flags >> 8 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r8e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint8_t  res;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                res = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            res = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    cpu_arith8(flags >> 25 & 7, &m_reg8[flags >> 12 & 15], res);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_e8r8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_arith8(flags >> 25 & 7, (uint8_t *)&m_read_result, m_reg8[flags >> 12 & 15]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_arith8(flags >> 25 & 7, ptr, m_reg8[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_e8i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_arith8(flags >> 25 & 7, (uint8_t *)&m_read_result, i->imm8);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_arith8(flags >> 25 & 7, ptr, i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r16r16(optype i)
{
    int flags = i->flags;
    cpu_arith16(flags >> 25 & 7, &m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r16i16(optype i)
{
    int flags = i->flags;
    cpu_arith16(flags >> 25 & 7, &m_reg16[flags >> 8 & 15], i->imm16);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t res;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                res = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            res = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    cpu_arith16(flags >> 25 & 7, &m_reg16[flags >> 12 & 15], res);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_e16r16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_arith16(flags >> 25 & 7, (uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_arith16(flags >> 25 & 7, ptr, m_reg16[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_e16i16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_arith16(flags >> 25 & 7, (uint16_t *)&m_read_result, i->imm16);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_arith16(flags >> 25 & 7, ptr, i->imm16);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r32r32(optype i)
{
    int flags = i->flags;
    cpu_arith32(flags >> 25 & 7, &m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r32i32(optype i)
{
    int flags = i->flags;
    cpu_arith32(flags >> 25 & 7, &m_reg32[flags >> 8 & 15], i->imm32);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint32_t res;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                res = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            res = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    cpu_arith32(flags >> 25 & 7, &m_reg32[flags >> 12 & 15], res);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_e32r32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_arith32(flags >> 25 & 7, (uint32_t *)&m_read_result, m_reg32[flags >> 12 & 15]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_arith32(flags >> 25 & 7, ptr, m_reg32[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arith_e32i32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_arith32(flags >> 25 & 7, (uint32_t *)&m_read_result, i->imm32);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_arith32(flags >> 25 & 7, ptr, i->imm32);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_r8cl(optype i)
{
    int flags = i->flags;
    cpu_shift8(flags >> 25 & 7, &m_reg8[flags >> 8 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_r8i8(optype i)
{
    int flags = i->flags;
    cpu_shift8(flags >> 25 & 7, &m_reg8[flags >> 8 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_e8cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shift8(flags >> 25 & 7, (uint8_t *)&m_read_result, m_reg8[4]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shift8(flags >> 25 & 7, ptr, m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_e8i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shift8(flags >> 25 & 7, (uint8_t *)&m_read_result, i->imm8);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shift8(flags >> 25 & 7, ptr, i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_r16cl(optype i)
{
    int flags = i->flags;
    cpu_shift16(flags >> 25 & 7, &m_reg16[flags >> 8 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_r16i16(optype i)
{
    int flags = i->flags;
    cpu_shift16(flags >> 25 & 7, &m_reg16[flags >> 8 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_e16cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shift16(flags >> 25 & 7, (uint16_t *)&m_read_result, m_reg8[4]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shift16(flags >> 25 & 7, ptr, m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_e16i16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shift16(flags >> 25 & 7, (uint16_t *)&m_read_result, i->imm8);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shift16(flags >> 25 & 7, ptr, i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_r32cl(optype i)
{
    int flags = i->flags;
    cpu_shift32(flags >> 25 & 7, &m_reg32[flags >> 8 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_r32i32(optype i)
{
    int flags = i->flags;
    cpu_shift32(flags >> 25 & 7, &m_reg32[flags >> 8 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_e32cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shift32(flags >> 25 & 7, (uint32_t *)&m_read_result, m_reg8[4]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shift32(flags >> 25 & 7, ptr, m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shift_e32i32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shift32(flags >> 25 & 7, (uint32_t *)&m_read_result, i->imm8);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shift32(flags >> 25 & 7, ptr, i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_e8r8(optype i)
{
    uint32_t flags = i->flags;
    uint8_t  src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = m_reg8[flags >> 12 & 15];
    m_lr   = (int8_t)(src - m_lop2);
    m_laux = SUB8;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r8r8(optype i)
{
    uint32_t flags = i->flags;
    m_lop2         = m_reg8[flags >> 12 & 15];
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB8;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r8e8(optype i)
{
    uint32_t flags = i->flags;
    uint8_t  src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = src;
    m_lr   = (int8_t)(m_reg8[flags >> 12 & 15] - src);
    m_laux = SUB8;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r8i8(optype i)
{
    uint32_t flags = i->flags;
    m_lop2         = i->imm8;
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB8;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_e8i8(optype i)
{
    uint32_t flags = i->flags;
    uint8_t  src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = i->imm8;
    m_lr   = (int8_t)(src - m_lop2);
    m_laux = SUB8;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_e16r16(optype i)
{
    uint32_t flags = i->flags;
    uint16_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = m_reg16[flags >> 12 & 15];
    m_lr   = (int16_t)(src - m_lop2);
    m_laux = SUB16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r16r16(optype i)
{
    uint32_t flags = i->flags;
    m_lop2         = m_reg16[flags >> 12 & 15];
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r16e16(optype i)
{
    uint32_t flags = i->flags;
    uint16_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = src;
    m_lr   = (int16_t)(m_reg16[flags >> 12 & 15] - src);
    m_laux = SUB16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r16i16(optype i)
{
    uint32_t flags = i->flags;
    m_lop2         = i->imm16;
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_e16i16(optype i)
{
    uint32_t flags = i->flags;
    uint16_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = i->imm16;
    m_lr   = (int16_t)(src - m_lop2);
    m_laux = SUB16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_e32r32(optype i)
{
    uint32_t flags = i->flags;
    uint32_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = m_reg32[flags >> 12 & 15];
    m_lr   = (int32_t)(src - m_lop2);
    m_laux = SUB32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r32r32(optype i)
{
    uint32_t flags = i->flags;
    m_lop2         = m_reg32[flags >> 12 & 15];
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r32e32(optype i)
{
    uint32_t flags = i->flags;
    uint32_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = src;
    m_lr   = (int32_t)(m_reg32[flags >> 12 & 15] - src);
    m_laux = SUB32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_r32i32(optype i)
{
    uint32_t flags = i->flags;
    m_lop2         = i->imm32;
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] - m_lop2);
    m_laux         = SUB32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmp_e32i32(optype i)
{
    uint32_t flags = i->flags;
    uint32_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lop2 = i->imm32;
    m_lr   = (int32_t)(src - m_lop2);
    m_laux = SUB32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_e8r8(optype i)
{
    uint32_t flags = i->flags;
    uint8_t  src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int8_t)(src & m_reg8[flags >> 12 & 15]);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r8r8(optype i)
{
    uint32_t flags = i->flags;
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] & m_reg8[flags >> 12 & 15]);
    m_laux         = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r8e8(optype i)
{
    uint32_t flags = i->flags;
    uint8_t  src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int8_t)(src & m_reg8[flags >> 12 & 15]);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r8i8(optype i)
{
    uint32_t flags = i->flags;
    m_lr           = (int8_t)(m_reg8[flags >> 8 & 15] & i->imm8);
    m_laux         = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_e8i8(optype i)
{
    uint32_t flags = i->flags;
    uint8_t  src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int8_t)(i->imm8 & src);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_e16r16(optype i)
{
    uint32_t flags = i->flags;
    uint16_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int16_t)(src & m_reg16[flags >> 12 & 15]);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r16r16(optype i)
{
    uint32_t flags = i->flags;
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] & m_reg16[flags >> 12 & 15]);
    m_laux         = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r16e16(optype i)
{
    uint32_t flags = i->flags;
    uint16_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int16_t)(src & m_reg16[flags >> 12 & 15]);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r16i16(optype i)
{
    uint32_t flags = i->flags;
    m_lr           = (int16_t)(m_reg16[flags >> 8 & 15] & i->imm16);
    m_laux         = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_e16i16(optype i)
{
    uint32_t flags = i->flags;
    uint16_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int16_t)(i->imm16 & src);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_e32r32(optype i)
{
    uint32_t flags = i->flags;
    uint32_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int32_t)(src & m_reg32[flags >> 12 & 15]);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r32r32(optype i)
{
    uint32_t flags = i->flags;
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] & m_reg32[flags >> 12 & 15]);
    m_laux         = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r32e32(optype i)
{
    uint32_t flags = i->flags;
    uint32_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int32_t)(src & m_reg32[flags >> 12 & 15]);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_r32i32(optype i)
{
    uint32_t flags = i->flags;
    m_lr           = (int32_t)(m_reg32[flags >> 8 & 15] & i->imm32);
    m_laux         = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_test_e32i32(optype i)
{
    uint32_t flags = i->flags;
    uint32_t src;
    do {
        uint32_t addr_ = cpu_get_linaddr(flags, i), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_lr   = (int32_t)(i->imm32 & src);
    m_laux = BIT;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_inc_r8(optype i)
{
    uint32_t flags = i->flags;
    cpu_inc8(&m_reg8[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_inc_e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_inc8((uint8_t *)&m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_inc8(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_inc_r16(optype i)
{
    uint32_t flags = i->flags;
    cpu_inc16(&m_reg16[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_inc_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_inc16((uint16_t *)&m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_inc16(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_inc_r32(optype i)
{
    uint32_t flags = i->flags;
    cpu_inc32(&m_reg32[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_inc_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_inc32((uint32_t *)&m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_inc32(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_dec_r8(optype i)
{
    uint32_t flags = i->flags;
    cpu_dec8(&m_reg8[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_dec_e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_dec8((uint8_t *)&m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_dec8(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_dec_r16(optype i)
{
    uint32_t flags = i->flags;
    cpu_dec16(&m_reg16[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_dec_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_dec16((uint16_t *)&m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_dec16(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_dec_r32(optype i)
{
    uint32_t flags = i->flags;
    cpu_dec32(&m_reg32[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_dec_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_dec32((uint32_t *)&m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_dec32(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_not_r8(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg8[rm] = ~m_reg8[rm];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_not_e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_not8((uint8_t *)&m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_not8(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_not_r16(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg16[rm] = ~m_reg16[rm];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_not_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_not16((uint16_t *)&m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_not16(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_not_r32(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    m_reg32[rm] = ~m_reg32[rm];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_not_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_not32((uint32_t *)&m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_not32(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_neg_r8(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    cpu_neg8(&m_reg8[rm]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_neg_e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_neg8((uint8_t *)&m_read_result);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_neg8(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_neg_r16(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    cpu_neg16(&m_reg16[rm]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_neg_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_neg16((uint16_t *)&m_read_result);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_neg16(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_neg_r32(optype i)
{
    int flags = i->flags, rm = flags >> 8 & 15;
    cpu_neg32(&m_reg32[rm]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_neg_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_neg32((uint32_t *)&m_read_result);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_neg32(ptr);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_muldiv_r8(optype i)
{
    uint32_t flags = i->flags;
    if (cpu_muldiv8(flags >> 25 & 7, m_reg8[flags >> 8 & 15]))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_muldiv_e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint8_t  src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_muldiv8(flags >> 25 & 7, src))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_muldiv_r16(optype i)
{
    uint32_t flags = i->flags;
    if (cpu_muldiv16(flags >> 25 & 7, m_reg16[flags >> 8 & 15]))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_muldiv_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_muldiv16(flags >> 25 & 7, src))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_muldiv_r32(optype i)
{
    uint32_t flags = i->flags;
    if (cpu_muldiv32(flags >> 25 & 7, m_reg32[flags >> 8 & 15]))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_muldiv_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint32_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_muldiv32(flags >> 25 & 7, src))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r16r16i16(optype i)
{
    uint32_t flags            = i->flags;
    m_reg16[flags >> 12 & 15] = cpu_imul16(m_reg16[flags >> 8 & 15], i->imm16);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r16e16i16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = cpu_imul16(src, i->imm16);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r32r32i32(optype i)
{
    uint32_t flags            = i->flags;
    m_reg32[flags >> 12 & 15] = cpu_imul32(m_reg32[flags >> 8 & 15], i->imm32);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r32e32i32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint32_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = cpu_imul32(src, i->imm32);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r16r16(optype i)
{
    uint32_t flags            = i->flags;
    m_reg16[flags >> 12 & 15] = cpu_imul16(m_reg16[flags >> 12 & 15], m_reg16[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r32r32(optype i)
{
    uint32_t flags            = i->flags;
    m_reg32[flags >> 12 & 15] = cpu_imul32(m_reg32[flags >> 12 & 15], m_reg32[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = cpu_imul16(m_reg16[flags >> 12 & 15], src);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_imul_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    uint32_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = cpu_imul32(m_reg32[flags >> 12 & 15], src);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_r16r16i8(optype i)
{
    int flags = i->flags;
    cpu_shrd16(&m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_r32r32i8(optype i)
{
    int flags = i->flags;
    cpu_shrd32(&m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_r16r16cl(optype i)
{
    int flags = i->flags;
    cpu_shrd16(&m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_r32r32cl(optype i)
{
    int flags = i->flags;
    cpu_shrd32(&m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_e16r16i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shrd16((uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15], i->imm8);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shrd16(ptr, m_reg16[flags >> 12 & 15], i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_e32r32i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shrd32((uint32_t *)&m_read_result, m_reg32[flags >> 12 & 15], i->imm8);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shrd32(ptr, m_reg32[flags >> 12 & 15], i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_e16r16cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shrd16((uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shrd16(ptr, m_reg16[flags >> 12 & 15], m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shrd_e32r32cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shrd32((uint32_t *)&m_read_result, m_reg32[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shrd32(ptr, m_reg32[flags >> 12 & 15], m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_r16r16i8(optype i)
{
    int flags = i->flags;
    cpu_shld16(&m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_r32r32i8(optype i)
{
    int flags = i->flags;
    cpu_shld32(&m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_r16r16cl(optype i)
{
    int flags = i->flags;
    cpu_shld16(&m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_r32r32cl(optype i)
{
    int flags = i->flags;
    cpu_shld32(&m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15], m_reg8[4]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_e16r16i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shld16((uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15], i->imm8);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shld16(ptr, m_reg16[flags >> 12 & 15], i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_e32r32i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shld32((uint32_t *)&m_read_result, m_reg32[flags >> 12 & 15], i->imm8);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shld32(ptr, m_reg32[flags >> 12 & 15], i->imm8);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_e16r16cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shld16((uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shld16(ptr, m_reg16[flags >> 12 & 15], m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_shld_e32r32cl(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_shld32((uint32_t *)&m_read_result, m_reg32[flags >> 12 & 15], m_reg8[4]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_shld32(ptr, m_reg32[flags >> 12 & 15], m_reg8[4]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_out_i8al(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    cpu_outb(port, m_reg8[0]);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_out_i8ax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 2))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    cpu_outw(port, m_reg16[0]);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_out_i8eax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 4))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    cpu_outd(port, m_reg32[0]);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_in_i8al(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg8[0] = cpu_inb(port);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_in_i8ax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 2))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg16[0] = cpu_inw(port);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_in_i8eax(optype i)
{
    int port = i->imm8;
    if (cpu_io_check_access(port, 4))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg32[0] = cpu_ind(port);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_out_dxal(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    cpu_outb(port, m_reg8[0]);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_out_dxax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 2))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    cpu_outw(port, m_reg16[0]);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_out_dxeax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 4))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    cpu_outd(port, m_reg32[0]);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_in_dxal(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 1))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg8[0] = cpu_inb(port);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_in_dxax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 2))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg16[0] = cpu_inw(port);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_in_dxeax(optype i)
{
    int port = m_reg16[4];
    if (cpu_io_check_access(port, 4))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    m_reg32[0] = cpu_ind(port);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r8i8(optype i)
{
    int flags               = i->flags;
    m_reg8[flags >> 8 & 15] = i->imm8;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r16i16(optype i)
{
    int flags                = i->flags;
    m_reg16[flags >> 8 & 15] = i->imm16;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r32i32(optype i)
{
    int flags                = i->flags;
    m_reg32[flags >> 8 & 15] = i->imm32;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r8e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg8[flags >> 12 & 15] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg8[flags >> 12 & 15] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r8r8(optype i)
{
    uint32_t flags          = i->flags;
    m_reg8[flags >> 8 & 15] = m_reg8[flags >> 12 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e8r8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg8[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e8i8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = i->imm8, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg16[flags >> 12 & 15] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg16[flags >> 12 & 15] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r16r16(optype i)
{
    uint32_t flags           = i->flags;
    m_reg16[flags >> 8 & 15] = m_reg16[flags >> 12 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e16r16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg16[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e16i16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = i->imm16, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg32[flags >> 12 & 15] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg32[flags >> 12 & 15] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r32r32(optype i)
{
    uint32_t flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_reg32[flags >> 12 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e32r32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg32[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e32i32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = i->imm32, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_s16r16(optype i)
{
    int flags = i->flags, dest = flags >> 12 & 15;
    if (cpu_load_seg_value_mov(dest, m_reg16[flags >> 8 & 15]))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    if (dest == 2)
        interrupt_guard();
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_s16e16(optype i)
{
    uint32_t flags = i->flags, dest = flags >> 12 & 15, linaddr = cpu_get_linaddr(flags, i);
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(dest, src))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    if (dest == 2)
        interrupt_guard();
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_e16s16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg[flags >> 12 & 15],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r16s16(optype i)
{
    uint32_t flags           = i->flags;
    m_reg16[flags >> 8 & 15] = m_seg[flags >> 12 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r32s16(optype i)
{
    uint32_t flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_seg[flags >> 12 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_eaxm32(optype i)
{
    int flags = i->flags;
    do {
        uint32_t addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg32[0] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg32[0] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_axm16(optype i)
{
    int flags = i->flags;
    do {
        uint32_t addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg16[0] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg16[0] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_alm8(optype i)
{
    int flags = i->flags;
    do {
        uint32_t addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg8[0] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg8[0] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_m32eax(optype i)
{
    int flags = i->flags;
    do {
        uint32_t addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_write, data_ = m_reg32[0],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_m16ax(optype i)
{
    int flags = i->flags;
    do {
        uint32_t addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_write, data_ = m_reg16[0],
                 tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_m8al(optype i)
{
    int flags = i->flags;
    do {
        uint32_t addr_ = i->imm32 + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_write, data_ = m_reg8[0],
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmov_r16e16(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_cond(flags >> 25 & 15))
        m_reg16[flags >> 12 & 15] = dest;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmov_r16r16(optype i)
{
    int flags = i->flags;
    if (cpu_cond(flags >> 25 & 15))
        m_reg16[flags >> 12 & 15] = m_reg16[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmov_r32e32(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_cond(flags >> 25 & 15))
        m_reg32[flags >> 12 & 15] = dest;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmov_r32r32(optype i)
{
    int flags = i->flags;
    if (cpu_cond(flags >> 25 & 15))
        m_reg32[flags >> 12 & 15] = m_reg32[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_setcc_e8(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = cpu_cond(flags >> 25 & 15),
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (cpu_access_write8(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_setcc_r8(optype i)
{
    int flags               = i->flags;
    m_reg8[flags >> 8 & 15] = cpu_cond(flags >> 25 & 15);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lea_r16e16(optype i)
{
    uint32_t flags            = i->flags;
    m_reg16[flags >> 12 & 15] = cpu_get_virtaddr(flags, i);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lea_r32e32(optype i)
{
    uint32_t flags            = i->flags;
    m_reg32[flags >> 12 & 15] = cpu_get_virtaddr(flags, i);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lds_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(3, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lds_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(3, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_les_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(0, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_les_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(0, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lss_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(2, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lss_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(2, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lfs_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(4, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lfs_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(4, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lgs_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(5, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lgs_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (cpu_load_seg_value_mov(5, data))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = data;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xchg_r8r8(optype i)
{
    uint32_t flags           = i->flags;
    uint8_t  temp            = m_reg8[flags >> 8 & 15];
    m_reg8[flags >> 8 & 15]  = m_reg8[flags >> 12 & 15];
    m_reg8[flags >> 12 & 15] = temp;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xchg_r16r16(optype i)
{
    uint32_t flags            = i->flags;
    uint16_t temp             = m_reg16[flags >> 8 & 15];
    m_reg16[flags >> 8 & 15]  = m_reg16[flags >> 12 & 15];
    m_reg16[flags >> 12 & 15] = temp;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xchg_r32r32(optype i)
{
    uint32_t flags            = i->flags;
    uint32_t temp             = m_reg32[flags >> 8 & 15];
    m_reg32[flags >> 8 & 15]  = m_reg32[flags >> 12 & 15];
    m_reg32[flags >> 12 & 15] = temp;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xchg_r8e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int      tlb_info = m_tlb_tags[linaddr >> 12];
    uint8_t *ptr;
    if ((tlb_info >> m_tlb_shift_write & 1)) {
        if (cpu_access_read8(linaddr, tlb_info, m_tlb_shift_write))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_access_write8(linaddr, m_reg8[flags >> 12 & 15], tlb_info, m_tlb_shift_write);
        m_reg8[flags >> 12 & 15] = m_read_result;
    } else {
        ptr                      = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        uint8_t tmp              = *ptr;
        *ptr                     = m_reg8[flags >> 12 & 15];
        m_reg8[flags >> 12 & 15] = tmp;
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xchg_r16e16(optype i)
{
    uint32_t  flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int       tlb_info = m_tlb_tags[linaddr >> 12];
    uint16_t *ptr;
    if (((linaddr | tlb_info >> m_tlb_shift_write) & 1)) {
        tlb_info >>= m_tlb_shift_write;
        if (cpu_access_read16(linaddr, tlb_info, m_tlb_shift_write))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_access_write16(linaddr, m_reg16[flags >> 12 & 15], tlb_info, m_tlb_shift_write);
        m_reg16[flags >> 12 & 15] = m_read_result;
    } else {
        ptr                       = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        uint16_t tmp              = *ptr;
        *ptr                      = m_reg16[flags >> 12 & 15];
        m_reg16[flags >> 12 & 15] = tmp;
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xchg_r32e32(optype i)
{
    uint32_t  flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int       tlb_info = m_tlb_tags[linaddr >> 12];
    uint32_t *ptr;
    if (((linaddr | tlb_info >> m_tlb_shift_write) & 3)) {
        tlb_info >>= m_tlb_shift_write;
        if (cpu_access_read32(linaddr, tlb_info, m_tlb_shift_write))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_access_write32(linaddr, m_reg32[flags >> 12 & 15], tlb_info, m_tlb_shift_write);
        m_reg32[flags >> 12 & 15] = m_read_result;
    } else {
        ptr                       = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        uint32_t tmp              = *ptr;
        *ptr                      = m_reg32[flags >> 12 & 15];
        m_reg32[flags >> 12 & 15] = tmp;
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg_r8r8(optype i)
{
    uint32_t flags = i->flags;
    cpu_cmpxchg8(&m_reg8[flags >> 8 & 15], m_reg8[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg_e8r8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_cmpxchg8((uint8_t *)&m_read_result, m_reg8[flags >> 12 & 15]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_cmpxchg8(ptr, m_reg8[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg_r16r16(optype i)
{
    uint32_t flags = i->flags;
    cpu_cmpxchg16(&m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg_e16r16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_cmpxchg16((uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_cmpxchg16(ptr, m_reg16[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg_r32r32(optype i)
{
    uint32_t flags = i->flags;
    cpu_cmpxchg32(&m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg_e32r32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        cpu_cmpxchg32((uint32_t *)&m_read_result, m_reg32[flags >> 12 & 15]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        cpu_cmpxchg32(ptr, m_reg32[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmpxchg8b_e32(optype i)
{
    uint32_t flags = i->flags, low64, high64, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                low64 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            low64 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_write, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                high64 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            high64 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (m_reg32[0] == low64 && m_reg32[2] == high64) {
        cpu_set_zf(1);
        do {
            uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_reg32[3], tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    do {
                        m_cycles_to_run++;
                        return cpu_get_trace();
                    } while (0);
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
        do {
            uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_write, data_ = m_reg32[1], tag = m_tlb_tags[addr_ >> 12];
            if (((addr_ | tag >> shift_) & 3)) {
                if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                    do {
                        m_cycles_to_run++;
                        return cpu_get_trace();
                    } while (0);
            } else
                *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
        } while (0);
    } else {
        cpu_set_zf(0);
        m_reg32[0] = low64;
        m_reg32[2] = high64;
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xadd_r8r8(optype i)
{
    uint32_t flags = i->flags;
    xadd8(&m_reg8[flags >> 8 & 15], &m_reg8[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xadd_r8e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint8_t *ptr;
    if ((tlb_shift >> shift & 1)) {
        if (cpu_access_read8(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        xadd8((uint8_t *)&m_read_result, &m_reg8[flags >> 12 & 15]);
        cpu_access_write8(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = ((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        xadd8(ptr, &m_reg8[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xadd_r16r16(optype i)
{
    uint32_t flags = i->flags;
    xadd16(&m_reg16[flags >> 8 & 15], &m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xadd_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        xadd16((uint16_t *)&m_read_result, &m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        xadd16(ptr, &m_reg16[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xadd_r32r32(optype i)
{
    uint32_t flags = i->flags;
    xadd32(&m_reg32[flags >> 8 & 15], &m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xadd_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        xadd32((uint32_t *)&m_read_result, &m_reg32[flags >> 12 & 15]);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        xadd32(ptr, &m_reg32[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bound_r16e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int16_t  index16 = m_reg16[flags >> 12 & 15], low, hi;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                low = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            low = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                hi = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            hi = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (index16 < low || index16 > hi)
        do {
            cpu_exception(5, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bound_r32e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    int32_t  index32 = m_reg32[flags >> 12 & 15], low, hi;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                low = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            low = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 4, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                hi = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            hi = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (index32 < low || index32 > hi)
        do {
            cpu_exception(5, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_daa(optype i)
{
    uint8_t old_al = m_reg8[0], old_cf = cpu_get_cf(), al = old_al;
    int     cond = (al & 0x0F) > 9 || cpu_get_af();
    if (cond) {
        al += 6;
    }
    cpu_set_af(cond);
    cond = (old_al > 0x99) || old_cf;
    if (cond)
        al += 0x60;
    cpu_set_cf(cond);
    m_lr      = (int8_t)al;
    m_reg8[0] = al;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_das(optype i)
{
    uint8_t old_al = m_reg8[0], old_cf = cpu_get_cf(), cf = 0, al = old_al;
    int     cond = (old_al & 0x0F) > 9 || cpu_get_af();
    if (cond) {
        al -= 6;
        cf = old_cf | (al > old_al);
    }
    cpu_set_af(cond);
    if (old_al > 0x99 || old_cf == 1) {
        al -= 0x60;
        cf = 1;
    }
    cpu_set_cf(cf);
    m_lr      = (int8_t)al;
    m_reg8[0] = al;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_aaa(optype i)
{
    int cond = 0;
    if ((m_reg8[0] & 15) > 9 || cpu_get_af()) {
        m_reg16[0] += 0x106;
        cond = 1;
    }
    m_reg8[0] &= 15;
    m_laux = BIT;
    m_lr   = (int8_t)m_reg8[0];
    cpu_set_af(cond);
    cpu_set_cf(cond);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_aas(optype i)
{
    int cond = (m_reg8[0] & 0x0F) > 9 || cpu_get_af();
    if (cond) {
        m_reg16[0] -= 6;
        m_reg8[1] -= 1;
    }
    m_reg8[0] &= 0x0F;
    m_laux = BIT;
    m_lr   = (int8_t)m_reg8[0];
    cpu_set_af(cond);
    cpu_set_cf(cond);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_aam(optype i)
{
    if (i->imm8 == 0)
        do {
            cpu_exception(0, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint8_t temp_al = m_reg8[0];
    m_reg8[1]       = temp_al / i->imm8;
    m_lr            = (int8_t)(m_reg8[0] = temp_al % i->imm8);
    m_laux          = BIT;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_aad(optype i)
{
    uint8_t temp_al = m_reg8[0];
    uint8_t temp_ah = m_reg8[1];
    m_lr            = (int8_t)(m_reg8[0] = ((temp_al + (temp_ah * i->imm8)) & 0xFF));
    m_reg8[1]       = 0;
    m_laux          = BIT;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bt_r16(optype i)
{
    uint32_t flags = i->flags;
    bt16(m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bts_r16(optype i)
{
    uint32_t flags = i->flags;
    bts16(&m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btc_r16(optype i)
{
    uint32_t flags = i->flags;
    btc16(&m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btr_r16(optype i)
{
    uint32_t flags = i->flags;
    btr16(&m_reg16[flags >> 8 & 15], (m_reg16[flags >> 12 & 15] & i->disp16) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bt_r32(optype i)
{
    uint32_t flags = i->flags;
    bt32(m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bts_r32(optype i)
{
    uint32_t flags = i->flags;
    bts32(&m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btc_r32(optype i)
{
    uint32_t flags = i->flags;
    btc32(&m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btr_r32(optype i)
{
    uint32_t flags = i->flags;
    btr32(&m_reg32[flags >> 8 & 15], (m_reg32[flags >> 12 & 15] & i->disp32) + i->imm8);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bt_e16(optype i)
{
    uint32_t flags = i->flags, x = (flags & (1 << 25)) ? i->imm8 : m_reg16[flags >> 12 & 15],
             linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        uint32_t addr_ = linaddr + ((x / 16) * 2), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    bt16(dest, x);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bt_e32(optype i)
{
    uint32_t flags = i->flags, x = (flags & (1 << 25)) ? i->imm8 : m_reg32[flags >> 12 & 15],
             linaddr = cpu_get_linaddr(flags, i), dest;
    do {
        uint32_t addr_ = linaddr + ((x / 32) * 4), shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                dest = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            dest = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    bt32(dest, x);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bts_e16(optype i)
{
    uint32_t x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg16[i->flags >> 12 & 15];
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 16) * 2),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        bts16((uint16_t *)&m_read_result, x);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        bts16(ptr, x);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btc_e16(optype i)
{
    uint32_t x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg16[i->flags >> 12 & 15];
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 16) * 2),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        btc16((uint16_t *)&m_read_result, x);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        btc16(ptr, x);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btr_e16(optype i)
{
    uint32_t x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg16[i->flags >> 12 & 15];
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 16) * 2),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        btr16((uint16_t *)&m_read_result, x);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        btr16(ptr, x);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bts_e32(optype i)
{
    uint32_t x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg32[i->flags >> 12 & 15];
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 32) * 4),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        bts32((uint32_t *)&m_read_result, x);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        bts32(ptr, x);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btc_e32(optype i)
{
    uint32_t x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg32[i->flags >> 12 & 15];
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 32) * 4),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        btc32((uint32_t *)&m_read_result, x);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        btc32(ptr, x);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_btr_e32(optype i)
{
    uint32_t x     = (i->flags & (1 << 25)) ? i->imm8 : m_reg32[i->flags >> 12 & 15];
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i) + ((x / 32) * 4),
             tlb_shift = m_tlb_tags[linaddr >> 12], shift = m_tlb_shift_write;
    uint32_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 3)) {
        if (cpu_access_read32(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        btr32((uint32_t *)&m_read_result, x);
        cpu_access_write32(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint32_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        btr32(ptr, x);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsf_r16r16(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = bsf16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsf_r16e16(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = bsf16(data, m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsf_r32r32(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = bsf32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsf_r32e32(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = bsf32(data, m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsr_r16r16(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = bsr16(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsr_r16e16(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = bsr16(data, m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsr_r32r32(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = bsr32(m_reg32[flags >> 8 & 15], m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bsr_r32e32(optype i)
{
    int flags = i->flags, linaddr = cpu_get_linaddr(flags, i), data;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                data = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            data = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = bsr32(data, m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cli(optype i)
{
    if ((unsigned int)m_cpl > (m_eflags >> 12 & 3)) {
        if (m_cr[4] & (1 << 0))
            m_eflags &= ~0x80000;
        else
            do {
                cpu_exception(13, (0) | 0x10000);
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            } while (0);
    } else
        m_eflags &= ~0x200;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sti(optype i)
{
    if ((unsigned int)m_cpl > (m_eflags >> 12 & 3)) {
        if (m_cr[4] & (1 << 0) && !(m_eflags & 0x100000))
            m_eflags |= 0x80000;
        else
            do {
                cpu_exception(13, (0) | 0x10000);
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            } while (0);
    } else
        m_eflags |= 0x200;
    interrupt_guard();
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cld(optype i)
{
    m_eflags &= ~0x400;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_std(optype i)
{
    m_eflags |= 0x400;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cmc(optype i)
{
    cpu_set_cf(cpu_get_cf() ^ 1);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_clc(optype i)
{
    cpu_set_cf(0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_stc(optype i)
{
    cpu_set_cf(1);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_hlt(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_cycles += cpu_get_cycles() - m_cycles;
    m_cycles_to_run      = 1;
    m_cycle_offset       = 1;
    m_refill_counter     = 0;
    m_interrupts_blocked = 0;
    m_exit_reason        = 3;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cpuid(optype i)
{
    cpuid();
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_rdmsr(optype i)
{
    if (rdmsr(m_reg32[1], &m_reg32[2], &m_reg32[0]))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_wrmsr(optype i)
{
    if (wrmsr(m_reg32[1], m_reg32[2], m_reg32[0]))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_rdtsc(optype i)
{
    if (!(m_cr[4] & (1 << 2)) || (m_cpl == 0) || !(m_cr[0] & 1)) {
        uint64_t tsc = cpu_get_cycles() - m_tsc_fudge;
        m_reg32[0]   = tsc;
        m_reg32[2]   = tsc >> 32;
    } else
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pushf(optype i)
{
    if (pushf())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_pushfd(optype i)
{
    if (pushfd())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_popf(optype i)
{
    if (popf())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_popfd(optype i)
{
    if (popfd())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cbw(optype i)
{
    m_reg16[0] = (int8_t)m_reg8[0];
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cwde(optype i)
{
    m_reg32[0] = (int16_t)m_reg16[0];
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cwd(optype i)
{
    m_reg16[4] = (int16_t)m_reg16[0] >> 15;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_cdq(optype i)
{
    m_reg32[2] = (int32_t)m_reg32[0] >> 31;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lahf(optype i)
{
    m_reg8[1] = cpu_get_eflags();
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sahf(optype i)
{
    cpu_set_eflags(m_reg8[1] | (cpu_get_eflags() & ~0xFF));
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_enter16(optype i)
{
    uint16_t alloc_size    = i->imm16;
    uint8_t  nesting_level = i->disp8;
    uint32_t frame_temp, ebp, res;
    nesting_level &= 0x1F;
    if (cpu_push16((m_reg16[10])))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    frame_temp = m_reg16[8];
    if (nesting_level != 0) {
        ebp = m_reg32[5];
        while (nesting_level > 1) {
            ebp = ((ebp - 2) & m_esp_mask) | (m_reg32[5] & ~m_esp_mask);
            res = 0;
            do {
                uint32_t addr_ = m_reg16[10] + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 1)) {
                    if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        res = m_read_result;
                    else
                        do {
                            m_cycles_to_run++;
                            return cpu_get_trace();
                        } while (0);
                } else
                    res = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            if (cpu_push16((res)))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            ;
            nesting_level--;
        }
        if (cpu_push16((frame_temp)))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        ;
    }
    m_reg16[10] = frame_temp;
    m_reg16[8] -= alloc_size;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_enter32(optype i)
{
    uint16_t alloc_size    = i->imm16;
    uint8_t  nesting_level = i->disp8;
    uint32_t frame_temp, ebp, res;
    nesting_level &= 0x1F;
    if (cpu_push32((m_reg32[5])))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    ;
    frame_temp = m_reg32[4];
    if (nesting_level != 0) {
        ebp = m_reg32[5];
        while (nesting_level > 1) {
            ebp = ((ebp - 4) & m_esp_mask) | (m_reg32[5] & ~m_esp_mask);
            res = 0;
            do {
                uint32_t addr_ = m_reg32[5] + m_seg_base[2], shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                if (((addr_ | tag >> shift_) & 3)) {
                    if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                        res = m_read_result;
                    else
                        do {
                            m_cycles_to_run++;
                            return cpu_get_trace();
                        } while (0);
                } else
                    res = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
            } while (0);
            if (cpu_push32((res)))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
            ;
            nesting_level--;
        }
        if (cpu_push32((frame_temp)))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        ;
    }
    m_reg32[5] = frame_temp;
    m_reg32[4] -= alloc_size;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_leave16(optype i)
{
    uint32_t ebp = m_reg32[5], ss_ebp = (ebp & m_esp_mask) + m_seg_base[2];
    do {
        uint32_t addr_ = ss_ebp, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg16[10] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg16[10] = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((ebp + 2) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_leave32(optype i)
{
    uint32_t ebp = m_reg32[5], ss_ebp = (ebp & m_esp_mask) + m_seg_base[2];
    do {
        uint32_t addr_ = ss_ebp, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                m_reg32[5] = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            m_reg32[5] = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[4] = ((ebp + 4) & m_esp_mask) | (m_reg32[4] & ~m_esp_mask);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sgdt_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg_limit[7], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_write, data_ = m_seg_base[7], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sidt_e32(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg_limit[9], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_write, data_ = m_seg_base[9], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_str_sldt_e16(optype i)
{
    if (m_cr[4] & (1 << 11) && m_cpl > 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_seg[i->imm8], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_str_sldt_r16(optype i)
{
    if (m_cr[4] & (1 << 11) && m_cpl > 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags           = i->flags;
    m_reg32[flags >> 8 & 15] = (m_seg[i->imm8] & i->disp32) | (m_reg32[flags >> 8 & 15] & ~i->disp32);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lgdt_e16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    uint16_t limit;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    base &= 0x00FFFFFF;
    m_seg_limit[7] = limit;
    m_seg_base[7]  = base;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lgdt_e32(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    uint16_t limit;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_seg_limit[7] = limit;
    m_seg_base[7]  = base;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lidt_e16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    uint16_t limit;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    base &= 0x00FFFFFF;
    m_seg_limit[9] = limit;
    m_seg_base[9]  = base;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lidt_e32(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), base;
    uint16_t limit;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                limit = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            limit = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    do {
        uint32_t addr_ = linaddr + 2, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                base = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            base = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_seg_limit[9] = limit;
    m_seg_base[9]  = base;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_smsw_r16(optype i)
{
    uint32_t flags           = i->flags;
    m_reg16[flags >> 8 & 15] = m_cr[0];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_smsw_r32(optype i)
{
    uint32_t flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_cr[0];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_smsw_e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, data_ = m_cr[0], tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (cpu_access_write16(addr_, data_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lmsw_r16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, v = (m_cr[0] & ~0xF) | (m_reg16[flags >> 8 & 15] & 0xF);
    cpu_prot_set_cr(0, v);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lmsw_e16(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), v;
    uint16_t src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_write, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_write, m_tlb_shift_write))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    v = (m_cr[0] & ~0xFFFF) | (src & 0xFFFF);
    cpu_prot_set_cr(0, v);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_invlpg_e8(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags;
    cpu_mmu_tlb_invalidate(cpu_get_linaddr(flags, i));
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_r32cr(optype i)
{
    uint32_t flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_cr[flags >> 12 & 15];
    m_phys_eip += flags & 15;
    m_last_phys_eip = m_phys_eip - 4096;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_mov_crr32(optype i)
{
    uint32_t flags = i->flags;
    cpu_prot_set_cr(flags >> 12 & 15, m_reg32[flags >> 8 & 15]);
    m_phys_eip += flags & 15;
    m_last_phys_eip = m_phys_eip - 4096;
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_mov_r32dr(optype i)
{
    uint32_t flags           = i->flags;
    m_reg32[flags >> 8 & 15] = m_dr[flags >> 12 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mov_drr32(optype i)
{
    uint32_t flags = i->flags;
    cpu_prot_set_dr(flags >> 12 & 15, m_reg32[flags >> 8 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_ltr_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tr;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                tr = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            tr = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (ltr(tr))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_ltr_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags;
    if (ltr(m_reg32[flags >> 8 & 15] & 0xFFFF))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lldt_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), ldtr;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                ldtr = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            ldtr = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (lldt(ldtr))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lldt_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags;
    if (lldt(m_reg32[flags >> 8 & 15] & 0xFFFF))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lar_r16e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = lar(op1, m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lar_r32e32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = lar(op1, m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lar_r16r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags            = i->flags;
    m_reg16[flags >> 12 & 15] = lar(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lar_r32r32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags            = i->flags;
    m_reg32[flags >> 12 & 15] = lar(m_reg32[flags >> 8 & 15] & 0xFFFF, m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lsl_r16e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = lsl(op1, m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lsl_r32e32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), op1;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                op1 = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            op1 = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = lsl(op1, m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lsl_r16r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags            = i->flags;
    m_reg16[flags >> 12 & 15] = lsl(m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_lsl_r32r32(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags            = i->flags;
    m_reg32[flags >> 12 & 15] = lsl(m_reg32[flags >> 8 & 15] & 0xFFFF, m_reg32[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arpl_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), tlb_shift = m_tlb_tags[linaddr >> 12],
             shift = m_tlb_shift_write;
    uint16_t *ptr;
    if (((linaddr | tlb_shift >> shift) & 1)) {
        if (cpu_access_read16(linaddr, tlb_shift >> shift, shift))
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        arpl((uint16_t *)&m_read_result, m_reg16[flags >> 12 & 15]);
        cpu_access_write16(linaddr, m_read_result, tlb_shift >> shift, shift);
    } else {
        ptr = (uint16_t *)((uint8_t *)m_tlb[linaddr >> 12] + linaddr);
        arpl(ptr, m_reg16[flags >> 12 & 15]);
    }
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_arpl_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    int flags = i->flags;
    arpl(&m_reg16[flags >> 8 & 15], m_reg16[flags >> 12 & 15]);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_verr_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), temp;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            temp = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    verify_segment_access(temp, 0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_verr_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags;
    verify_segment_access(m_reg16[flags >> 8 & 15], 0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_verw_e16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), temp;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                temp = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            temp = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    verify_segment_access(temp, 1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_verw_r16(optype i)
{
    if ((m_cr[0] & 1) == 0 || m_eflags & 0x20000)
        do {
            cpu_exception(6, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    uint32_t flags = i->flags;
    verify_segment_access(m_reg16[flags >> 8 & 15], 1);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_clts(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_cr[0] &= ~8;
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_wbinvd(optype i)
{
    if (m_cpl != 0)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_prefetchh(optype i)
{
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movzx_r16r8(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = m_reg8[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movzx_r32r8(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = m_reg8[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movzx_r32r16(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = m_reg32[flags >> 8 & 15] & 0xFFFF;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movzx_r16e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src = 0;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movzx_r32e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src = 0;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movzx_r32e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src = 0;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsx_r16r8(optype i)
{
    int flags                 = i->flags;
    m_reg16[flags >> 12 & 15] = (int8_t)m_reg8[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsx_r32r8(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = (int8_t)m_reg8[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsx_r32r16(optype i)
{
    int flags                 = i->flags;
    m_reg32[flags >> 12 & 15] = (int16_t)m_reg32[flags >> 8 & 15];
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsx_r16e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg16[flags >> 12 & 15] = (int8_t)src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsx_r32e8(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = (int8_t)src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsx_r32e16(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), src;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 1)) {
            if (!cpu_access_read16(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint16_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg32[flags >> 12 & 15] = (int16_t)src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xlat16(optype i)
{
    uint32_t flags = i->flags, linaddr = (m_reg16[6] + m_reg8[0]) & 0xFFFF, src;
    do {
        uint32_t addr_ = linaddr + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg8[0] = src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_xlat32(optype i)
{
    uint32_t flags = i->flags, linaddr = m_reg32[3] + m_reg8[0], src;
    do {
        uint32_t addr_ = linaddr + m_seg_base[flags >> 22 & 7], shift_ = m_tlb_shift_read,
                 tag = m_tlb_tags[addr_ >> 12];
        if ((tag >> shift_ & 1)) {
            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                src = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            src = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    m_reg8[0] = src;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bswap_r16(optype i)
{
    int flags                = i->flags;
    m_reg16[flags >> 8 & 15] = 0;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_bswap_r32(optype i)
{
    int      flags           = i->flags;
    uint32_t reg             = m_reg32[flags >> 8 & 15];
    reg                      = (reg & 0xFF) << 24 | (reg & 0xFF00) << 8 | (reg & 0xFF0000) >> 8 | reg >> 24;
    m_reg32[flags >> 8 & 15] = reg;
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_fpu_mem(optype i)
{
    int flags = i->flags;
    if (fpu_mem_op(i, cpu_get_virtaddr(flags, i), flags >> 22 & 7))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_fpu_reg(optype i)
{
    int flags = i->flags;
    if (fpu_reg_op(i, i->flags))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_fwait(optype i)
{
    if (m_cr[0] & (4 | 8))
        do {
            cpu_exception(7, 0);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    if (fpu_fwait())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sysenter(optype i)
{
    (void)(i);

    if (f_sysenter())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_sysexit(optype i)
{
    (void)(i);
    if (sysexit())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        do {
        } while (0);
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_sse_10_17(optype i)
{
    if (execute_0F10_17(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_28_2F(optype i)
{
    if (execute_0F28_2F(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_38(optype i)
{
    if (execute_0F38(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_6638(optype i)
{
    if (execute_660F38(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_50_57(optype i)
{
    if (execute_0F50_57(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_58_5F(optype i)
{
    if (execute_0F58_5F(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_60_67(optype i)
{
    if (execute_0F60_67(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_68_6F(optype i)
{
    if (execute_0F68_6F(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_70_76(optype i)
{
    if (execute_0F70_76(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_7C_7D(optype i)
{
    if (execute_0F7C_7D(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_7E_7F(optype i)
{
    if (execute_0F7E_7F(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_C2_C6(optype i)
{
    if (execute_0FC2_C6(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_D0_D7(optype i)
{
    if (execute_0FD0_D7(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_D8_DF(optype i)
{
    if (execute_0FD8_DF(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_E0_E7(optype i)
{
    if (execute_0FE0_E7(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_E8_EF(optype i)
{
    if (execute_0FE8_EF(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_F1_F7(optype i)
{
    if (execute_0FF1_F7(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_sse_F8_FE(optype i)
{
    if (execute_0FF8_FE(i))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_ldmxcsr(optype i)
{
    if (cpu_sse_exception())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i), mxcsr;
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (!cpu_access_read32(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                mxcsr = m_read_result;
            else
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            mxcsr = *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
    } while (0);
    if (mxcsr & ~0xFFFF)
        do {
            cpu_exception(13, (0) | 0x10000);
            do {
                m_cycles_to_run++;
                return cpu_get_trace();
            } while (0);
        } while (0);
    m_mxcsr = mxcsr;
    cpu_update_mxcsr();
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_stmxcsr(optype i)
{
    if (cpu_sse_exception())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    do {
        uint32_t addr_ = linaddr, shift_ = m_tlb_shift_read, data_ = m_mxcsr, tag = m_tlb_tags[addr_ >> 12];
        if (((addr_ | tag >> shift_) & 3)) {
            if (cpu_access_write32(addr_, data_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                do {
                    m_cycles_to_run++;
                    return cpu_get_trace();
                } while (0);
        } else
            *(uint32_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_) = data_;
    } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_mfence(optype i)
{
    do {
        m_phys_eip += i->flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_fxsave(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    if (fpu_fxsave(linaddr))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_fxrstor(optype i)
{
    uint32_t flags = i->flags, linaddr = cpu_get_linaddr(flags, i);
    if (fpu_fxrstor(linaddr))
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += flags & 15;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_emms(optype i)
{
    if (cpu_emms())
        do {
            m_cycles_to_run++;
            return cpu_get_trace();
        } while (0);
    do {
        m_phys_eip += i->flags;
        do {
        } while (0);
        return i + 1;
    } while (1);
}
optype CPU::op_movsb16(optype i)
{
    int flags = i->flags, result = movsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_movsb32(optype i)
{
    int flags = i->flags, result = movsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_movsw16(optype i)
{
    int flags = i->flags, result = movsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_movsw32(optype i)
{
    int flags = i->flags, result = movsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_movsd16(optype i)
{
    int flags = i->flags, result = movsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_movsd32(optype i)
{
    int flags = i->flags, result = movsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_stosb16(optype i)
{
    int flags = i->flags, result = stosb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_stosb32(optype i)
{
    int flags = i->flags, result = stosb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_stosw16(optype i)
{
    int flags = i->flags, result = stosw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_stosw32(optype i)
{
    int flags = i->flags, result = stosw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_stosd16(optype i)
{
    int flags = i->flags, result = stosd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_stosd32(optype i)
{
    int flags = i->flags, result = stosd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_scasb16(optype i)
{
    int flags = i->flags, result = scasb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_scasb32(optype i)
{
    int flags = i->flags, result = scasb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_scasw16(optype i)
{
    int flags = i->flags, result = scasw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_scasw32(optype i)
{
    int flags = i->flags, result = scasw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_scasd16(optype i)
{
    int flags = i->flags, result = scasd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_scasd32(optype i)
{
    int flags = i->flags, result = scasd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_insb16(optype i)
{
    int flags = i->flags, result = insb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_insb32(optype i)
{
    int flags = i->flags, result = insb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_insw16(optype i)
{
    int flags = i->flags, result = insw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_insw32(optype i)
{
    int flags = i->flags, result = insw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_insd16(optype i)
{
    int flags = i->flags, result = insd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_insd32(optype i)
{
    int flags = i->flags, result = insd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_outsb16(optype i)
{
    int flags = i->flags, result = outsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_outsb32(optype i)
{
    int flags = i->flags, result = outsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_outsw16(optype i)
{
    int flags = i->flags, result = outsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_outsw32(optype i)
{
    int flags = i->flags, result = outsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_outsd16(optype i)
{
    int flags = i->flags, result = outsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_outsd32(optype i)
{
    int flags = i->flags, result = outsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_cmpsb16(optype i)
{
    int flags = i->flags, result = cmpsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_cmpsb32(optype i)
{
    int flags = i->flags, result = cmpsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_cmpsw16(optype i)
{
    int flags = i->flags, result = cmpsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_cmpsw32(optype i)
{
    int flags = i->flags, result = cmpsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_cmpsd16(optype i)
{
    int flags = i->flags, result = cmpsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_cmpsd32(optype i)
{
    int flags = i->flags, result = cmpsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_lodsb16(optype i)
{
    int flags = i->flags, result = lodsb16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_lodsb32(optype i)
{
    int flags = i->flags, result = lodsb32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_lodsw16(optype i)
{
    int flags = i->flags, result = lodsw16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_lodsw32(optype i)
{
    int flags = i->flags, result = lodsw32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_lodsd16(optype i)
{
    int flags = i->flags, result = lodsd16(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
optype CPU::op_lodsd32(optype i)
{
    int flags = i->flags, result = lodsd32(flags);
    if (result == 0)
        do {
            m_phys_eip += flags & 15;
            do {
            } while (0);
            return i + 1;
        } while (1);
    do {
        m_cycles_to_run++;
        return cpu_get_trace();
    } while (0);
}
