#include "cpu.h"

extern CPU cpu;


void CPU::bt16(uint16_t a, int shift)
{
    cpu_set_cf(a >> (shift & 15) & 1);
}
void CPU::bt32(uint32_t a, int shift)
{
    cpu_set_cf(a >> (shift & 31) & 1);
}
void CPU::bts16(uint16_t *a, int shift)
{
    shift &= 15;
    cpu_set_cf(*a >> shift & 1);
    *a |= 1 << shift;
}
void CPU::bts32(uint32_t *a, int shift)
{
    shift &= 31;
    cpu_set_cf(*a >> shift & 1);
    *a |= 1 << shift;
}
void CPU::btc16(uint16_t *a, int shift)
{
    shift &= 15;
    cpu_set_cf(*a >> shift & 1);
    *a ^= 1 << shift;
}
void CPU::btc32(uint32_t *a, int shift)
{
    shift &= 31;
    cpu_set_cf(*a >> shift & 1);
    *a ^= 1 << shift;
}
void CPU::btr16(uint16_t *a, int shift)
{
    shift &= 15;
    cpu_set_cf(*a >> shift & 1);
    *a &= ~(1 << shift);
}
void CPU::btr32(uint32_t *a, int shift)
{
    shift &= 31;
    cpu_set_cf(*a >> shift & 1);
    *a &= ~(1 << shift);
}
uint16_t CPU::bsf16(uint16_t src, uint16_t old)
{
    if (src) {
        cpu_set_zf(0);
        return __builtin_ctz(src & 0xFFFF);
    } else {
        cpu_set_zf(1);
        return old;
    }
}
uint32_t CPU::bsf32(uint32_t src, uint32_t old)
{
    m_laux = BIT;
    if (src) {
        m_lr = 1;
        return __builtin_ctz(src);
    } else {
        m_lr = 0;
        return old;
    }
}
uint16_t CPU::bsr16(uint16_t src, uint16_t old)
{
    if (src) {
        cpu_set_zf(0);
        return __builtin_clz(src & 0xFFFF) ^ 31;
    } else {
        cpu_set_zf(1);
        return old;
    }
}
uint32_t CPU::bsr32(uint32_t src, uint32_t old)
{
    if (src) {
        cpu_set_zf(0);
        return __builtin_clz(src) ^ 31;
    } else {
        cpu_set_zf(1);
        return old;
    }
}
