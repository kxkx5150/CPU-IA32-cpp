#include "cpu.h"
#include "../io.h"
#include <cstdint>

extern CPU cpu;


void CPU::cpu_mmu_tlb_flush(void)
{
    for (unsigned int i = 0; i < m_tlb_entry_count; i++) {
        uint32_t entry = m_tlb_entry_indexes[i];
        if (entry == (uint32_t)-1)
            continue;
        m_tlb[entry]           = ((void *)0);
        m_tlb_tags[entry]      = 0xFF;
        m_tlb_entry_indexes[i] = -1;
        m_tlb_attrs[entry]     = 0xFF;
    }
    m_tlb_entry_count = 0;
}
void CPU::cpu_mmu_tlb_flush_nonglobal(void)
{
    for (unsigned int i = 0; i < m_tlb_entry_count; i++) {
        uint32_t entry = m_tlb_entry_indexes[i];
        if (entry == (uint32_t)-1)
            continue;
        if ((m_tlb_attrs[entry] & 2) == 0)
            continue;
        m_tlb[entry]           = ((void *)0);
        m_tlb_tags[entry]      = 0xFF;
        m_tlb_entry_indexes[i] = -1;
        m_tlb_attrs[entry]     = 0xFF;
    }
    m_tlb_entry_count = m_tlb_entry_count;
}
void CPU::cpu_set_tlb_entry(uint32_t lin, uint32_t phys, void *ptr, int user, int write, int global, int nx)
{
    phys &= m_a20_mask;
    if (phys >= 0xFFF00000)
        phys &= 0xFFFFF;
    int tag = 0, tag_write = 0;
    if (phys >= 0xA0000 && phys < 0x100000) {
        tag       = (phys & 0x40000) == 0;
        tag_write = 1;
    }
    if (phys >= m_memory_size) {
        tag       = 1;
        tag_write = 1;
    }
    if (cpu_smc_page_has_code(phys)) {
        tag_write = 1;
    }
    if (m_tlb_entry_count >= 8192) {
        cpu_mmu_tlb_flush();
    }
    int system_read = tag << 0, system_write = (tag_write | (!write ? 3 : 0)) << 2,
        user_read = (tag | (!user ? 3 : 0)) << 4, user_write = (tag_write | ((!user | !write) ? 3 : 0)) << 6;
    uint32_t entry                           = lin >> 12;
    m_tlb_entry_indexes[m_tlb_entry_count++] = entry;
    m_tlb_attrs[entry]                       = (nx ? 1 : 0) | (global ? 0 : 2);
    if (!ptr)
        ptr = ((uint8_t *)m_mem + phys);
    m_tlb[entry]      = (void *)(((uintptr_t)ptr) - lin);
    m_tlb_tags[entry] = system_read | system_write | user_read | user_write;
}
uint32_t CPU::cpu_read_phys(uint32_t addr)
{
    if (addr >= m_memory_size || (addr >= 0xA0000 && addr < 0xC0000))
        return io_handle_mmio_read(addr, 2);
    else
        return *(uint32_t *)((uint8_t *)m_mem + addr);
}
void CPU::cpu_write_phys(uint32_t addr, uint32_t data)
{
    if (addr >= m_memory_size || (addr >= 0xA0000 && addr < 0xC0000))
        io_handle_mmio_write(addr, data, 2);
    else
        *(uint32_t *)((uint8_t *)m_mem + addr) = data;
}
int CPU::cpu_mmu_translate(uint32_t lin, int shift)
{
    int fail = 0;
    if (!(m_cr[0] & (1 << 31))) {
        cpu_set_tlb_entry(lin & ~0xFFF, lin & ~0xFFF, ((void *)0), 1, 1, 0, 0);
        return 0;
    } else {
        int execute = shift & 8;
        shift &= 7;
        int write = shift >> 1 & 1, user = shift >> 2 & 1;
        if (!(m_cr[4] & (1 << 5))) {
            int      error_code                = 0;
            uint32_t page_directory_entry_addr = m_cr[3] + (lin >> 20 & 0xFFC), page_directory_entry = -1,
                     page_table_entry_addr = -1, page_table_entry = -1;
            page_directory_entry = cpu_read_phys(page_directory_entry_addr);
            if (!(page_directory_entry & 1)) {
                error_code = 0;
                goto page_fault;
            }
            page_table_entry_addr = ((lin >> 10 & 0xFFC) + (page_directory_entry & ~0xFFF));
            page_table_entry      = -1;
            if (page_directory_entry & 0x80 && m_cr[4] & (1 << 4)) {
                uint32_t new_page_dierctory_entry = page_directory_entry | 0x20 | (write << 6);
                if (new_page_dierctory_entry != page_directory_entry) {
                    cpu_write_phys(page_directory_entry_addr, new_page_dierctory_entry);
                }
                uint32_t phys = (page_directory_entry & 0xFFC00000) | (lin & 0x3FF000);
                cpu_set_tlb_entry(lin & ~0xFFF, phys, ((void *)0), user, write, page_directory_entry & 0x100, 0);
            } else {
                page_table_entry = cpu_read_phys(page_table_entry_addr);
                if ((page_table_entry & 1) == 0) {
                    error_code = 0;
                    goto page_fault;
                }
                uint32_t combined   = ~page_table_entry | ~page_directory_entry;
                int      write_mask = write << 1;
                if (combined & write_mask) {
                    if (user || (m_cr[0] & 65536)) {
                        error_code = 1;
                        goto page_fault;
                    }
                }
                int user_mask = user << 2;
                if (combined & user_mask) {
                    error_code = 1;
                    goto page_fault;
                }
                if ((page_directory_entry & 0x20) == 0) {
                    cpu_write_phys(page_directory_entry_addr, page_directory_entry | 0x20);
                }
                uint32_t new_page_table_entry = page_table_entry | (write << 6) | 0x20;
                if (new_page_table_entry != page_table_entry) {
                    cpu_write_phys(page_table_entry_addr, new_page_table_entry);
                }
                cpu_set_tlb_entry(lin & ~0xFFF, page_table_entry & ~0xFFF, ((void *)0), user, write,
                                  page_table_entry & 0x100, 0);
            }
            return 0;
        page_fault:
            m_cr[2] = lin;
            error_code |= (write << 1) | (user << 2);

            do {
                cpu_exception(14, (error_code) | 0x10000);
                return 1;
            } while (0);
            return -1;
        } else {
            uint32_t pdp_addr = (m_cr[3] & ~31) | (lin >> 27 & 0x18), pdpte = cpu_read_phys(pdp_addr);
            fail = (write << 1) | (user << 2);
            if ((pdpte & 1) == 0)
                goto pae_page_fault;
            if (cpu_read_phys(pdp_addr + 4) & ~15)
                do {
                    cpu_exception(13, (0) | 0x10000);
                    return 1;
                } while (0);
            uint32_t pde_addr = (pdpte & ~0xFFF) | (lin >> 18 & 0xFF8), pde = cpu_read_phys(pde_addr),
                     pde2    = cpu_read_phys(pde_addr + 4);
            uint32_t nx_mask = -1 ^ (m_ia32_efer << 20 & 0x80000000);
            if (cpu_read_phys(pdp_addr + 4) & ~15 & nx_mask)
                do {
                    cpu_exception(13, (0) | 0x10000);
                    return 1;
                } while (0);
            if (pde2 & ~15 & nx_mask)
                do {
                    cpu_exception(13, (0) | 0x10000);
                    return 1;
                } while (0);
            int nx_enabled = m_ia32_efer >> 11 & 1, nx = (pde2 >> 31) & nx_enabled;
            fail |= (execute && nx_enabled) << 4;
            if ((pde & 1) == 0) {
                fail |= 0;
                goto pae_page_fault;
            }
            uint32_t flags = ~pde;
            if ((write << 1) & flags) {
                if (user || (m_cr[0] & 65536)) {
                    fail |= 1;
                    goto pae_page_fault;
                }
            }
            if ((user << 2) & flags) {
                fail |= 1;
                goto pae_page_fault;
            }
            if (pde & (1 << 7)) {
                uint32_t new_pde = pde | 0x20 | (write << 6);
                if (new_pde != pde) {
                    cpu_write_phys(pde_addr, new_pde);
                }
                uint32_t phys = (pde & 0xFFE00000) | (lin & 0x1FF000);
                cpu_set_tlb_entry(lin & ~0xFFF, phys, ((void *)0), user, write, pde & 0x100, nx);
            } else {
                uint32_t pte_addr = (pde & ~0xFFF) | (lin >> 9 & 0xFF8), pte = cpu_read_phys(pte_addr),
                         pte2 = cpu_read_phys(pte_addr + 4);
                if ((pte & 1) == 0)
                    goto pae_page_fault;
                (void)(pte2);
                flags = ~pte;
                if ((write << 1) & flags) {
                    if (user || (m_cr[0] & 65536)) {

                        fail |= 1;
                        goto pae_page_fault;
                    }
                }
                if ((user << 2) & flags) {

                    fail |= 1;
                    goto pae_page_fault;
                }
                uint32_t new_pde = pde | 0x20;
                if (new_pde != pde) {
                    cpu_write_phys(pde_addr, new_pde);
                }
                uint32_t new_pte = pte | 0x20 | (write << 6);
                if (new_pte != pte) {
                    cpu_write_phys(pte_addr, new_pte);
                }
                cpu_set_tlb_entry(lin & ~0xFFF, pte & ~0xFFF, ((void *)0), user, write, pte & 0x100, nx);
            }
            return 0;
        }
    }
pae_page_fault:
    m_cr[2] = lin;
    do {
        cpu_exception(14, (fail) | 0x10000);
        return 1;
    } while (0);
}
void CPU::cpu_mmu_tlb_invalidate(uint32_t lin)
{
    lin >>= 12;
    m_tlb[lin]      = ((void *)0);
    m_tlb_tags[lin] = 0xFF;
}
