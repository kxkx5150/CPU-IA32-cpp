#include "cpu.h"
#include <cstdint>

extern CPU cpu;
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))


uint32_t CPU::rw(void)
{
    uint32_t val = m_rawp[0] | m_rawp[1] << 8;
    m_rawp += 2;
    return val;
}
uint32_t CPU::rd(void)
{
    uint32_t val = m_rawp[0] | m_rawp[1] << 8 | m_rawp[2] << 16 | m_rawp[3] << 24;
    m_rawp += 4;
    return val;
}
uint32_t CPU::rv(void)
{
    if (m_d_state_hash & 0x0001)
        return rw();
    else
        return rd();
}
uint32_t CPU::rvs(void)
{
    if (m_d_state_hash & 0x0001)
        return (int16_t)rw();
    else
        return rd();
}
int CPU::find_instruction_length(int max_bytes)
{
    int            opcode, m_d_state_hash = m_state_hash, initial_max_bytes = max_bytes, opcode_info;
    const uint8_t *tbl = optable;
top:
    opcode = m_rawp[initial_max_bytes - max_bytes];
    if (max_bytes-- < 0)
        return -1;
    switch (opcode) {
        case 0x0F:
            tbl    = optable0F;
            opcode = m_rawp[initial_max_bytes - max_bytes];
            if (max_bytes-- < 0)
                return -1;
            goto done;
        case 0x66:
            if (!((m_d_state_hash ^ m_state_hash) & 0x0001))
                m_d_state_hash ^= 0x0001;
            break;
        case 0x67:
            if (!((m_d_state_hash ^ m_state_hash) & 0x0002))
                m_d_state_hash ^= 0x0002;
            break;
        case 0x26:
        case 0x2E:
        case 0x36:
        case 0x3E:
        case 0x64:
        case 0x65:
        case 0xF0:
        case 0xF2:
        case 0xF3:
            break;
        default:
            goto done;
    }
    goto top;
done:
    opcode_info = tbl[opcode];
    switch (opcode_info & 15) {
        case opcode_singlebyte:
            break;
        case opcode_special:
            do {
                util_abort();
                abort();
            } while (0);
        case opcode_modrm: {
            int modrm, sib;
            modrm = m_rawp[initial_max_bytes - max_bytes];
            if (max_bytes-- < 0)
                return -1;
            if (tbl == optable && ((opcode & 0xFE) == 0xF6)) {
                if (!(modrm >> 3 & 6)) {
                    if (opcode & 1)
                        opcode_info |= 0x20;
                    else
                        opcode_info |= 0x10;
                }
            }
            if (modrm < 0xC0) {
                if (m_d_state_hash & 0x0002) {
                    switch (modrm >> 6 & 3) {
                        case 0:
                            if ((modrm & 7) == 6)
                                max_bytes -= 2;
                            break;
                        case 1:
                            max_bytes--;
                            break;
                        case 2:
                            max_bytes -= 2;
                            break;
                    }
                } else {
                    switch ((modrm >> 3 & 0x18) | (modrm & 7)) {
                        case 4: {
                            sib = m_rawp[initial_max_bytes - max_bytes];
                            if (max_bytes-- < 0)
                                return -1;
                            if ((sib & 7) == 5)
                                max_bytes -= 4;
                            break;
                        }
                        case 0x0C:
                            max_bytes -= 2;
                            break;
                        case 0x14:
                            max_bytes -= 5;
                            break;
                        case 5:
                        case 16 ... 19:
                        case 21 ... 23:
                            max_bytes -= 4;
                            break;
                        case 0 ... 3:
                        case 6 ... 7:
                        case 24 ... 31:
                            break;
                        case 8 ... 0x0B:
                        case 0x0D ... 0x0F:
                            max_bytes--;
                            break;
                    }
                }
            }
            break;
        }
        case opcode_moffs:
            if (m_d_state_hash & 0x0002)
                max_bytes -= 2;
            else
                max_bytes -= 4;
            break;
        case opcode_invalid:
            break;
    }
    if (opcode_info & 0x10)
        max_bytes -= 1;
    if (opcode_info & 0x40)
        max_bytes -= 2;
    if (opcode_info & 0x20)
        max_bytes -= m_d_state_hash & 0x0001 ? 2 : 4;
    if (max_bytes < 0)
        return -1;
    return initial_max_bytes - max_bytes;
}
int CPU::parse_modrm(struct decoded_instruction *i, uint8_t modrm, int is8)
{
    int addr16 = m_d_state_hash >> 1 & 1, flags = addr16 << 4, rm = modrm & 7, new_modrm = rm | ((modrm & 0xC0) >> 3);
    switch (is8 & 3) {
        case 0:
        case 3:
            if (m_d_state_hash & 0x0001)
                flags |= ((modrm >> 3 & 7) << 1) << 12;
            else
                flags |= (modrm >> 3 & 7) << 12;
            break;
        case 1:
            flags |= (((modrm >> 3 & 7) & 3) << 2 | (modrm >> 3 & 7) >> 2) << 12;
            ;
            break;
        case 2:
            flags |= (modrm >> 3 & 7) << 12;
            break;
    }
    if (addr16) {
        switch (new_modrm) {
            case 0 ... 5:
            case 7:
                flags |= (addr16_lut[rm]) << 8;
                flags |= (addr16_lut[rm | 8]) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[addr16_lut[rm | 16]]) << 22;
                i->disp32 = 0;
                break;
            case 6:
                flags |= (8) << 8;
                flags |= (8) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[0]) << 22;
                i->disp32 = rw();
                break;
            case 8 ... 15:
                flags |= (addr16_lut2[rm]) << 8;
                flags |= (addr16_lut2[rm | 8]) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[addr16_lut2[rm | 16]]) << 22;
                i->disp32 = (int8_t)*m_rawp++;
                break;
            case 16 ... 23:
                flags |= (addr16_lut2[rm]) << 8;
                flags |= (addr16_lut2[rm | 8]) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[addr16_lut2[rm | 16]]) << 22;
                i->disp32 = rw();
                break;
            case 24 ... 31:
                if (is8 & 4) {
                    flags |= (modrm & 7) << 8;
                } else if (is8 & 1) {
                    flags |= (((modrm & 7) & 3) << 2 | (modrm & 7) >> 2) << 8;
                    ;
                } else {
                    if (m_d_state_hash & 0x0001)
                        flags |= ((modrm & 7) << 1) << 8;
                    else
                        flags |= (modrm & 7) << 8;
                }
                break;
        }
    } else {
        int sib, index, base;
        switch (new_modrm) {
            case 0 ... 3:
            case 6 ... 7:
                flags |= (rm) << 8;
                flags |= (8) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[addr32_lut[rm]]) << 22;
                i->disp32 = 0;
                break;
            case 4:
                sib   = *m_rawp++;
                index = sib >> 3 & 7;
                base  = sib & 7;
                if (base == 5) {
                    base = 0;
                    flags |= (8) << 8;
                    i->disp32 = rd();
                } else {
                    flags |= (base) << 8;
                    i->disp32 = 0;
                }
                if (index != 4) {
                    flags |= (index) << 16;
                    flags |= (sib >> 6) << 20;
                } else
                    flags |= (8) << 16;
                flags |= (m_seg_prefix[addr32_lut2[base]]) << 22;
                break;
            case 5:
                rm = modrm & 7;
                flags |= (8) << 8;
                flags |= (8) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[0]) << 22;
                i->disp32 = rd();
                break;
            case 0x08 ... 0x0B:
            case 0x0D ... 0x0F:
                rm = modrm & 7;
                flags |= (rm) << 8;
                flags |= (8) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[addr32_lut[rm]]) << 22;
                i->disp32 = (int8_t)*m_rawp++;
                break;
            case 0x0C:
                sib   = *m_rawp++;
                index = sib >> 3 & 7;
                base  = sib & 7;
                flags |= (base) << 8;
                if (index != 4) {
                    flags |= (index) << 16;
                    flags |= (sib >> 6) << 20;
                } else
                    flags |= (8) << 16;
                flags |= (m_seg_prefix[addr32_lut2[base]]) << 22;
                i->disp32 = (int8_t)*m_rawp++;
                break;
            case 0x10 ... 0x13:
            case 0x15 ... 0x17:
                rm = modrm & 7;
                flags |= (rm) << 8;
                flags |= (8) << 16;
                flags |= (0) << 20;
                flags |= (m_seg_prefix[addr32_lut[rm]]) << 22;
                i->disp32 = rd();
                break;
            case 0x14:
                sib   = *m_rawp++;
                index = sib >> 3 & 7;
                base  = sib & 7;
                flags |= (base) << 8;
                if (index != 4) {
                    flags |= (index) << 16;
                    flags |= (sib >> 6) << 20;
                } else
                    flags |= (8) << 16;
                flags |= (m_seg_prefix[addr32_lut2[base]]) << 22;
                i->disp32 = rd();
                break;
            case 24 ... 31:
                if (is8 & 4) {
                    flags |= (modrm & 7) << 8;
                } else if (is8 & 1) {
                    flags |= (((modrm & 7) & 3) << 2 | (modrm & 7) >> 2) << 8;
                    ;
                } else {
                    if (m_d_state_hash & 0x0001)
                        flags |= ((modrm & 7) << 1) << 8;
                    else
                        flags |= (modrm & 7) << 8;
                }
                break;
        }
    }
    return flags;
}
int CPU::swap_rm_reg(int flags)
{
    unsigned int x = 15 & ((flags >> 8) ^ (flags >> 12));
    return flags ^ ((x << 8) | (x << 12));
}
int CPU::decode_invalid(struct decoded_instruction *i)
{
    (void)(i);
    m_rawp--;
    for (int i = 0; i < 16; i++)
        printf("%02x ", m_rawp[i - 16]);
    printf("\n");
    for (int i = 0; i < 16; i++)
        printf("%02x ", m_rawp[i]);
    printf("\n");
    i->handler = &CPU::op_ud_exception;
    i->flags   = 0;
    return 1;
}
int CPU::decode_invalid0F(struct decoded_instruction *i)
{
    (void)(i);
    m_rawp--;
    for (int i = 0; i < 16; i++)
        printf("%02x ", m_rawp[i - 16]);
    printf("\n");
    for (int i = 0; i < 16; i++)
        printf("%02x ", m_rawp[i]);
    printf("\n");
    i->handler = &CPU::op_ud_exception;
    i->flags   = 0;
    return 1;
}
int CPU::decode_0F(struct decoded_instruction *i)
{

    return CALL_MEMBER_FN(*this, table0F[*m_rawp++])(i);
}
int CPU::decode_prefix(struct decoded_instruction *i)
{
    uint8_t prefix     = m_rawp[-1];
    int     prefix_set = 0, return_value = 0;
    i->flags = 0;
    while (1) {
        switch (prefix) {
            case 0xF3:
                m_sse_prefix = SSE_PREFIX_F3;
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                i->flags |= (1 << 6);
                prefix_set |= 1;
                m_d_state_hash |= 4;
                break;
            case 0xF2:
                m_sse_prefix = SSE_PREFIX_F2;
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                i->flags |= (2 << 6);
                prefix_set |= 1;
                m_d_state_hash |= 4;
                break;
            case 0x66:
                m_sse_prefix = SSE_PREFIX_66;
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                if (!(prefix_set & 2))
                    m_d_state_hash ^= 0x0001;
                prefix_set |= 2;
                break;
            case 0x67:
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                if (!(prefix_set & 4))
                    m_d_state_hash ^= 0x0002;
                prefix_set |= 4;
                break;
            case 0xF0:
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                prefix_set |= 8;
                break;
            case 0x26:
            case 0x2E:
            case 0x36:
            case 0x3E:
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                m_seg_prefix[0] = m_seg_prefix[1] = prefix >> 3 & 3;
                prefix_set |= 16;
                break;
            case 0x64:
            case 0x65:
                if (!prefix_set)
                    if (find_instruction_length(15) == -1)
                        goto error;
                m_seg_prefix[0] = m_seg_prefix[1] = 4 + (prefix & 1);
                prefix_set |= 16;
                break;
            case 0x0F:
                prefix_set |= 32;
                m_d_state_hash |= 4;
                return_value = CALL_MEMBER_FN(*this, table0F[prefix = *m_rawp++])(i);

                goto done;
            default:
                m_d_state_hash |= 4;
                return_value = CALL_MEMBER_FN(*this, table[prefix])(i);
                goto done;
        }
        prefix = *m_rawp++;
    }
done:
    if (prefix_set > 0) {
        if (prefix_set & 8) {
            int valid;
            if (prefix_set & 32)
                valid = optable0F[prefix] & 0x80;
            else
                valid = optable[prefix] & 0x80;
            if (!valid)
                goto error;
        }
        m_seg_prefix[0] = 3;
        m_seg_prefix[1] = 2;
        m_d_state_hash  = m_state_hash;
    }
    m_sse_prefix = 0;
    return return_value;
error:
    m_sse_prefix = 0;
    i->handler   = &CPU::op_ud_exception;
    return 1;
}
int CPU::decode_jcc8(struct decoded_instruction *i)
{
    i->flags   = 0;
    int cond   = m_rawp[-1] & 15;
    i->handler = m_d_state_hash & 0x0001 ? jcc16[cond] : jcc32[cond];
    i->imm32   = (int8_t)*m_rawp++;
    return 0;
}
int CPU::decode_jccv(struct decoded_instruction *i)
{
    i->flags   = 0;
    int cond   = m_rawp[-1] & 15;
    i->handler = m_d_state_hash & 0x0001 ? jcc16[cond] : jcc32[cond];
    i->imm32   = rvs();
    return 0;
}
int CPU::decode_cmov(struct decoded_instruction *i)
{
    uint8_t cond = m_rawp[-1] & 15, modrm = *m_rawp++;
    i->flags = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cmov_r16e16 : &CPU::op_cmov_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cmov_r16r16 : &CPU::op_cmov_r32r32;
    i->flags |= (cond) << 25;
    return 0;
}
int CPU::decode_setcc(struct decoded_instruction *i)
{
    uint8_t cond = m_rawp[-1] & 15, modrm = *m_rawp++;
    i->flags = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        i->handler = &CPU::op_setcc_e8;
    else
        i->handler = &CPU::op_setcc_r8;
    i->flags |= (cond) << 25;
    return 0;
}
int CPU::decode_mov_rbib(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (((m_rawp[-1] & 7) & 3) << 2 | (m_rawp[-1] & 7) >> 2) << 8;
    ;
    i->flags   = flags;
    i->handler = &CPU::op_mov_r8i8;
    i->imm32   = *m_rawp++;
    return 0;
}
int CPU::decode_mov_rviv(struct decoded_instruction *i)
{
    int flags = 0;
    if (m_d_state_hash & 0x0001)
        flags |= ((m_rawp[-1] & 7) << 1) << 8;
    else
        flags |= (m_rawp[-1] & 7) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_r16i16 : &CPU::op_mov_r32i32;
    i->imm32   = rv();
    return 0;
}
int CPU::decode_push_rv(struct decoded_instruction *i)
{
    int flags = 0;
    if (m_d_state_hash & 0x0001)
        flags |= ((m_rawp[-1] & 7) << 1) << 8;
    else
        flags |= (m_rawp[-1] & 7) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_r16 : &CPU::op_push_r32;
    return 0;
}
int CPU::decode_pop_rv(struct decoded_instruction *i)
{
    int flags = 0;
    if (m_d_state_hash & 0x0001)
        flags |= ((m_rawp[-1] & 7) << 1) << 8;
    else
        flags |= (m_rawp[-1] & 7) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pop_r16 : &CPU::op_pop_r32;
    return 0;
}
int CPU::decode_push_sv(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (m_rawp[-1] >> 3 & 3) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_s16 : &CPU::op_push_s32;
    return 0;
}
int CPU::decode_pop_sv(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (m_rawp[-1] >> 3 & 3) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pop_s16 : &CPU::op_pop_s32;
    return 0;
}
int CPU::decode_inc_rv(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_inc_r16 : &CPU::op_inc_r32;
    i->flags   = 0;
    if (m_d_state_hash & 0x0001)
        i->flags |= ((m_rawp[-1] & 7) << 1) << 8;
    else
        i->flags |= (m_rawp[-1] & 7) << 8;
    return 0;
}
int CPU::decode_dec_rv(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_dec_r16 : &CPU::op_dec_r32;
    i->flags   = 0;
    if (m_d_state_hash & 0x0001)
        i->flags |= ((m_rawp[-1] & 7) << 1) << 8;
    else
        i->flags |= (m_rawp[-1] & 7) << 8;
    return 0;
}
int CPU::decode_fpu(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1], modrm = *m_rawp++;
    if (modrm < 0xC0) {
        i->flags = parse_modrm(i, modrm, 2);
        i->flags |= (m_d_state_hash & 1) << 25;
        i->handler = &CPU::op_fpu_mem;
    } else {
        int flags = 0;
        flags |= (modrm >> 3 & 7) << 12;
        i->flags = flags;
        i->flags |= (m_d_state_hash & 1) << 25;
        i->handler = &CPU::op_fpu_reg;
    }
    i->imm32 = (opcode << 8 & 0x700) | modrm;
    return 0;
}
int CPU::decode_arith_00(struct decoded_instruction *i)
{
    int     op    = m_rawp[-1] >> 3 & 7;
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 1);
    flags |= (op) << 25;
    i->flags   = flags;
    i->handler = modrm < 0xC0 ? &CPU::op_arith_e8r8 : &CPU::op_arith_r8r8;
    return 0;
}
int CPU::decode_arith_01(struct decoded_instruction *i)
{
    int     op    = m_rawp[-1] >> 3 & 7;
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    flags |= (op) << 25;
    i->flags   = flags;
    i->handler = modrm < 0xC0              ? m_d_state_hash & 0x0001 ? &CPU::op_arith_e16r16 : &CPU::op_arith_e32r32
                 : m_d_state_hash & 0x0001 ? &CPU::op_arith_r16r16
                                           : &CPU::op_arith_r32r32;
    return 0;
}
int CPU::decode_arith_02(struct decoded_instruction *i)
{
    int     op    = m_rawp[-1] >> 3 & 7;
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 1);
    flags |= (op) << 25;
    if (modrm < 0xC0) {
        i->flags   = flags;
        i->handler = &CPU::op_arith_r8e8;
    } else {
        i->flags   = swap_rm_reg(flags);
        i->handler = &CPU::op_arith_r8r8;
    }
    return 0;
}
int CPU::decode_arith_03(struct decoded_instruction *i)
{
    int     op    = m_rawp[-1] >> 3 & 7;
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    flags |= (op) << 25;
    if (modrm < 0xC0) {
        i->flags   = flags;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_arith_r16e16 : &CPU::op_arith_r32e32;
    } else {
        i->flags   = swap_rm_reg(flags);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_arith_r16r16 : &CPU::op_arith_r32r32;
    }
    return 0;
}
int CPU::decode_arith_04(struct decoded_instruction *i)
{
    i->flags = 0;
    i->flags |= (m_rawp[-1] >> 3 & 7) << 25;
    i->handler = &CPU::op_arith_r8i8;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_arith_05(struct decoded_instruction *i)
{
    i->flags = 0;
    i->flags |= (m_rawp[-1] >> 3 & 7) << 25;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_arith_r16i16 : &CPU::op_arith_r32i32;
    i->imm32   = rv();
    return 0;
}
int CPU::decode_xchg(struct decoded_instruction *i)
{
    i->flags = 0;
    if (m_d_state_hash & 0x0001)
        i->flags |= ((m_rawp[-1] & 7) << 1) << 12;
    else
        i->flags |= (m_rawp[-1] & 7) << 12;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_xchg_r16r16 : &CPU::op_xchg_r32r32;
    return 0;
}
int CPU::decode_bswap(struct decoded_instruction *i)
{
    i->flags = 0;
    if (m_d_state_hash & 0x0001)
        i->flags |= ((m_rawp[-1] & 7) << 1) << 8;
    else
        i->flags |= (m_rawp[-1] & 7) << 8;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bswap_r16 : &CPU::op_bswap_r32;
    return 0;
}
int CPU::decode_ud(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_ud_exception;
    return 1;
}
int CPU::decode_27(struct decoded_instruction *i)
{
    i->handler = &CPU::op_daa;
    i->flags   = 0;
    return 0;
}
int CPU::decode_2F(struct decoded_instruction *i)
{
    i->handler = &CPU::op_das;
    i->flags   = 0;
    return 0;
}
int CPU::decode_37(struct decoded_instruction *i)
{
    i->handler = &CPU::op_aaa;
    i->flags   = 0;
    return 0;
}
int CPU::decode_3F(struct decoded_instruction *i)
{
    i->handler = &CPU::op_aas;
    i->flags   = 0;
    return 0;
}
int CPU::decode_38(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 1);
    i->flags      = flags;
    i->handler    = modrm < 0xC0 ? &CPU::op_cmp_e8r8 : &CPU::op_cmp_r8r8;
    return 0;
}
int CPU::decode_39(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    i->flags      = flags;
    i->handler    = modrm < 0xC0              ? m_d_state_hash & 0x0001 ? &CPU::op_cmp_e16r16 : &CPU::op_cmp_e32r32
                    : m_d_state_hash & 0x0001 ? &CPU::op_cmp_r16r16
                                              : &CPU::op_cmp_r32r32;
    return 0;
}
int CPU::decode_3A(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0) {
        i->flags   = flags;
        i->handler = &CPU::op_cmp_r8e8;
    } else {
        i->flags   = swap_rm_reg(flags);
        i->handler = &CPU::op_cmp_r8r8;
    }
    return 0;
}
int CPU::decode_3B(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0) {
        i->flags   = flags;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cmp_r16e16 : &CPU::op_cmp_r32e32;
    } else {
        i->flags   = swap_rm_reg(flags);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cmp_r16r16 : &CPU::op_cmp_r32r32;
    }
    return 0;
}
int CPU::decode_3C(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_cmp_r8i8;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_3D(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cmp_r16i16 : &CPU::op_cmp_r32i32;
    i->imm32   = rv();
    return 0;
}
int CPU::decode_60(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pusha : &CPU::op_pushad;
    return 0;
}
int CPU::decode_61(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_popa : &CPU::op_popad;
    return 0;
}
int CPU::decode_62(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    }
    i->flags   = parse_modrm(i, modrm, 0);
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bound_r16e16 : &CPU::op_bound_r32e32;
    return 0;
}
int CPU::decode_63(struct decoded_instruction *i)
{
    uint8_t modrm              = *m_rawp++;
    int     old_m_d_state_hash = m_state_hash;
    m_d_state_hash |= 0x0001;
    i->flags       = parse_modrm(i, modrm, 0);
    m_d_state_hash = old_m_d_state_hash;
    if (modrm < 0xC0)
        i->handler = &CPU::op_arpl_e16;
    else
        i->handler = &CPU::op_arpl_r16;
    return 0;
}
int CPU::decode_68(struct decoded_instruction *i)
{
    i->imm32   = rv();
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_i16 : &CPU::op_push_i32;
    i->flags   = 0;
    return 0;
}
int CPU::decode_69(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->handler    = modrm < 0xC0 ? m_d_state_hash & 0x0001 ? &CPU::op_imul_r16e16i16 : &CPU::op_imul_r32e32i32
                    : m_d_state_hash & 0x0001 ? &CPU::op_imul_r16r16i16
                                              : &CPU::op_imul_r32r32i32;
    i->imm32      = rvs();
    return 0;
}
int CPU::decode_6A(struct decoded_instruction *i)
{
    i->imm32   = (int8_t)*m_rawp++;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_i16 : &CPU::op_push_i32;
    i->flags   = 0;
    return 0;
}
int CPU::decode_6B(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->handler    = modrm < 0xC0 ? m_d_state_hash & 0x0001 ? &CPU::op_imul_r16e16i16 : &CPU::op_imul_r32e32i32
                    : m_d_state_hash & 0x0001 ? &CPU::op_imul_r16r16i16
                                              : &CPU::op_imul_r32r32i32;
    i->imm32      = (int8_t)*m_rawp++;
    return 0;
}
int CPU::decode_6C(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_insb16 : &CPU::op_insb32;
    i->flags |= (m_seg_prefix[0]) << 22;
    return 0;
}
int CPU::decode_6D(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    static const insn_handler_t atbl[4] = {&CPU::op_insd32, &CPU::op_insw32, &CPU::op_insd16, &CPU::op_insw16};
    i->handler                          = atbl[m_d_state_hash & 3];
    return 0;
}
int CPU::decode_6E(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_outsb16 : &CPU::op_outsb32;
    return 0;
}
int CPU::decode_6F(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    static const insn_handler_t atbl[4] = {&CPU::op_outsd32, &CPU::op_outsw32, &CPU::op_outsd16, &CPU::op_outsw16};
    i->handler                          = atbl[m_d_state_hash & 3];
    return 0;
}
int CPU::decode_80(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 1);
    i->imm8       = *m_rawp++;
    if ((modrm & 0x38) == 0x38) {
        i->handler = modrm < 0xC0 ? &CPU::op_cmp_e8i8 : &CPU::op_cmp_r8i8;
    } else {
        flags |= (modrm >> 3 & 7) << 25;
        i->handler = modrm < 0xC0 ? &CPU::op_arith_e8i8 : &CPU::op_arith_r8i8;
    }
    i->flags = flags;
    return 0;
}
int CPU::decode_81(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    i->imm32      = rvs();
    if ((modrm & 0x38) == 0x38) {
        i->handler = m_d_state_hash & 0x0001 ? modrm < 0xC0 ? &CPU::op_cmp_e16i16 : &CPU::op_cmp_r16i16
                     : modrm < 0xC0          ? &CPU::op_cmp_e32i32
                                             : &CPU::op_cmp_r32i32;
    } else {
        flags |= (modrm >> 3 & 7) << 25;
        i->handler = m_d_state_hash & 0x0001 ? modrm < 0xC0 ? &CPU::op_arith_e16i16 : &CPU::op_arith_r16i16
                     : modrm < 0xC0          ? &CPU::op_arith_e32i32
                                             : &CPU::op_arith_r32i32;
    }
    i->flags = flags;
    return 0;
}
int CPU::decode_83(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    i->imm32      = (int8_t)*m_rawp++;
    if ((modrm & 0x38) == 0x38) {
        i->handler = m_d_state_hash & 0x0001 ? modrm < 0xC0 ? &CPU::op_cmp_e16i16 : &CPU::op_cmp_r16i16
                     : modrm < 0xC0          ? &CPU::op_cmp_e32i32
                                             : &CPU::op_cmp_r32i32;
    } else {
        flags |= (modrm >> 3 & 7) << 25;
        i->handler = m_d_state_hash & 0x0001 ? modrm < 0xC0 ? &CPU::op_arith_e16i16 : &CPU::op_arith_r16i16
                     : modrm < 0xC0          ? &CPU::op_arith_e32i32
                                             : &CPU::op_arith_r32i32;
    }
    i->flags = flags;
    return 0;
}
int CPU::decode_84(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        i->handler = &CPU::op_test_e8r8;
    else
        i->handler = &CPU::op_test_r8r8;
    return 0;
}
int CPU::decode_85(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_test_e16r16 : &CPU::op_test_e32r32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_test_r16r16 : &CPU::op_test_r32r32;
    return 0;
}
int CPU::decode_86(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        i->handler = &CPU::op_xchg_r8e8;
    else
        i->handler = &CPU::op_xchg_r8r8;
    return 0;
}
int CPU::decode_87(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_xchg_r16e16 : &CPU::op_xchg_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_xchg_r16r16 : &CPU::op_xchg_r32r32;
    return 0;
}
int CPU::decode_88(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        i->handler = &CPU::op_mov_e8r8;
    else
        i->handler = &CPU::op_mov_r8r8;
    return 0;
}
int CPU::decode_89(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->handler    = modrm < 0xC0              ? m_d_state_hash & 0x0001 ? &CPU::op_mov_e16r16 : &CPU::op_mov_e32r32
                    : m_d_state_hash & 0x0001 ? &CPU::op_mov_r16r16
                                              : &CPU::op_mov_r32r32;
    return 0;
}
int CPU::decode_8A(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        i->handler = &CPU::op_mov_r8e8;
    else {
        flags      = swap_rm_reg(flags);
        i->handler = &CPU::op_mov_r8r8;
    }
    i->flags = flags;
    return 0;
}
int CPU::decode_8B(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_r16e16 : &CPU::op_mov_r32e32;
    else {
        flags      = swap_rm_reg(flags);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_r16r16 : &CPU::op_mov_r32r32;
    }
    i->flags = flags;
    return 0;
}
int CPU::decode_8C(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 2);
    if (modrm < 0xC0)
        i->handler = &CPU::op_mov_e16s16;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_r16s16 : &CPU::op_mov_r32s16;
    return 0;
}
int CPU::decode_8D(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->handler = &CPU::op_ud_exception;
        i->flags   = 0;
        return 1;
    }
    i->flags   = parse_modrm(i, modrm, 0);
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lea_r16e16 : &CPU::op_lea_r32e32;
    return 0;
}
int CPU::decode_8E(struct decoded_instruction *i)
{
    uint8_t modrm              = *m_rawp++;
    int     old_m_d_state_hash = m_state_hash;
    m_d_state_hash |= 0x0001;
    i->flags       = parse_modrm(i, modrm, 2);
    m_d_state_hash = old_m_d_state_hash;
    if (modrm < 0xC0)
        i->handler = &CPU::op_mov_s16e16;
    else
        i->handler = &CPU::op_mov_s16r16;
    return 0;
}
int CPU::decode_8F(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pop_r16 : &CPU::op_pop_r32;
    } else {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pop_e16 : &CPU::op_pop_e32;
    }
    return 0;
}
int CPU::decode_90(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_nop;
    return 0;
}
int CPU::decode_98(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cbw : &CPU::op_cwde;
    return 0;
}
int CPU::decode_99(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_cwd : &CPU::op_cdq;
    return 0;
}
int CPU::decode_9A(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_callf16_ap : &CPU::op_callf32_ap;
    i->imm32   = rv();
    i->disp16  = rw();
    i->flags   = 0;
    return 1;
}
int CPU::decode_9B(struct decoded_instruction *i)
{
    i->handler = &CPU::op_fwait;
    i->flags   = 0;
    return 0;
}
int CPU::decode_9C(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pushf : &CPU::op_pushfd;
    return 0;
}
int CPU::decode_9D(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_popf : &CPU::op_popfd;
    return 0;
}
int CPU::decode_9E(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_sahf;
    return 0;
}
int CPU::decode_9F(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_lahf;
    return 0;
}
int CPU::decode_A0(struct decoded_instruction *i)
{
    i->handler = &CPU::op_mov_alm8;
    i->imm32   = m_d_state_hash & 0x0002 ? rw() : rd();
    i->flags   = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    return 0;
}
int CPU::decode_A1(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_axm16 : &CPU::op_mov_eaxm32;
    i->imm32   = m_d_state_hash & 0x0002 ? rw() : rd();
    i->flags   = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    return 0;
}
int CPU::decode_A2(struct decoded_instruction *i)
{
    i->handler = &CPU::op_mov_m8al;
    i->imm32   = m_d_state_hash & 0x0002 ? rw() : rd();
    i->flags   = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    return 0;
}
int CPU::decode_A3(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_m16ax : &CPU::op_mov_m32eax;
    i->imm32   = m_d_state_hash & 0x0002 ? rw() : rd();
    i->flags   = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    return 0;
}
int CPU::decode_A4(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_movsb16 : &CPU::op_movsb32;
    return 0;
}
int CPU::decode_A5(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    switch (m_d_state_hash & 3) {
        case 0:
            i->handler = &CPU::op_movsd32;
            break;
        case 0x0001:
            i->handler = &CPU::op_movsw32;
            break;
        case 0x0002:
            i->handler = &CPU::op_movsd16;
            break;
        case 0x0002 | 0x0001:
            i->handler = &CPU::op_movsw16;
            break;
    }
    return 0;
}
int CPU::decode_A6(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_cmpsb16 : &CPU::op_cmpsb32;
    return 0;
}
int CPU::decode_A7(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    switch (m_d_state_hash & 3) {
        case 0:
            i->handler = &CPU::op_cmpsd32;
            break;
        case 0x0001:
            i->handler = &CPU::op_cmpsw32;
            break;
        case 0x0002:
            i->handler = &CPU::op_cmpsd16;
            break;
        case 0x0002 | 0x0001:
            i->handler = &CPU::op_cmpsw16;
            break;
    }
    return 0;
}
int CPU::decode_A8(struct decoded_instruction *i)
{
    i->handler = &CPU::op_test_r8i8;
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_A9(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_test_r16i16 : &CPU::op_test_r32i32;
    i->flags   = 0;
    i->imm32   = rv();
    return 0;
}
int CPU::decode_AA(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_stosb16 : &CPU::op_stosb32;
    return 0;
}
int CPU::decode_AB(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    switch (m_d_state_hash & 3) {
        case 0:
            i->handler = &CPU::op_stosd32;
            break;
        case 0x0001:
            i->handler = &CPU::op_stosw32;
            break;
        case 0x0002:
            i->handler = &CPU::op_stosd16;
            break;
        case 0x0002 | 0x0001:
            i->handler = &CPU::op_stosw16;
            break;
    }
    return 0;
}
int CPU::decode_AC(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_lodsb16 : &CPU::op_lodsb32;
    return 0;
}
int CPU::decode_AD(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    switch (m_d_state_hash & 3) {
        case 0:
            i->handler = &CPU::op_lodsd32;
            break;
        case 0x0001:
            i->handler = &CPU::op_lodsw32;
            break;
        case 0x0002:
            i->handler = &CPU::op_lodsd16;
            break;
        case 0x0002 | 0x0001:
            i->handler = &CPU::op_lodsw16;
            break;
    }
    return 0;
}
int CPU::decode_AE(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    i->handler = m_d_state_hash & 0x0002 ? &CPU::op_scasb16 : &CPU::op_scasb32;
    return 0;
}
int CPU::decode_AF(struct decoded_instruction *i)
{
    if (!(m_d_state_hash & 4))
        i->flags = 0;
    switch (m_d_state_hash & 3) {
        case 0:
            i->handler = &CPU::op_scasd32;
            break;
        case 0x0001:
            i->handler = &CPU::op_scasw32;
            break;
        case 0x0002:
            i->handler = &CPU::op_scasd16;
            break;
        case 0x0002 | 0x0001:
            i->handler = &CPU::op_scasw16;
            break;
    }
    return 0;
}
int CPU::decode_C0(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    i->flags |= (modrm >> 3 & 7) << 25;
    if (modrm < 0xC0)
        i->handler = &CPU::op_shift_e8i8;
    else
        i->handler = &CPU::op_shift_r8i8;
    i->imm8 = *m_rawp++;
    return 0;
}
int CPU::decode_C1(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->flags |= (modrm >> 3 & 7) << 25;
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shift_e16i16 : &CPU::op_shift_e32i32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shift_r16i16 : &CPU::op_shift_r32i32;
    i->imm8 = *m_rawp++;
    return 0;
}
int CPU::decode_C2(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_ret16_iw : &CPU::op_ret32_iw;
    i->imm16   = rw();
    i->flags   = 0;
    return 1;
}
int CPU::decode_C3(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_ret16 : &CPU::op_ret32;
    i->flags   = 0;
    return 1;
}
int CPU::decode_C4(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_les_r16e16 : &CPU::op_les_r32e32;
    }
    return 0;
}
int CPU::decode_C5(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lds_r16e16 : &CPU::op_lds_r32e32;
    }
    return 0;
}
int CPU::decode_C6(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >> 3 & 7) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    }
    i->flags = parse_modrm(i, modrm, 1);
    if (modrm >= 0xC0)
        i->handler = &CPU::op_mov_r8i8;
    else
        i->handler = &CPU::op_mov_e8i8;
    i->imm8 = *m_rawp++;
    return 0;
}
int CPU::decode_C7(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >> 3 & 7) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    }
    i->flags = parse_modrm(i, modrm, 0);
    if (modrm >= 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_r16i16 : &CPU::op_mov_r32i32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_mov_e16i16 : &CPU::op_mov_e32i32;
    i->imm32 = rv();
    return 0;
}
int CPU::decode_C8(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_enter16 : &CPU::op_enter32;
    i->imm16   = rw();
    i->disp8   = *m_rawp++;
    return 0;
}
int CPU::decode_C9(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_leave16 : &CPU::op_leave32;
    return 0;
}
int CPU::decode_CA(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->imm16   = rw();
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_retf16 : &CPU::op_retf32;
    return 1;
}
int CPU::decode_CB(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->imm16   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_retf16 : &CPU::op_retf32;
    return 1;
}
int CPU::decode_CC(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->imm8    = 3;
    i->handler = &CPU::op_int;
    return 1;
}
int CPU::decode_CD(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    i->handler = &CPU::op_int;
    return 1;
}
int CPU::decode_CE(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_into;
    return 0;
}
int CPU::decode_CF(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_iret16 : &CPU::op_iret32;
    return 1;
}
int CPU::decode_D0(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    i->flags |= (modrm >> 3 & 7) << 25;
    if (modrm < 0xC0)
        i->handler = &CPU::op_shift_e8i8;
    else
        i->handler = &CPU::op_shift_r8i8;
    i->imm8 = 1;
    return 0;
}
int CPU::decode_D1(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->flags |= (modrm >> 3 & 7) << 25;
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shift_e16i16 : &CPU::op_shift_e32i32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shift_r16i16 : &CPU::op_shift_r32i32;
    i->imm8 = 1;
    return 0;
}
int CPU::decode_D2(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    i->flags |= (modrm >> 3 & 7) << 25;
    if (modrm < 0xC0)
        i->handler = &CPU::op_shift_e8cl;
    else
        i->handler = &CPU::op_shift_r8cl;
    i->imm8 = 1;
    return 0;
}
int CPU::decode_D3(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->flags |= (modrm >> 3 & 7) << 25;
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shift_e16cl : &CPU::op_shift_e32cl;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shift_r16cl : &CPU::op_shift_r32cl;
    return 0;
}
int CPU::decode_D4(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    i->handler = &CPU::op_aam;
    return 0;
}
int CPU::decode_D5(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    i->handler = &CPU::op_aad;
    return 0;
}
int CPU::decode_D7(struct decoded_instruction *i)
{
    i->flags = 0;
    i->flags |= (m_seg_prefix[0]) << 22;
    i->handler = (m_d_state_hash & 0x0002) ? &CPU::op_xlat16 : &CPU::op_xlat32;
    return 0;
}
int CPU::decode_E0(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_loopnz_rel16 : &CPU::op_loopnz_rel32;
    i->flags   = 0;
    i->disp32  = m_d_state_hash & 0x0002 ? 0xFFFF : -1;
    i->imm32   = (int8_t)*m_rawp++;
    return 0;
}
int CPU::decode_E1(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_loopz_rel16 : &CPU::op_loopz_rel32;
    i->flags   = 0;
    i->disp32  = m_d_state_hash & 0x0002 ? 0xFFFF : -1;
    i->imm32   = (int8_t)*m_rawp++;
    return 0;
}
int CPU::decode_E2(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_loop_rel16 : &CPU::op_loop_rel32;
    i->flags   = 0;
    i->disp32  = m_d_state_hash & 0x0002 ? 0xFFFF : -1;
    i->imm32   = (int8_t)*m_rawp++;
    return 0;
}
int CPU::decode_E3(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_jecxz_rel16 : &CPU::op_jecxz_rel32;
    i->disp32  = m_d_state_hash & 0x0002 ? 0xFFFF : -1;
    i->flags   = 0;
    i->imm32   = (int8_t)*m_rawp++;
    return 0;
}
int CPU::decode_E4(struct decoded_instruction *i)
{
    i->handler = &CPU::op_in_i8al;
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_E5(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_in_i8ax : &CPU::op_in_i8eax;
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_E6(struct decoded_instruction *i)
{
    i->handler = &CPU::op_out_i8al;
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_E7(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_out_i8ax : &CPU::op_out_i8eax;
    i->flags   = 0;
    i->imm8    = *m_rawp++;
    return 0;
}
int CPU::decode_E8(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_call_j16 : &CPU::op_call_j32;
    i->flags   = 0;
    i->imm32   = rvs();
    return 1;
}
int CPU::decode_E9(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_jmp_rel16 : &CPU::op_jmp_rel32;
    i->flags   = 0;
    i->imm32   = rvs();
    return 1;
}
int CPU::decode_EA(struct decoded_instruction *i)
{
    i->handler = &CPU::op_jmpf;
    i->imm32   = rv();
    i->disp16  = rw();
    i->flags   = 0;
    return 1;
}
int CPU::decode_EB(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_jmp_rel16 : &CPU::op_jmp_rel32;
    i->imm32   = (int8_t)*m_rawp++;
    i->flags   = 0;
    return 1;
}
int CPU::decode_EC(struct decoded_instruction *i)
{
    i->handler = &CPU::op_in_dxal;
    i->flags   = 0;
    return 0;
}
int CPU::decode_ED(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_in_dxax : &CPU::op_in_dxeax;
    i->flags   = 0;
    return 0;
}
int CPU::decode_EE(struct decoded_instruction *i)
{
    i->handler = &CPU::op_out_dxal;
    i->flags   = 0;
    return 0;
}
int CPU::decode_EF(struct decoded_instruction *i)
{
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_out_dxax : &CPU::op_out_dxeax;
    i->flags   = 0;
    return 0;
}
int CPU::decode_F4(struct decoded_instruction *i)
{
    i->handler = &CPU::op_hlt;
    i->flags   = 0;
    return 1;
}
int CPU::decode_F5(struct decoded_instruction *i)
{
    i->handler = &CPU::op_cmc;
    i->flags   = 0;
    return 0;
}
int CPU::decode_F6(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++, reg = modrm >> 3 & 7;
    i->flags = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        switch (modrm >> 3 & 7) {
            case 0:
            case 1:
                i->handler = &CPU::op_test_e8i8;
                i->imm8    = *m_rawp++;
                break;
            case 2:
                i->handler = &CPU::op_not_e8;
                break;
            case 3:
                i->handler = &CPU::op_neg_e8;
                break;
            default:
                i->flags |= (reg) << 25;
                i->handler = &CPU::op_muldiv_e8;
                break;
        }
    else
        switch (modrm >> 3 & 7) {
            case 0:
            case 1:
                i->handler = &CPU::op_test_r8i8;
                i->imm8    = *m_rawp++;
                break;
            case 2:
                i->handler = &CPU::op_not_r8;
                break;
            case 3:
                i->handler = &CPU::op_neg_r8;
                break;
            default:
                i->flags |= (reg) << 25;
                i->handler = &CPU::op_muldiv_r8;
                break;
        }
    return 0;
}
int CPU::decode_F7(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++, reg = modrm >> 3 & 7;
    i->flags = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        switch (modrm >> 3 & 7) {
            case 0:
            case 1:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_test_e16i16 : &CPU::op_test_e32i32;
                i->imm32   = rv();
                break;
            case 2:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_not_e16 : &CPU::op_not_e32;
                break;
            case 3:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_neg_e16 : &CPU::op_neg_e32;
                break;
            default:
                i->flags |= (reg) << 25;
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_muldiv_e16 : &CPU::op_muldiv_e32;
                break;
        }
    else
        switch (modrm >> 3 & 7) {
            case 0:
            case 1:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_test_r16i16 : &CPU::op_test_r32i32;
                i->imm32   = rv();
                break;
            case 2:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_not_r16 : &CPU::op_not_r32;
                break;
            case 3:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_neg_r16 : &CPU::op_neg_r32;
                break;
            default:
                i->flags |= (reg) << 25;
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_muldiv_r16 : &CPU::op_muldiv_r32;
                break;
        }
    return 0;
}
int CPU::decode_F8(struct decoded_instruction *i)
{
    i->handler = &CPU::op_clc;
    i->flags   = 0;
    return 0;
}
int CPU::decode_F9(struct decoded_instruction *i)
{
    i->handler = &CPU::op_stc;
    i->flags   = 0;
    return 0;
}
int CPU::decode_FA(struct decoded_instruction *i)
{
    i->handler = &CPU::op_cli;
    i->flags   = 0;
    return 0;
}
int CPU::decode_FB(struct decoded_instruction *i)
{
    i->handler = &CPU::op_sti;
    i->flags   = 0;
    return 0;
}
int CPU::decode_FC(struct decoded_instruction *i)
{
    i->handler = &CPU::op_cld;
    i->flags   = 0;
    return 0;
}
int CPU::decode_FD(struct decoded_instruction *i)
{
    i->handler = &CPU::op_std;
    i->flags   = 0;
    return 0;
}
int CPU::decode_FE(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    if (modrm < 0xC0)
        switch (modrm >> 3 & 7) {
            case 0:
                i->handler = &CPU::op_inc_e8;
                break;
            case 1:
                i->handler = &CPU::op_dec_e8;
                break;
            default:
                i->handler = &CPU::op_ud_exception;
                return 1;
        }
    else
        switch (modrm >> 3 & 7) {
            case 0:
                i->handler = &CPU::op_inc_r8;
                break;
            case 1:
                i->handler = &CPU::op_dec_r8;
                break;
            default:
                i->handler = &CPU::op_ud_exception;
                return 1;
        }
    return 0;
}
int CPU::decode_FF(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        switch (modrm >> 3 & 7) {
            case 0:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_inc_e16 : &CPU::op_inc_e32;
                return 0;
            case 1:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_dec_e16 : &CPU::op_dec_e32;
                return 0;
            case 2:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_call_e16 : &CPU::op_call_e32;
                return 1;
            case 3:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_callf_e16 : &CPU::op_callf_e32;
                return 1;
            case 4:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_jmp_e16 : &CPU::op_jmp_e32;
                return 1;
            case 5:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_jmpf_e16 : &CPU::op_jmpf_e32;
                return 1;
            case 6:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_e16 : &CPU::op_push_e32;
                return 0;
            case 7:
                i->handler = &CPU::op_ud_exception;
                return 1;
        }
    else
        switch (modrm >> 3 & 7) {
            case 0:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_inc_r16 : &CPU::op_inc_r32;
                return 0;
            case 1:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_dec_r16 : &CPU::op_dec_r32;
                return 0;
            case 2:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_call_r16 : &CPU::op_call_r32;
                return 1;
            case 4:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_jmp_r16 : &CPU::op_jmp_r32;
                return 1;
            case 6:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_r16 : &CPU::op_push_r32;
                return 0;
            case 3:
            case 5:
                i->handler = &CPU::op_ud_exception;
                return 1;
            case 7:
                i->handler = &CPU::op_ud_exception;
                return 1;
        }
    do {

        util_abort();
        abort();
    } while (0);
}
int CPU::decode_0F00(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++, reg = modrm >> 3 & 7;
    if ((modrm & 48) == 32) {
        int old_m_d_state_hash = m_state_hash;
        m_d_state_hash |= 0x0001;
        i->flags       = parse_modrm(i, modrm, 0);
        m_d_state_hash = old_m_d_state_hash;
        if (modrm & 8) {
            i->handler = modrm < 0xC0 ? &CPU::op_verw_e16 : &CPU::op_verw_r16;
        } else {
            i->handler = modrm < 0xC0 ? &CPU::op_verr_e16 : &CPU::op_verr_r16;
        }
        return 0;
    }
    i->flags = parse_modrm(i, modrm, 6);
    if (modrm < 0xC0) {
        switch (reg) {
            case 0:
            case 1:
                i->imm8    = reg == 0 ? 8 : 6;
                i->handler = &CPU::op_str_sldt_e16;
                break;
            case 2:
                i->handler = &CPU::op_lldt_e16;
                break;
            case 3:
                i->handler = &CPU::op_ltr_e16;
                break;
            default:
                do {
                    util_abort();
                    abort();
                } while (0);
        }
    } else {
        switch (reg) {
            case 0:
            case 1:
                i->imm8    = reg == 0 ? 8 : 6;
                i->disp32  = m_d_state_hash & 0x0001 ? 0xFFFF : -1;
                i->handler = &CPU::op_str_sldt_r16;
                break;
            case 2:
                i->handler = &CPU::op_lldt_r16;
                break;
            case 3:
                i->handler = &CPU::op_ltr_r16;
                break;
            default:
                do {
                    util_abort();
                    abort();
                } while (0);
        }
    }
    return 0;
}
int CPU::decode_0F01(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++, reg = modrm >> 3 & 7;
    i->flags = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0) {
        switch (reg) {
            case 0:
                i->handler = &CPU::op_sgdt_e32;
                break;
            case 1:
                i->handler = &CPU::op_sidt_e32;
                break;
            case 2:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lgdt_e16 : &CPU::op_lgdt_e32;
                break;
            case 3:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lidt_e16 : &CPU::op_lidt_e32;
                break;
            case 4:
                i->handler = &CPU::op_smsw_e16;
                break;
            case 5:
                i->handler = &CPU::op_ud_exception;
                return 1;
            case 6:
                i->handler = &CPU::op_lmsw_e16;
                break;
            case 7:
                i->handler = &CPU::op_invlpg_e8;
                break;
        }
    } else {
        int lmsw_temp;
        switch (reg) {
            case 4:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_smsw_r16 : &CPU::op_smsw_r32;
                break;
            case 1:
                i->flags   = 0;
                i->handler = &CPU::op_nop;
                break;
            case 0:
            case 2:
            case 3:
            case 5:
            case 7:
                i->handler = &CPU::op_ud_exception;
                return 1;
            case 6:
                lmsw_temp = i->flags >> 8 & 15;
                i->flags &= ~(0xF << 8);
                i->flags |= (lmsw_temp << 1) << 8;
                i->handler = &CPU::op_lmsw_r16;
                break;
        }
    }
    return 0;
}
int CPU::decode_0F02(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lar_r16e16 : &CPU::op_lar_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lar_r16r16 : &CPU::op_lar_r32r32;
    return 0;
}
int CPU::decode_0F03(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lsl_r16e16 : &CPU::op_lsl_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lsl_r16r16 : &CPU::op_lsl_r32r32;
    return 0;
}
int CPU::decode_0F06(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_clts;
    return 0;
}
int CPU::decode_0F09(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_wbinvd;
    return 0;
}
int CPU::decode_0F0B(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_ud_exception;
    return 1;
}
int CPU::decode_sse10_17(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_10_17;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sse10_17_tbl[opcode << 2 | m_sse_prefix];
    return 0;
}
int CPU::decode_0F18(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    parse_modrm(i, modrm, 0);
    i->flags   = 0;
    i->handler = &CPU::op_prefetchh;
    return 0;
}
int CPU::decode_0F1F(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    parse_modrm(i, modrm, 0);
    i->flags   = 0;
    i->handler = &CPU::op_nop;
    return 0;
}
int CPU::decode_0F20(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm < 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        int flags = 0;
        flags |= (modrm >> 3 & 7) << 12;
        flags |= (modrm & 7) << 8;
        i->flags   = flags;
        i->handler = &CPU::op_mov_r32cr;
    }
    return 1;
}
int CPU::decode_0F21(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm < 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        int flags = 0;
        flags |= (modrm >> 3 & 7) << 12;
        flags |= (modrm & 7) << 8;
        i->flags   = flags;
        i->handler = &CPU::op_mov_r32dr;
    }
    return 0;
}
int CPU::decode_0F22(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm < 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        int flags = 0;
        flags |= (modrm >> 3 & 7) << 12;
        flags |= (modrm & 7) << 8;
        i->flags   = flags;
        i->handler = &CPU::op_mov_crr32;
    }
    return 1;
}
int CPU::decode_0F23(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm < 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        int flags = 0;
        flags |= (modrm >> 3 & 7) << 12;
        flags |= (modrm & 7) << 8;
        i->flags   = flags;
        i->handler = &CPU::op_mov_drr32;
    }
    return 0;
}
int CPU::decode_sse28_2F(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_28_2F;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sse28_2F_tbl[opcode << 2 | m_sse_prefix] | ((opcode & 1) << 4);
    return 0;
}
int CPU::decode_0F30(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_wrmsr;
    return 0;
}
int CPU::decode_0F31(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_rdtsc;
    return 0;
}
int CPU::decode_0F32(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_rdmsr;
    return 0;
}
int CPU::decode_0F38(struct decoded_instruction *i)
{
    i->imm8       = *m_rawp++;
    uint8_t modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    if (m_sse_prefix == SSE_PREFIX_66)
        i->handler = &CPU::op_sse_6638;
    else
        i->handler = &CPU::op_sse_38;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    return 0;
}
int CPU::decode_sysenter_sysexit(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = m_rawp[-1] & 1 ? &CPU::op_sysexit : &CPU::op_sysenter;
    return 0;
}
int CPU::decode_sse50_57(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_50_57;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sse50_57_tbl[opcode << 2 | m_sse_prefix] | ((opcode & 1) << 4);
    return 0;
}
int CPU::decode_sse58_5F(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_58_5F;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sse58_5F_tbl[opcode << 2 | m_sse_prefix];
    return 0;
}
int CPU::decode_sse60_67(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_60_67;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sse60_67_tbl[opcode << 1 | (m_sse_prefix == SSE_PREFIX_66)];
    return 0;
}
int CPU::decode_sse68_6F(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_68_6F;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sse68_6F_tbl[opcode << 2 | m_sse_prefix] | ((opcode & 1) << 4);
    return 0;
}
int CPU::decode_sse70_76(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_70_76;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    int op = decode_sse70_76_tbl[opcode << 2 | m_sse_prefix], combined_op = (modrm >> 3 & 7) | ((opcode - 1) & 3) << 3;
    int _or = 0;
    if (op < PCMPEQB_MGqMEq)
        _or = *m_rawp++;
    if ((op & 15) == PSHIFT_MGqIb)
        op |= rm_table_pshift_mmx[combined_op] << 4;
    else if ((op & 15) == PSHIFT_XEoIb) {
        int y = rm_table_pshift_sse[combined_op];
        op |= y << 4;
        if (y == PSHIFT_PSRLDQ || y == PSHIFT_PSLLDQ)
            _or <<= 3;
    }
    op |= _or << 8;
    i->imm16 = op;
    return 0;
}
int CPU::decode_0F77(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_emms;
    return 0;
}
int CPU::decode_sse7C_7D(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 1, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    if (m_sse_prefix != SSE_PREFIX_F2 && m_sse_prefix != SSE_PREFIX_66)
        i->handler = &CPU::op_ud_exception;
    else
        i->handler = &CPU::op_sse_7C_7D;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_7C_7F[opcode << 1 | (m_sse_prefix == SSE_PREFIX_F2)];
    return 0;
}
int CPU::decode_sse7E_7F(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 1, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_7E_7F;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_7E_7F[opcode << 2 | m_sse_prefix];
    return 0;
}
int CPU::decode_0FA0(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (4) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_s16 : &CPU::op_push_s32;
    return 0;
}
int CPU::decode_0FA1(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (4) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pop_s16 : &CPU::op_pop_s32;
    return 0;
}
int CPU::decode_0FA2(struct decoded_instruction *i)
{
    i->flags   = 0;
    i->handler = &CPU::op_cpuid;
    return 0;
}
int CPU::decode_0FA3(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0) {
        i->flags |= (0) << 25;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bt_e16 : &CPU::op_bt_e32;
    } else {
        i->disp32  = -1;
        i->imm32   = 0;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bt_r16 : &CPU::op_bt_r32;
    }
    return 0;
}
int CPU::decode_0FA4(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->imm8       = *m_rawp++;
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shld_e16r16i8 : &CPU::op_shld_e32r32i8;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shld_r16r16i8 : &CPU::op_shld_r32r32i8;
    return 0;
}
int CPU::decode_0FA5(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shld_e16r16cl : &CPU::op_shld_e32r32cl;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shld_r16r16cl : &CPU::op_shld_r32r32cl;
    return 0;
}
int CPU::decode_0FA8(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (5) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_push_s16 : &CPU::op_push_s32;
    return 0;
}
int CPU::decode_0FA9(struct decoded_instruction *i)
{
    int flags = 0;
    flags |= (5) << 8;
    i->flags   = flags;
    i->handler = m_d_state_hash & 0x0001 ? &CPU::op_pop_s16 : &CPU::op_pop_s32;
    return 0;
}
int CPU::decode_0FAB(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0) {
        i->flags |= (0) << 25;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bts_e16 : &CPU::op_bts_e32;
    } else {
        i->disp32  = -1;
        i->imm32   = 0;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bts_r16 : &CPU::op_bts_r32;
    }
    return 0;
}
int CPU::decode_0FAC(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    i->imm8       = *m_rawp++;
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shrd_e16r16i8 : &CPU::op_shrd_e32r32i8;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shrd_r16r16i8 : &CPU::op_shrd_r32r32i8;
    return 0;
}
int CPU::decode_0FAD(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shrd_e16r16cl : &CPU::op_shrd_e32r32cl;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_shrd_r16r16cl : &CPU::op_shrd_r32r32cl;
    return 0;
}
int CPU::decode_0FAE(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    switch (modrm >> 3 & 7) {
        case 0:
            if (modrm >= 0xC0) {
                i->handler = &CPU::op_ud_exception;
                return 1;
            } else
                i->handler = &CPU::op_fxsave;
            break;
        case 1:
            if (modrm >= 0xC0) {
                i->handler = &CPU::op_ud_exception;
                return 1;
            } else
                i->handler = &CPU::op_fxrstor;
            break;
        case 2:
            if (modrm >= 0xC0) {
                i->handler = &CPU::op_ud_exception;
                return 1;
            } else
                i->handler = &CPU::op_ldmxcsr;
            break;
        case 3:
            if (modrm >= 0xC0) {
                i->handler = &CPU::op_ud_exception;
                return 1;
            } else
                i->handler = &CPU::op_stmxcsr;
            break;
        case 4:
            i->handler = &CPU::op_ud_exception;
            return 1;
        case 6:
        case 5:
        case 7:
            i->handler = &CPU::op_mfence;
            break;
        default:
            do {
                util_abort();
                abort();
            } while (0);
    }
    return 0;
}
int CPU::decode_0FAF(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_imul_r16e16 : &CPU::op_imul_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_imul_r16r16 : &CPU::op_imul_r32r32;
    return 0;
}
int CPU::decode_0FB0(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    i->handler    = modrm < 0xC0 ? &CPU::op_cmpxchg_e8r8 : &CPU::op_cmpxchg_r8r8;
    return 0;
}
int CPU::decode_0FB1(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (m_d_state_hash & 0x0001)
        i->handler = modrm < 0xC0 ? &CPU::op_cmpxchg_e16r16 : &CPU::op_cmpxchg_r16r16;
    else
        i->handler = modrm < 0xC0 ? &CPU::op_cmpxchg_e32r32 : &CPU::op_cmpxchg_r32r32;
    return 0;
}
int CPU::decode_0FB2(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lss_r16e16 : &CPU::op_lss_r32e32;
    }
    return 0;
}
int CPU::decode_0FB3(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0) {
        i->flags |= (0) << 25;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btr_e16 : &CPU::op_btr_e32;
    } else {
        i->disp32  = -1;
        i->imm32   = 0;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btr_r16 : &CPU::op_btr_r32;
    }
    return 0;
}
int CPU::decode_0FB4(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lfs_r16e16 : &CPU::op_lfs_r32e32;
    }
    return 0;
}
int CPU::decode_0FB5(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        i->flags   = parse_modrm(i, modrm, 0);
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_lgs_r16e16 : &CPU::op_lgs_r32e32;
    }
    return 0;
}
int CPU::decode_0FB6(struct decoded_instruction *i)
{
    uint8_t modrm                        = *m_rawp++;
    i->flags                             = parse_modrm(i, modrm, 3);
    static const insn_handler_t movzx[4] = {&CPU::op_movzx_r32r8, &CPU::op_movzx_r16r8, &CPU::op_movzx_r32e8,
                                            &CPU::op_movzx_r16e8};
    i->handler                           = movzx[(modrm < 0xC0) << 1 | (m_d_state_hash & 0x0001)];
    return 0;
}
int CPU::decode_0FB7(struct decoded_instruction *i)
{
    uint8_t modrm                        = *m_rawp++;
    i->flags                             = parse_modrm(i, modrm, 0);
    static const insn_handler_t movzx[4] = {&CPU::op_movzx_r32r16, &CPU::op_mov_r16r16, &CPU::op_movzx_r32e16,
                                            &CPU::op_mov_r16e16};
    i->handler                           = movzx[(modrm < 0xC0) << 1 | (m_d_state_hash & 0x0001)];
    return 0;
}
int CPU::decode_0FBA(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if ((modrm & 0x20) == 0) {
        i->handler = &CPU::op_ud_exception;
        return 1;
    }
    i->imm8 = *m_rawp++;
    if (modrm < 0xC0) {
        i->flags |= (1) << 25;
        switch (modrm >> 3 & 7) {
            case 4:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bt_e16 : &CPU::op_bt_e32;
                break;
            case 5:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bts_e16 : &CPU::op_bts_e32;
                break;
            case 6:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btr_e16 : &CPU::op_btr_e32;
                break;
            case 7:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btc_e16 : &CPU::op_btc_e32;
                break;
        }
    } else {
        i->flags |= (1) << 25;
        i->disp32 = 0;
        switch (modrm >> 3 & 7) {
            case 4:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bt_r16 : &CPU::op_bt_r32;
                break;
            case 5:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bts_r16 : &CPU::op_bts_r32;
                break;
            case 6:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btr_r16 : &CPU::op_btr_r32;
                break;
            case 7:
                i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btc_r16 : &CPU::op_btc_r32;
                break;
        }
    }
    return 0;
}
int CPU::decode_0FBB(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0) {
        i->flags |= (0) << 25;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btc_e16 : &CPU::op_btc_e32;
    } else {
        i->disp32  = -1;
        i->imm32   = 0;
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_btc_r16 : &CPU::op_btc_r32;
    }
    return 0;
}
int CPU::decode_0FBC(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bsf_r16e16 : &CPU::op_bsf_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bsf_r16r16 : &CPU::op_bsf_r32r32;
    return 0;
}
int CPU::decode_0FBD(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm < 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bsr_r16e16 : &CPU::op_bsr_r32e32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_bsr_r16r16 : &CPU::op_bsr_r32r32;
    return 0;
}
int CPU::decode_0FBE(struct decoded_instruction *i)
{
    uint8_t modrm                        = *m_rawp++;
    i->flags                             = parse_modrm(i, modrm, 3);
    static const insn_handler_t movzx[4] = {&CPU::op_movsx_r32r8, &CPU::op_movsx_r16r8, &CPU::op_movsx_r32e8,
                                            &CPU::op_movsx_r16e8};
    i->handler                           = movzx[(modrm < 0xC0) << 1 | (m_d_state_hash & 0x0001)];
    return 0;
}
int CPU::decode_0FBF(struct decoded_instruction *i)
{
    uint8_t modrm                        = *m_rawp++;
    i->flags                             = parse_modrm(i, modrm, 0);
    static const insn_handler_t movzx[4] = {&CPU::op_movsx_r32r16, &CPU::op_mov_r16r16, &CPU::op_movsx_r32e16,
                                            &CPU::op_mov_r16e16};
    i->handler                           = movzx[(modrm < 0xC0) << 1 | (m_d_state_hash & 0x0001)];
    return 0;
}
int CPU::decode_0FC0(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 1);
    if (modrm >= 0xC0)
        i->handler = &CPU::op_xadd_r8r8;
    else
        i->handler = &CPU::op_xadd_r8e8;
    return 0;
}
int CPU::decode_0FC1(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    i->flags      = parse_modrm(i, modrm, 0);
    if (modrm >= 0xC0)
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_xadd_r16r16 : &CPU::op_xadd_r32r32;
    else
        i->handler = m_d_state_hash & 0x0001 ? &CPU::op_xadd_r16e16 : &CPU::op_xadd_r32e32;
    return 0;
}
int CPU::decode_0FC7(struct decoded_instruction *i)
{
    uint8_t modrm = *m_rawp++;
    if (modrm >= 0xC0) {
        i->flags   = 0;
        i->handler = &CPU::op_ud_exception;
        return 1;
    } else {
        i->flags   = parse_modrm(i, modrm, 6);
        i->handler = &CPU::op_cmpxchg8b_e32;
        return 0;
    }
}
int CPU::decode_sseC2_C6(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_C2_C6;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    opcode -= 2;
    int op = decode_sseC2_C6_tbl[opcode << 2 | m_sse_prefix];
    if (op != MOVNTI_EdGd)
        op |= *m_rawp++ << 8;
    i->imm16 = op;
    return 0;
}
int CPU::decode_sseD0_D7(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_D0_D7;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    opcode--;
    i->imm8 = decode_sseD0_D7_tbl[opcode << 2 | m_sse_prefix];
    return 0;
}
int CPU::decode_sseD8_DF(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_D8_DF;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sseD8_DF_tbl[opcode << 1 | (m_sse_prefix == SSE_PREFIX_66)];
    return 0;
}
int CPU::decode_sseE0_E7(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_E0_E7;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sseE0_E7_tbl[opcode << 2 | m_sse_prefix];
    return 0;
}
int CPU::decode_sseE8_EF(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_E8_EF;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sseE8_EF_tbl[opcode << 1 | (m_sse_prefix == SSE_PREFIX_66)];
    return 0;
}
int CPU::decode_sseF1_F7(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_F1_F7;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    opcode--;
    i->imm8 = decode_sseF1_F7_tbl[opcode << 1 | (m_sse_prefix == SSE_PREFIX_66)];
    return 0;
}
int CPU::decode_sseF8_FE(struct decoded_instruction *i)
{
    uint8_t opcode = m_rawp[-1] & 7, modrm = *m_rawp++;
    int     flags = parse_modrm(i, modrm, 6);
    i->handler    = &CPU::op_sse_F8_FE;
    flags |= (modrm >= 0xC0) << 25;
    i->flags = flags;
    i->imm8  = decode_sseF8_FE_tbl[opcode << 1 | (m_sse_prefix == SSE_PREFIX_66)];
    return 0;
}
void CPU::set_smc(int length, uint32_t lin)
{
    m_tlb_tags[lin >> 12] |= 0x44;
    int b128 = ((m_phys_eip + length) >> 7) - (m_phys_eip >> 7) + 1;
    for (int i = 0; i < b128; i++)
        cpu_smc_set_code(m_phys_eip + (i << 7));
}
int CPU::cpu_decode(struct trace_info *info, struct decoded_instruction *i)
{
    m_d_state_hash                    = m_state_hash;
    m_rawp                            = ((uint8_t *)m_mem + (m_phys_eip));
    uint8_t  *m_rawp_base             = m_rawp;
    uintptr_t high_mark               = (uintptr_t)(((uint8_t *)m_mem + ((m_phys_eip & ~0xFFF) + 0xFF0)));
    void     *original                = i;
    int       instructions_translated = 0, instructions_mask = -1;
    while (1) {
        if ((uintptr_t)m_rawp > high_mark) {
            uint32_t current_phys_eip    = (uint32_t)(uintptr_t)((uint8_t *)m_rawp - (uint8_t *)m_mem),
                     maximum_insn_length = 0x1000 - (current_phys_eip & 0xFFF);
            if (maximum_insn_length > 15 || find_instruction_length(maximum_insn_length) == -1) {
                if (instructions_translated != 0) {
                    i->handler = &CPU::op_trace_end;
                    instructions_translated++;
                    int length = (uintptr_t)m_rawp - (uintptr_t)m_rawp_base;
                    if (instructions_mask != 0) {
                        info->phys       = m_phys_eip;
                        info->state_hash = m_state_hash;
                        info->flags      = length;
                        info->ptr        = (struct decoded_instruction *)original;
                        set_smc(length, (m_phys_eip + m_eip_phys_bias + m_seg_base[1]));
                    }
                    return instructions_translated & instructions_mask;
                }
                uint32_t lin_eip   = (m_phys_eip + m_eip_phys_bias + m_seg_base[1]);
                uint32_t next_page = (lin_eip + 15) & ~0xFFF;
                uint8_t  tlb_tag   = m_tlb_tags[next_page >> 12];
                if ((tlb_tag >> m_tlb_shift_read & 1) || m_tlb_attrs[next_page >> 12] & 1) {
                    if (cpu_mmu_translate(next_page, m_tlb_shift_read | 8))
                        do {
                            i->handler = &CPU::op_trace_end;
                            return 0;
                        } while (0);
                }
                for (int j = 0; j < 15; j++)
                    do {
                        uint32_t addr_ = lin_eip + j, shift_ = m_tlb_shift_read, tag = m_tlb_tags[addr_ >> 12];
                        if ((tag >> shift_ & 1)) {
                            if (!cpu_access_read8(addr_, tag >> m_tlb_shift_read, m_tlb_shift_read))
                                m_prefetch[j] = m_read_result;
                            else
                                do {
                                    i->handler = &CPU::op_trace_end;
                                    return 0;
                                } while (0);
                        } else
                            m_prefetch[j] = *(uint8_t *)((uint8_t *)m_tlb[addr_ >> 12] + addr_);
                    } while (0);
                m_rawp                  = m_prefetch;
                instructions_translated = 1000;
                instructions_mask       = 0;
            }
        }
        uint8_t *prev_rawp = m_rawp, opcode = *m_rawp++;
        int      end_of_trace = CALL_MEMBER_FN(*this, table[opcode])(i);

        instructions_translated++;
        i->flags = (i->flags & ~15) | ((uintptr_t)m_rawp - (uintptr_t)prev_rawp);
        ++i;
        if (end_of_trace || instructions_translated >= (32 - 1)) {
            if (!end_of_trace) {
                i->handler = &CPU::op_trace_end;
                instructions_translated++;
            }
            int length = (uintptr_t)m_rawp - (uintptr_t)m_rawp_base;
            if (instructions_mask != 0) {
                info->phys       = m_phys_eip;
                info->state_hash = m_state_hash;
                info->flags      = length;
                info->ptr        = (struct decoded_instruction *)original;
                set_smc(length, (m_phys_eip + m_eip_phys_bias + m_seg_base[1]));
            }
            return instructions_translated & instructions_mask;
        }
    }
}
