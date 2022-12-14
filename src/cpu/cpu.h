#ifndef CPU_H
#define CPU_H

#include "cpu_def.h"
#include "../util.h"
#include <stdint.h>
#include <vector>


typedef struct
{
    uint64_t fraction;
    uint16_t exp;
} floatx80;
typedef struct float_status
{
    int float_rounding_precision;
    int float_rounding_mode;
    int float_exception_flags;
    int float_exception_masks;
    int float_suppress_exception;
    int float_nan_handling_mode;
    int flush_underflow_to_zero;
    int denormals_are_zeros;
} float_status_t;
typedef enum
{
    float_zero,
    float_SNaN,
    float_QNaN,
    float_negative_inf,
    float_positive_inf,
    float_denormal,
    float_normalized
} float_class_t;
typedef struct
{
    uint64_t lo, hi;
} float128;


enum
{
    ADD8,
    ADD16,
    ADD32,
    BIT,
    ADC8,
    ADC16,
    ADC32,
    SBB8,
    SBB16,
    SBB32,
    SUB8,
    SUB16,
    SUB32,
    SHL8,
    SHL16,
    SHL32,
    SHR8,
    SHR16,
    SHR32,
    SAR8,
    SAR16,
    SAR32,
    SHLD16,
    SHLD32,
    SHRD16,
    SHRD32,
    MUL,
    INC8,
    INC16,
    INC32,
    DEC8,
    DEC16,
    DEC32,
    EFLAGS_FULL_UPDATE
};
enum
{
    AVAILABLE_TSS_286  = 1,
    LDT                = 2,
    BUSY_TSS_286       = 3,
    CALL_GATE_286      = 4,
    TASK_GATE          = 5,
    INTERRUPT_GATE_286 = 6,
    TRAP_GATE_286      = 7,
    AVAILABLE_TSS_386  = 9,
    BUSY_TSS_386       = 11,
    CALL_GATE_386      = 12,
    INTERRUPT_GATE_386 = 14,
    TRAP_GATE_386      = 15,
};
enum
{
    MOVUPS_XGoXEo,
    MOVSS_XGdXEd,
    MOVSD_XGqXEq,
    MOVUPS_XEoXGo,
    MOVSS_XEdXGd,
    MOVSD_XEqXGq,
    MOVHLPS_XGqXEq,
    MOVLPS_XGqXEq,
    UNPCKLPS_XGoXEq,
    UNPCKLPD_XGoXEo,
    UNPCKHPS_XGoXEq,
    UNPCKHPD_XGoXEo,
    MOVLHPS_XGqXEq,
    MOVHPS_XGqXEq,
    MOVSHDUP_XGoXEo,
    MOVHPS_XEqXGq
};
enum
{
    MOVAPS_XGoXEo,
    MOVAPS_XEoXGo,
    CVTPI2PS_XGqMEq,
    CVTSI2SS_XGdEd,
    CVTPI2PD_XGoMEq,
    CVTSI2SD_XGqMEd,
    CVTPS2PI_MGqXEq,
    CVTSS2SI_GdXEd,
    CVTPD2PI_MGqXEo,
    CVTSD2SI_GdXEq,
    UCOMISS_XGdXEd,
    UCOMISD_XGqXEq
};
enum
{
    MOVMSKPS_GdXEo,
    MOVMSKPD_GdXEo,
    SQRTPS_XGoXEo,
    SQRTSS_XGdXEd,
    SQRTPD_XGoXEo,
    SQRTSD_XGqXEq,
    RSQRTPS_XGoXEo,
    RSQRTSS_XGdXEd,
    RCPPS_XGoXEo,
    RCPSS_XGdXEd,
    ANDPS_XGoXEo,
    ANDNPS_XGoXEo,
    ORPS_XGoXEo,
    XORPS_XGoXEo
};
enum
{
    ADDPS_XGoXEo,
    ADDSS_XGdXEd,
    ADDPD_XGoXEo,
    ADDSD_XGqXEq,
    MULPS_XGoXEo,
    MULSS_XGdXEd,
    MULPD_XGoXEo,
    MULSD_XGqXEq,
    CVTPS2PD_XGoXEo,
    CVTPD2PS_XGoXEo,
    CVTSS2SD_XGoXEd,
    CVTSD2SS_XGoXEq,
    CVTDQ2PS_XGoXEo,
    CVTPS2DQ_XGoXEo,
    CVTTPS2DQ_XGoXEo,
    SUBPS_XGoXEo,
    SUBSS_XGdXEd,
    SUBPD_XGoXEo,
    SUBSD_XGqXEq,
    MINPS_XGoXEo,
    MINSS_XGdXEd,
    MINPD_XGoXEo,
    MINSD_XGqXEq,
    DIVPS_XGoXEo,
    DIVSS_XGdXEd,
    DIVPD_XGoXEo,
    DIVSD_XGqXEq,
    MAXPS_XGoXEo,
    MAXSS_XGdXEd,
    MAXPD_XGoXEo,
    MAXSD_XGqXEq
};
enum
{
    PUNPCKLBW_MGqMEq,
    PUNPCKLBW_XGoXEo,
    PUNPCKLWD_MGqMEq,
    PUNPCKLWD_XGoXEo,
    PUNPCKLDQ_MGqMEq,
    PUNPCKLDQ_XGoXEo,
    PACKSSWB_MGqMEq,
    PACKSSWB_XGoXEo,
    PCMPGTB_MGqMEq,
    PCMPGTB_XGoXEo,
    PCMPGTW_MGqMEq,
    PCMPGTW_XGoXEo,
    PCMPGTD_MGqMEq,
    PCMPGTD_XGoXEo,
    PACKUSWB_MGqMEq,
    PACKUSWB_XGoXEo
};
enum
{
    PUNPCKHBW_MGqMEq,
    PUNPCKHBW_XGoXEo,
    PUNPCKHWD_MGqMEq,
    PUNPCKHWD_XGoXEo,
    PUNPCKHDQ_MGqMEq,
    PUNPCKHDQ_XGoXEo,
    PACKSSDW_MGqMEq,
    PACKSSDW_XGoXEo,
    PUNPCKLQDQ_XGoXEo,
    PUNPCKHQDQ_XGoXEo,
    MOVD_MGdEd,
    MOVD_XGdEd,
    MOVQ_MGqMEq,
    MOVDQA_XGoXEo,
    MOVDQU_XGoXEo,
    OP_68_6F_INVALID
};
enum
{
    PSHUFW_MGqMEqIb,
    PSHUFLW_XGoXEoIb,
    PSHUFHW_XGoXEoIb,
    PSHUFD_XGoXEoIb,
    PSHIFT_MGqIb,
    PSHIFT_XEoIb,
    PCMPEQB_MGqMEq,
    PCMPEQB_XGoXEo,
    PCMPEQW_MGqMEq,
    PCMPEQW_XGoXEo,
    PCMPEQD_MGqMEq,
    PCMPEQD_XGoXEo
};
enum
{
    HADDPD_XGoXEo,
    HADDPS_XGoXEo,
    HSUBPD_XGoXEo,
    HSUBPS_XGoXEo
};
enum
{
    MOVD_EdMGd,
    MOVD_EdXGd,
    MOVQ_XGqXEq,
    MOVQ_MEqMGq,
    MOVDQA_XEqXGq,
    MOVDQU_XEqXGq
};
enum
{
    CMPPS_XGoXEoIb,
    CMPSS_XGdXEdIb,
    CMPPD_XGoXEoIb,
    CMPSD_XGqXEqIb,
    MOVNTI_EdGd,
    PINSRW_MGqEdIb,
    PINSRW_XGoEdIb,
    PEXTRW_GdMEqIb,
    PEXTRW_GdXEoIb,
    SHUFPS_XGoXEoIb,
    SHUFPD_XGoXEoIb
};
enum
{
    PSHIFT_PSRLW,
    PSHIFT_PSRAW,
    PSHIFT_PSLLW,
    PSHIFT_PSRLD,
    PSHIFT_PSRAD,
    PSHIFT_PSLLD,
    PSHIFT_PSRLQ,
    PSHIFT_PSRLDQ,
    PSHIFT_PSLLQ,
    PSHIFT_PSLLDQ
};
enum
{
    PSRLW_MGqMEq,
    PSRLW_XGoXEo,
    PSRLD_MGqMEq,
    PSRLD_XGoXEo,
    PSRLQ_MGqMEq,
    PSRLQ_XGoXEo,
    PADDQ_MGqMEq,
    PADDQ_XGoXEo,
    PMULLW_MGqMEq,
    PMULLW_XGoXEo,
    MOVQ_XEqXGq,
    MOVQ2DQ_XGoMEq,
    MOVDQ2Q_MGqXEo,
    PMOVMSKB_GdMEq,
    PMOVMSKB_GdXEo
};
enum
{
    PSUBUSB_MGqMEq,
    PSUBUSB_XGoXEo,
    PSUBUSW_MGqMEq,
    PSUBUSW_XGoXEo,
    PMINUB_MGqMEq,
    PMINUB_XGoXEo,
    PAND_MGqMEq,
    PAND_XGoXEo,
    PADDUSB_MGqMEq,
    PADDUSB_XGoXEo,
    PADDUSW_MGqMEq,
    PADDUSW_XGoXEo,
    PMAXUB_MGqMEq,
    PMAXUB_XGoXEo,
    PANDN_MGqMEq,
    PANDN_XGoXEo
};
enum
{
    PAVGB_MGqMEq,
    PAVGB_XGoXEo,
    PSRAW_MGqMEq,
    PSRAW_XGoXEo,
    PSRAD_MGqMEq,
    PSRAD_XGoXEo,
    PAVGW_MGqMEq,
    PAVGW_XGoXEo,
    PMULHUW_MGqMEq,
    PMULHUW_XGoXEo,
    PMULHW_MGqMEq,
    PMULHW_XGoXEo,
    CVTPD2DQ_XGoXEo,
    CVTTPD2DQ_XGoXEo,
    CVTDQ2PD_XGoXEq,
    MOVNTQ_MEqMGq,
    MOVNTDQ_XEoXGo
};
enum
{
    PSUBSB_MGqMEq,
    PSUBSB_XGoXEo,
    PSUBSW_MGqMEq,
    PSUBSW_XGoXEo,
    PMINSW_MGqMEq,
    PMINSW_XGoXEo,
    POR_MGqMEq,
    POR_XGoXEo,
    PADDSB_MGqMEq,
    PADDSB_XGoXEo,
    PADDSW_MGqMEq,
    PADDSW_XGoXEo,
    PMAXSW_MGqMEq,
    PMAXSW_XGoXEo,
    PXOR_MGqMEq,
    PXOR_XGoXEo
};
enum
{
    PSLLW_MGqMEq,
    PSLLW_XGoXEo,
    PSLLD_MGqMEq,
    PSLLD_XGoXEo,
    PSLLQ_MGqMEq,
    PSLLQ_XGoXEo,
    PMULLUDQ_MGqMEq,
    PMULLUDQ_XGoXEo,
    PMADDWD_MGqMEq,
    PMADDWD_XGoXEo,
    PSADBW_MGqMEq,
    PSADBW_XGoXEo,
    MASKMOVQ_MEqMGq,
    MASKMOVDQ_XEoXGo
};
enum
{
    PSUBB_MGqMEq,
    PSUBB_XGoXEo,
    PSUBW_MGqMEq,
    PSUBW_XGoXEo,
    PSUBD_MGqMEq,
    PSUBD_XGoXEo,
    PSUBQ_MGqMEq,
    PSUBQ_XGoXEo,
    PADDB_MGqMEq,
    PADDB_XGoXEo,
    PADDW_MGqMEq,
    PADDW_XGoXEo,
    PADDD_MGqMEq,
    PADDD_XGoXEo
};
enum
{
    float_relation_less      = -1,
    float_relation_equal     = 0,
    float_relation_greater   = 1,
    float_relation_unordered = 2
};
enum
{
    float_muladd_negate_c       = 1,
    float_muladd_negate_product = 2,
    float_muladd_negate_result  = float_muladd_negate_c | float_muladd_negate_product
};
enum
{
    INTERRUPT_TYPE_EXCEPTION,
    INTERRUPT_TYPE_SOFTWARE,
    INTERRUPT_TYPE_HARDWARE
};
enum
{
    FEATURE_EAX_1 = 0,
    FEATURE_EAX_80000000,
    FEATURE_SIZE_MAX
};
enum
{
    TASK_JMP,
    TASK_CALL,
    TASK_INT,
    TASK_IRET
};
enum
{
    opcode_singlebyte = 0,
    opcode_prefix,
    opcode_special,
    opcode_modrm,
    opcode_moffs,
    opcode_invalid
};
enum
{
    SSE_PREFIX_NONE = 0,
    SSE_PREFIX_66,
    SSE_PREFIX_F2,
    SSE_PREFIX_F3
};
enum
{
    MODRM_SIZE8,
    MODRM_SIZE16,
    MODRM_SIZE32
};
enum
{
    FPU_TAG_VALID   = 0,
    FPU_TAG_ZERO    = 1,
    FPU_TAG_SPECIAL = 2,
    FPU_TAG_EMPTY   = 3
};
enum
{
    FPU_ROUND_NEAREST  = 0,
    FPU_ROUND_DOWN     = 1,
    FPU_ROUND_UP       = 2,
    FPU_ROUND_TRUNCATE = 3
};
enum
{
    FPU_PRECISION_24 = 0,
    FPU_PRECISION_53 = 2,
    FPU_PRECISION_64 = 3
};
enum float_nanandling_mode_t
{
    float_larger_significand_nan = 0,
    float_first_operand_nan      = 1
};
enum float_round_t
{
    float_round_nearest_even = 0,
    float_round_down         = 1,
    float_round_up           = 2,
    float_round_to_zero      = 3
};
enum float_exception_flag_t
{
    float_flag_invalid   = 0x01,
    float_flag_denormal  = 0x02,
    float_flag_divbyzero = 0x04,
    float_flag_overflow  = 0x08,
    float_flag_underflow = 0x10,
    float_flag_inexact   = 0x20
};

struct seg_desc
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint16_t limit_0_15;
            uint16_t base_0_15;
            uint8_t  base_16_23;
            uint16_t access;
            uint8_t  base_24_31;
        } fields;
        uint32_t raw[2];
    };
};
struct cpuid_level_info
{
    int eax;
    int ecx;
    int edx;
    int ebx;
    int level;
};
struct cpu_config
{
    char                   *vendor_name;
    int                     level;
    int                     cpuid_limit_winnt;
    struct cpuid_level_info features[FEATURE_SIZE_MAX];
};


class FPU {
  public:
    union
    {
        struct
        {
            union
            {
                uint8_t  r8[8];
                uint16_t r16[4];
                uint32_t r32[2];
                uint64_t r64;
            } reg;
            uint16_t dummy;
        } mm[8];
        floatx80 st[8];
    };
    int            ftop;
    uint16_t       control_word, status_word, tag_word;
    uint32_t       fpu_eip, fpu_data_ptr;
    uint16_t       fpu_cs, fpu_opcode, fpu_data_seg;
    float_status_t status;
};
class CPU {
  public:
    class FPU m_fpu;

  private:
    struct decoded_instruction;
    typedef struct CPU::decoded_instruction *(CPU::*insn_handler_t)(struct CPU::decoded_instruction *);
    typedef int (CPU::*decode_handler_t)(struct CPU::decoded_instruction *);
    typedef int (*float32_compare_method)(float32, float32, float_status_t *status);
    typedef int (*float64_compare_method)(float64, float64, float_status_t *status);

    struct decoded_instruction
    {
        uint32_t flags;
        union
        {
            uint32_t imm32;
            uint16_t imm16;
            uint8_t  imm8;
        };
        union
        {
            uint32_t disp32;
            uint16_t disp16;
            uint8_t  disp8;
            int16_t  disp16s;
            int8_t   disp8s;
        };
        insn_handler_t handler;
    };
    typedef struct decoded_instruction __insn_t;

    struct trace_info
    {
        uint32_t                    phys, state_hash;
        struct decoded_instruction *ptr;
        uint32_t                    flags;
    };
    struct decoded_instruction temporary_placeholder;

    typedef struct
    {
        int      sign;
        uint64_t hi, lo;
    } commonNaNT;

  private:
    union
    {
        uint32_t d32;
        uint32_t d64[2];
        uint32_t d128[4];
    } temp;
    union
    {
        uint8_t  d8;
        uint16_t d16;
        uint32_t d32;
    } utemp;
    union
    {
        uint32_t m_xmm32[32];
        uint16_t m_xmm16[64];
        uint8_t  m_xmm8[128];
        uint64_t m_xmm64[16];
    } __attribute__((aligned(16)));

    union
    {
        uint32_t m_reg32[16];
        uint16_t m_reg16[16 * 2];
        uint8_t  m_reg8[16 * 4];
    };
    union
    {
        void     *m_mem;
        uint32_t *m_mem32;
        uint16_t *m_mem16;
        uint8_t  *m_mem8;
    };

  private:
    const float32  m_float32_one         = 0x3f800000;
    const float16  m_float16_default_nan = 0xFE00;
    float_status_t m_status;

    int   m_current_exception = -1;
    int   m_winnt_limit_cpuid;
    void *m_result_ptr;
    int   m_write_back, m_write_back_dwords, m_write_back_linaddr;

    uint32_t m_mxcsr;
    uint32_t m_esp_mask;

    uint32_t m_memory_size;
    uint32_t m_eflags;
    uint32_t m_laux;
    uint32_t m_lop1, m_lop2, m_lr;
    uint32_t m_phys_eip;
    uint32_t m_last_phys_eip;
    uint32_t m_eip_phys_bias;
    uint32_t m_state_hash;

    uint64_t m_cycles, m_cycle_frame_end;
    int      m_cycles_to_run, m_refill_counter, m_hlt_counter, m_cycle_offset;
    uint32_t m_cr[8], m_dr[8];
    int      m_cpl;

    uint16_t m_seg[16];
    uint32_t m_seg_base[16];
    uint32_t m_seg_limit[16];
    uint32_t m_seg_access[16];
    uint32_t m_seg_valid[16];

    int      m_trace_cache_usage;
    int      m_tlb_shift_read, m_tlb_shift_write;
    uint64_t m_mtrr_fixed[32];
    uint64_t m_mtrr_variable_addr_mask[16];
    uint64_t m_mtrr_deftype;

    uint64_t m_page_attribute_tables;
    uint32_t m_a20_mask;
    uint64_t m_apic_base;
    uint64_t m_tsc_fudge;
    uint32_t m_read_result;
    int      m_intr_line_state;
    int      m_interrupts_blocked;
    int      m_exit_reason;
    uint64_t m_ia32_efer;
    uint32_t m_sysenter[3];

    uint32_t  m_smc_has_code_length;
    uint32_t *m_smc_has_code;

    uint32_t m_tlb_entry_count;
    uint32_t m_tlb_entry_indexes[MAX_TLB_ENTRIES];
    uint8_t  m_tlb_tags[1 << 20];
    uint8_t  m_tlb_attrs[1 << 20];
    void    *m_tlb[1 << 20];

    uint32_t m_partial_sw, m_bits_to_clear;

    uint8_t *m_rawp;
    uint8_t  m_prefetch[16];
    int      m_d_state_hash;
    int      m_seg_prefix[2] = {DS, SS};
    int      m_sse_prefix    = 0;

  public:    // functions
    CPU();
    void create_ftable();

  public:
    int      cpu_access_read8(uint32_t addr, uint32_t tag, int shift);
    int      cpu_access_read16(uint32_t addr, uint32_t tag, int shift);
    int      cpu_access_read32(uint32_t addr, uint32_t tag, int shift);
    int      cpu_access_write8(uint32_t addr, uint32_t data, uint32_t tag, int shift);
    int      cpu_access_write16(uint32_t addr, uint32_t data, uint32_t tag, int shift);
    int      cpu_access_write32(uint32_t addr, uint32_t data, uint32_t tag, int shift);
    int      cpu_access_verify(uint32_t addr, uint32_t end, int shift);
    uint8_t  read8(uint32_t lin);
    uint16_t read16(uint32_t lin);
    uint32_t read32(uint32_t lin);
    void     readmem(uint32_t lin, int bytes);
    void     readphys(uint32_t lin, int bytes);
    uint32_t lin2phys(uint32_t addr);

  public:
    void     cpu_arith8(int op, uint8_t *dest, uint8_t src);
    void     cpu_arith16(int op, uint16_t *dest, uint16_t src);
    void     cpu_arith32(int op, uint32_t *dest, uint32_t src);
    void     cpu_shift8(int op, uint8_t *dest, uint8_t src);
    void     cpu_shift16(int op, uint16_t *dest, uint16_t src);
    void     cpu_shift32(int op, uint32_t *dest, uint32_t src);
    int      cpu_muldiv8(int op, uint32_t src);
    int      cpu_muldiv16(int op, uint32_t src);
    int      cpu_muldiv32(int op, uint32_t src);
    void     cpu_neg8(uint8_t *dest);
    void     cpu_neg16(uint16_t *dest);
    void     cpu_neg32(uint32_t *dest);
    void     cpu_shrd16(uint16_t *dest_ptr, uint16_t src, int count);
    void     cpu_shrd32(uint32_t *dest_ptr, uint32_t src, int count);
    void     cpu_shld16(uint16_t *dest_ptr, uint16_t src, int count);
    void     cpu_shld32(uint32_t *dest_ptr, uint32_t src, int count);
    void     cpu_inc8(uint8_t *dest_ptr);
    void     cpu_inc16(uint16_t *dest_ptr);
    void     cpu_inc32(uint32_t *dest_ptr);
    void     cpu_dec8(uint8_t *dest_ptr);
    void     cpu_dec16(uint16_t *dest_ptr);
    void     cpu_dec32(uint32_t *dest_ptr);
    void     cpu_not8(uint8_t *dest_ptr);
    void     cpu_not16(uint16_t *dest_ptr);
    void     cpu_not32(uint32_t *dest_ptr);
    uint8_t  cpu_imul8(uint8_t op1, uint8_t op2);
    uint16_t cpu_imul16(uint16_t op1, uint16_t op2);
    uint32_t cpu_imul32(uint32_t op1, uint32_t op2);
    void     cpu_cmpxchg8(uint8_t *op1, uint8_t op2);
    void     cpu_cmpxchg16(uint16_t *op1, uint16_t op2);
    void     cpu_cmpxchg32(uint32_t *op1, uint32_t op2);
    void     xadd8(uint8_t *op1, uint8_t *op2);
    void     xadd16(uint16_t *op1, uint16_t *op2);
    void     xadd32(uint32_t *op1, uint32_t *op2);

  public:
    void     bt16(uint16_t a, int shift);
    void     bt32(uint32_t a, int shift);
    void     bts16(uint16_t *a, int shift);
    void     bts32(uint32_t *a, int shift);
    void     btc16(uint16_t *a, int shift);
    void     btc32(uint32_t *a, int shift);
    void     btr16(uint16_t *a, int shift);
    void     btr32(uint32_t *a, int shift);
    uint16_t bsf16(uint16_t src, uint16_t old);
    uint32_t bsf32(uint32_t src, uint32_t old);
    uint16_t bsr16(uint16_t src, uint16_t old);
    uint32_t bsr32(uint32_t src, uint32_t old);

  public:
    void    cpu_set_a20(int a20_enabled);
    int     cpu_init_mem(int size);
    int     cpu_interrupts_masked(void);
    itick_t cpu_get_cycles(void);
    int     cpu_run(int cycles);
    void    cpu_raise_intr_line(void);
    void    cpu_lower_intr_line(void);
    void    cpu_request_fast_return(int reason);
    void    cpu_cancel_execution_cycle(int reason);
    void   *cpu_get_ram_ptr(void);
    int     cpu_add_rom(int addr, int size, void *data);
    int     cpu_get_exit_reason(void);
    void    cpu_set_break(void);
    void    cpu_reset(void);
    int     cpu_apic_connected(void);
    void    cpu_state(void);
    int     cpu_init(void);
    void    cpu_init_dma(uint32_t page);
    void    cpu_write_mem(uint32_t addr, void *data, uint32_t length);
    void    cpu_debug(void);

  public:
    int      tss_is_16(int type);
    int      load_tss_from_task_gate(uint32_t *seg, struct seg_desc *info);
    int      get_tss_esp(int level, int *dest);
    int      get_tss_ss(int level, int *dest);
    int      do_task_switch(int sel, struct seg_desc *info, int type, int eip);
    int      cpu_interrupt(int vector, int error_code, int type, int eip_to_push);
    void     cpu_exception(int vec, int code);
    int      jmpf(uint32_t eip, uint32_t cs, uint32_t eip_after);
    uint32_t call_gate_read_param32(uint32_t addr, uint32_t *dest, int mask);
    uint16_t call_gate_read_param16(uint32_t addr, uint32_t *dest, int mask);
    int      callf(uint32_t eip, uint32_t cs, uint32_t oldeip, int is32);
    void     iret_handle_seg(int x);
    void     f_reload_cs_base();
    int      iret(uint32_t tss_eip, int is32);
    int      retf(int adjust, int is32);
    void     reload_cs_base(void);
    int      f_sysenter(void);
    int      sysexit(void);

  public:
    void    float_raise(float_status_t *status, int flags);
    int     get_exception_flags(const float_status_t *status);
    int     float_exception_masked(const float_status_t *status, int flag);
    int     get_float_rounding_mode(const float_status_t *status);
    int     get_float_rounding_precision(const float_status_t *status);
    int     get_float_nan_handling_mode(const float_status_t *status);
    void    set_float_rounding_up(float_status_t *status);
    int     get_denormals_are_zeros(const float_status_t *status);
    int     get_flush_underflow_to_zero(const float_status_t *status);
    float32 float32_round_to_int(float32 a, float_status_t *status);
    float32 float32_fmadd(float32 a, float32 b, float32 c, float_status_t *status);
    float32 float32_fmsub(float32 a, float32 b, float32 c, float_status_t *status);
    float32 float32_fnmadd(float32 a, float32 b, float32 c, float_status_t *status);
    float32 float32_fnmsub(float32 a, float32 b, float32 c, float_status_t *status);
    int     float32_compare(float32 a, float32 b, float_status_t *status);
    int     float32_compare_quiet(float32 a, float32 b, float_status_t *status);
    float64 float64_round_to_int(float64 a, float_status_t *status);
    float64 float64_fmadd(float64 a, float64 b, float64 c, float_status_t *status);
    float64 float64_fmsub(float64 a, float64 b, float64 c, float_status_t *status);
    float64 float64_fnmadd(float64 a, float64 b, float64 c, float_status_t *status);
    float64 float64_fnmsub(float64 a, float64 b, float64 c, float_status_t *status);
    int     float64_compare(float64 a, float64 b, float_status_t *status);
    int     float64_compare_quiet(float64 a, float64 b, float_status_t *status);
    int     floatx80_compare(floatx80 a, floatx80 b, float_status_t *status);
    int     floatx80_compare_quiet(floatx80 a, floatx80 b, float_status_t *status);
    void    floatx80_abs(floatx80 *reg);
    void    floatx80_chs(floatx80 *reg);

  public:
    uint32_t rw(void);
    uint32_t rd(void);
    uint32_t rv(void);
    uint32_t rvs(void);

    int  find_instruction_length(int max_bytes);
    int  parse_modrm(struct decoded_instruction *i, uint8_t modrm, int is8);
    int  swap_rm_reg(int flags);
    int  decode_invalid(struct decoded_instruction *i);
    int  decode_invalid0F(struct decoded_instruction *i);
    int  decode_0F(struct decoded_instruction *i);
    int  decode_prefix(struct decoded_instruction *i);
    int  decode_jcc8(struct decoded_instruction *i);
    int  decode_jccv(struct decoded_instruction *i);
    int  decode_cmov(struct decoded_instruction *i);
    int  decode_setcc(struct decoded_instruction *i);
    int  decode_mov_rbib(struct decoded_instruction *i);
    int  decode_mov_rviv(struct decoded_instruction *i);
    int  decode_push_rv(struct decoded_instruction *i);
    int  decode_pop_rv(struct decoded_instruction *i);
    int  decode_push_sv(struct decoded_instruction *i);
    int  decode_pop_sv(struct decoded_instruction *i);
    int  decode_inc_rv(struct decoded_instruction *i);
    int  decode_dec_rv(struct decoded_instruction *i);
    int  decode_fpu(struct decoded_instruction *i);
    int  decode_arith_00(struct decoded_instruction *i);
    int  decode_arith_01(struct decoded_instruction *i);
    int  decode_arith_02(struct decoded_instruction *i);
    int  decode_arith_03(struct decoded_instruction *i);
    int  decode_arith_04(struct decoded_instruction *i);
    int  decode_arith_05(struct decoded_instruction *i);
    int  decode_xchg(struct decoded_instruction *i);
    int  decode_bswap(struct decoded_instruction *i);
    int  decode_ud(struct decoded_instruction *i);
    int  decode_27(struct decoded_instruction *i);
    int  decode_2F(struct decoded_instruction *i);
    int  decode_37(struct decoded_instruction *i);
    int  decode_3F(struct decoded_instruction *i);
    int  decode_38(struct decoded_instruction *i);
    int  decode_39(struct decoded_instruction *i);
    int  decode_3A(struct decoded_instruction *i);
    int  decode_3B(struct decoded_instruction *i);
    int  decode_3C(struct decoded_instruction *i);
    int  decode_3D(struct decoded_instruction *i);
    int  decode_60(struct decoded_instruction *i);
    int  decode_61(struct decoded_instruction *i);
    int  decode_62(struct decoded_instruction *i);
    int  decode_63(struct decoded_instruction *i);
    int  decode_68(struct decoded_instruction *i);
    int  decode_69(struct decoded_instruction *i);
    int  decode_6A(struct decoded_instruction *i);
    int  decode_6B(struct decoded_instruction *i);
    int  decode_6C(struct decoded_instruction *i);
    int  decode_6D(struct decoded_instruction *i);
    int  decode_6E(struct decoded_instruction *i);
    int  decode_6F(struct decoded_instruction *i);
    int  decode_80(struct decoded_instruction *i);
    int  decode_81(struct decoded_instruction *i);
    int  decode_83(struct decoded_instruction *i);
    int  decode_84(struct decoded_instruction *i);
    int  decode_85(struct decoded_instruction *i);
    int  decode_86(struct decoded_instruction *i);
    int  decode_87(struct decoded_instruction *i);
    int  decode_88(struct decoded_instruction *i);
    int  decode_89(struct decoded_instruction *i);
    int  decode_8A(struct decoded_instruction *i);
    int  decode_8B(struct decoded_instruction *i);
    int  decode_8C(struct decoded_instruction *i);
    int  decode_8D(struct decoded_instruction *i);
    int  decode_8E(struct decoded_instruction *i);
    int  decode_8F(struct decoded_instruction *i);
    int  decode_90(struct decoded_instruction *i);
    int  decode_98(struct decoded_instruction *i);
    int  decode_99(struct decoded_instruction *i);
    int  decode_9A(struct decoded_instruction *i);
    int  decode_9B(struct decoded_instruction *i);
    int  decode_9C(struct decoded_instruction *i);
    int  decode_9D(struct decoded_instruction *i);
    int  decode_9E(struct decoded_instruction *i);
    int  decode_9F(struct decoded_instruction *i);
    int  decode_A0(struct decoded_instruction *i);
    int  decode_A1(struct decoded_instruction *i);
    int  decode_A2(struct decoded_instruction *i);
    int  decode_A3(struct decoded_instruction *i);
    int  decode_A4(struct decoded_instruction *i);
    int  decode_A5(struct decoded_instruction *i);
    int  decode_A6(struct decoded_instruction *i);
    int  decode_A7(struct decoded_instruction *i);
    int  decode_A8(struct decoded_instruction *i);
    int  decode_A9(struct decoded_instruction *i);
    int  decode_AA(struct decoded_instruction *i);
    int  decode_AB(struct decoded_instruction *i);
    int  decode_AC(struct decoded_instruction *i);
    int  decode_AD(struct decoded_instruction *i);
    int  decode_AE(struct decoded_instruction *i);
    int  decode_AF(struct decoded_instruction *i);
    int  decode_C0(struct decoded_instruction *i);
    int  decode_C1(struct decoded_instruction *i);
    int  decode_C2(struct decoded_instruction *i);
    int  decode_C3(struct decoded_instruction *i);
    int  decode_C4(struct decoded_instruction *i);
    int  decode_C5(struct decoded_instruction *i);
    int  decode_C6(struct decoded_instruction *i);
    int  decode_C7(struct decoded_instruction *i);
    int  decode_C8(struct decoded_instruction *i);
    int  decode_C9(struct decoded_instruction *i);
    int  decode_CA(struct decoded_instruction *i);
    int  decode_CB(struct decoded_instruction *i);
    int  decode_CC(struct decoded_instruction *i);
    int  decode_CD(struct decoded_instruction *i);
    int  decode_CE(struct decoded_instruction *i);
    int  decode_CF(struct decoded_instruction *i);
    int  decode_D0(struct decoded_instruction *i);
    int  decode_D1(struct decoded_instruction *i);
    int  decode_D2(struct decoded_instruction *i);
    int  decode_D3(struct decoded_instruction *i);
    int  decode_D4(struct decoded_instruction *i);
    int  decode_D5(struct decoded_instruction *i);
    int  decode_D7(struct decoded_instruction *i);
    int  decode_E0(struct decoded_instruction *i);
    int  decode_E1(struct decoded_instruction *i);
    int  decode_E2(struct decoded_instruction *i);
    int  decode_E3(struct decoded_instruction *i);
    int  decode_E4(struct decoded_instruction *i);
    int  decode_E5(struct decoded_instruction *i);
    int  decode_E6(struct decoded_instruction *i);
    int  decode_E7(struct decoded_instruction *i);
    int  decode_E8(struct decoded_instruction *i);
    int  decode_E9(struct decoded_instruction *i);
    int  decode_EA(struct decoded_instruction *i);
    int  decode_EB(struct decoded_instruction *i);
    int  decode_EC(struct decoded_instruction *i);
    int  decode_ED(struct decoded_instruction *i);
    int  decode_EE(struct decoded_instruction *i);
    int  decode_EF(struct decoded_instruction *i);
    int  decode_F4(struct decoded_instruction *i);
    int  decode_F5(struct decoded_instruction *i);
    int  decode_F6(struct decoded_instruction *i);
    int  decode_F7(struct decoded_instruction *i);
    int  decode_F8(struct decoded_instruction *i);
    int  decode_F9(struct decoded_instruction *i);
    int  decode_FA(struct decoded_instruction *i);
    int  decode_FB(struct decoded_instruction *i);
    int  decode_FC(struct decoded_instruction *i);
    int  decode_FD(struct decoded_instruction *i);
    int  decode_FE(struct decoded_instruction *i);
    int  decode_FF(struct decoded_instruction *i);
    int  decode_0F00(struct decoded_instruction *i);
    int  decode_0F01(struct decoded_instruction *i);
    int  decode_0F02(struct decoded_instruction *i);
    int  decode_0F03(struct decoded_instruction *i);
    int  decode_0F06(struct decoded_instruction *i);
    int  decode_0F09(struct decoded_instruction *i);
    int  decode_0F0B(struct decoded_instruction *i);
    int  decode_sse10_17(struct decoded_instruction *i);
    int  decode_0F18(struct decoded_instruction *i);
    int  decode_0F1F(struct decoded_instruction *i);
    int  decode_0F20(struct decoded_instruction *i);
    int  decode_0F21(struct decoded_instruction *i);
    int  decode_0F22(struct decoded_instruction *i);
    int  decode_0F23(struct decoded_instruction *i);
    int  decode_sse28_2F(struct decoded_instruction *i);
    int  decode_0F30(struct decoded_instruction *i);
    int  decode_0F31(struct decoded_instruction *i);
    int  decode_0F32(struct decoded_instruction *i);
    int  decode_0F38(struct decoded_instruction *i);
    int  decode_sysenter_sysexit(struct decoded_instruction *i);
    int  decode_sse50_57(struct decoded_instruction *i);
    int  decode_sse58_5F(struct decoded_instruction *i);
    int  decode_sse60_67(struct decoded_instruction *i);
    int  decode_sse68_6F(struct decoded_instruction *i);
    int  decode_sse70_76(struct decoded_instruction *i);
    int  decode_0F77(struct decoded_instruction *i);
    int  decode_sse7C_7D(struct decoded_instruction *i);
    int  decode_sse7E_7F(struct decoded_instruction *i);
    int  decode_0FA0(struct decoded_instruction *i);
    int  decode_0FA1(struct decoded_instruction *i);
    int  decode_0FA2(struct decoded_instruction *i);
    int  decode_0FA3(struct decoded_instruction *i);
    int  decode_0FA4(struct decoded_instruction *i);
    int  decode_0FA5(struct decoded_instruction *i);
    int  decode_0FA8(struct decoded_instruction *i);
    int  decode_0FA9(struct decoded_instruction *i);
    int  decode_0FAB(struct decoded_instruction *i);
    int  decode_0FAC(struct decoded_instruction *i);
    int  decode_0FAD(struct decoded_instruction *i);
    int  decode_0FAE(struct decoded_instruction *i);
    int  decode_0FAF(struct decoded_instruction *i);
    int  decode_0FB0(struct decoded_instruction *i);
    int  decode_0FB1(struct decoded_instruction *i);
    int  decode_0FB2(struct decoded_instruction *i);
    int  decode_0FB3(struct decoded_instruction *i);
    int  decode_0FB4(struct decoded_instruction *i);
    int  decode_0FB5(struct decoded_instruction *i);
    int  decode_0FB6(struct decoded_instruction *i);
    int  decode_0FB7(struct decoded_instruction *i);
    int  decode_0FBA(struct decoded_instruction *i);
    int  decode_0FBB(struct decoded_instruction *i);
    int  decode_0FBC(struct decoded_instruction *i);
    int  decode_0FBD(struct decoded_instruction *i);
    int  decode_0FBE(struct decoded_instruction *i);
    int  decode_0FBF(struct decoded_instruction *i);
    int  decode_0FC0(struct decoded_instruction *i);
    int  decode_0FC1(struct decoded_instruction *i);
    int  decode_0FC7(struct decoded_instruction *i);
    int  decode_sseC2_C6(struct decoded_instruction *i);
    int  decode_sseD0_D7(struct decoded_instruction *i);
    int  decode_sseD8_DF(struct decoded_instruction *i);
    int  decode_sseE0_E7(struct decoded_instruction *i);
    int  decode_sseE8_EF(struct decoded_instruction *i);
    int  decode_sseF1_F7(struct decoded_instruction *i);
    int  decode_sseF8_FE(struct decoded_instruction *i);
    int  cpu_decode(struct trace_info *info, struct decoded_instruction *i);
    void set_smc(int length, uint32_t lin);

  public:
    int      cpu_get_sf(void);
    void     cpu_set_sf(int set);
    int      cpu_get_pf(void);
    void     cpu_set_pf(int set);
    int      cpu_get_oac(void);
    int      cpu_get_of(void);
    void     cpu_set_of(int set);
    int      cpu_get_af(void);
    void     cpu_set_af(int set);
    int      cpu_get_cf(void);
    void     cpu_set_cf(int set);
    void     cpu_set_zf(int set);
    uint32_t cpu_get_eflags(void);
    int      cpu_cond(int val);
    void     cpu_set_eflags(uint32_t eflags);

  public:
    void     fpu_set_control_word(uint16_t control_word);
    void     fpu_state(void);
    uint16_t fpu_get_status_word(void);
    int      is_denormal(uint16_t exponent, uint64_t mantissa);
    int      is_pseudo_denormal(uint16_t exponent, uint64_t mantissa);
    int      is_zero(uint16_t exponent, uint64_t mantissa);
    int      is_zero_any_sign(uint16_t exponent, uint64_t mantissa);
    int      is_negative(uint16_t exponent, uint64_t mantissa);
    int      is_invalid(uint16_t exponent, uint64_t mantissa);
    int      is_infinity(uint16_t exponent, uint64_t mantissa);
    int      is_nan(uint16_t exponent, uint64_t mantissa);
    int      fpu_get_tag_from_value(floatx80 *f);
    int      fpu_get_tag(int st);
    void     fpu_set_tag(int st, int v);
    int      fpu_exception_raised(int flags);
    void     fpu_stack_fault(void);
    void     fpu_commit_sw(void);
    int      fpu_check_exceptions2(int commit_sw);
    int      fpu_check_exceptions(void);
    void     fninit(void);
    int      fpu_nm_check(void);

    floatx80 *fpu_get_st_ptr(int st);
    floatx80  fpu_get_st(int st);

    void   fpu_set_st(int st, floatx80 data);
    int    fpu_check_stack_overflow(int st);
    int    fpu_check_stack_underflow(int st, int commit_sw);
    int    fpu_exception_masked(int excep);
    int    fpu_push(floatx80 data);
    void   fpu_pop();
    void   fpu_update_pointers(uint32_t opcode);
    void   fpu_update_pointers2(uint32_t opcode, uint32_t virtaddr, uint32_t seg);
    int    write_float32(uint32_t linaddr, float32 src);
    int    write_float64(uint32_t linaddr, float64 dest);
    int    fpu_check_push(void);
    int    fpu_store_f80(uint32_t linaddr, floatx80 *data);
    int    fpu_read_f80(uint32_t linaddr, floatx80 *data);
    int    fpu_fcom(floatx80 op1, floatx80 op2, int unordered);
    int    fpu_fcomi(floatx80 op1, floatx80 op2, int unordered);
    int    fstenv(uint32_t linaddr, int code16);
    int    fldenv(uint32_t linaddr, int code16);
    void   fpu_watchpoint(void);
    void   fpu_watchpoint2(void);
    int    fpu_reg_op(struct decoded_instruction *i, uint32_t flags);
    int    fpu_mem_op(struct decoded_instruction *i, uint32_t virtaddr, uint32_t seg);
    int    fpu_fxsave(uint32_t linaddr);
    int    fpu_fxrstor(uint32_t linaddr);
    int    fpu_fwait(void);
    void   fpu_init(void);
    double f80_to_double(floatx80 *f80);
    void   fpu_debug(void);
    void   printFloat80(floatx80 *arg);
    void  *fpu_get_st_ptr1(void);

  public:
    int      cpu_io_check_access(uint32_t port, int size);
    void     cpu_outb(uint32_t port, uint32_t data);
    void     cpu_outw(uint32_t port, uint32_t data);
    void     cpu_outd(uint32_t port, uint32_t data);
    uint32_t cpu_inb(uint32_t port);
    uint32_t cpu_inw(uint32_t port);
    uint32_t cpu_ind(uint32_t port);

  public:
    int      cpu_set_cpuid(struct cpu_config *x);
    void     cpuid(void);
    int      rdmsr(uint32_t index, uint32_t *high, uint32_t *low);
    int      wrmsr(uint32_t index, uint32_t high, uint32_t low);
    int      pushf(void);
    int      pushfd(void);
    int      popf(void);
    int      popfd(void);
    int      ltr(uint32_t selector);
    int      lldt(uint32_t selector);
    uint32_t lar(uint16_t op1, uint32_t op2);
    uint32_t lsl(uint16_t op, uint32_t op2);
    void     verify_segment_access(uint16_t sel, int write);
    void     arpl(uint16_t *ptr, uint16_t reg);

  public:
    void     cpu_mmu_tlb_flush(void);
    void     cpu_mmu_tlb_flush_nonglobal(void);
    void     cpu_set_tlb_entry(uint32_t lin, uint32_t phys, void *ptr, int user, int write, int global, int nx);
    uint32_t cpu_read_phys(uint32_t addr);
    void     cpu_write_phys(uint32_t addr, uint32_t data);
    int      cpu_mmu_translate(uint32_t lin, int shift);
    void     cpu_mmu_tlb_invalidate(uint32_t lin);

  public:
    void     interrupt_guard(void);
    uint32_t cpu_get_linaddr(uint32_t i, optype j);
    uint32_t cpu_get_virtaddr(uint32_t i, optype j);
    void     cpu_execute(void);

    optype op_ud_exception(optype i);
    optype op_trace_end(optype i);
    optype op_nop(optype i);
    optype op_jmp_r16(optype i);
    optype op_jmp_r32(optype i);
    optype op_jmp_e16(optype i);
    optype op_jmp_e32(optype i);
    optype op_call_r16(optype i);
    optype op_call_r32(optype i);
    optype op_call_e16(optype i);
    optype op_call_e32(optype i);
    optype op_jmp_rel32(optype i);
    optype op_jmp_rel16(optype i);
    optype op_jmpf(optype i);
    optype op_jmpf_e16(optype i);
    optype op_jmpf_e32(optype i);
    optype op_callf16_ap(optype i);
    optype op_callf32_ap(optype i);
    optype op_callf_e16(optype i);
    optype op_callf_e32(optype i);
    optype op_retf16(optype i);
    optype op_retf32(optype i);
    optype op_iret16(optype i);
    optype op_iret32(optype i);
    optype op_loop_rel16(optype i);
    optype op_loop_rel32(optype i);
    optype op_loopz_rel16(optype i);
    optype op_loopz_rel32(optype i);
    optype op_loopnz_rel16(optype i);
    optype op_loopnz_rel32(optype i);
    optype op_jecxz_rel16(optype i);
    optype op_jecxz_rel32(optype i);
    optype op_jo16(optype i);
    optype op_jo32(optype i);
    optype op_jno16(optype i);
    optype op_jno32(optype i);
    optype op_jb16(optype i);
    optype op_jb32(optype i);
    optype op_jnb16(optype i);
    optype op_jnb32(optype i);
    optype op_jz16(optype i);
    optype op_jz32(optype i);
    optype op_jnz16(optype i);
    optype op_jnz32(optype i);
    optype op_jbe16(optype i);
    optype op_jbe32(optype i);
    optype op_jnbe16(optype i);
    optype op_jnbe32(optype i);
    optype op_js16(optype i);
    optype op_js32(optype i);
    optype op_jns16(optype i);
    optype op_jns32(optype i);
    optype op_jp16(optype i);
    optype op_jp32(optype i);
    optype op_jnp16(optype i);
    optype op_jnp32(optype i);
    optype op_jl16(optype i);
    optype op_jl32(optype i);
    optype op_jnl16(optype i);
    optype op_jnl32(optype i);
    optype op_jle16(optype i);
    optype op_jle32(optype i);
    optype op_jnle16(optype i);
    optype op_jnle32(optype i);
    optype op_call_j16(optype i);
    optype op_call_j32(optype i);
    optype op_ret16(optype i);
    optype op_ret32(optype i);
    optype op_ret16_iw(optype i);
    optype op_ret32_iw(optype i);
    optype op_int(optype i);
    optype op_into(optype i);
    optype op_push_r16(optype i);
    optype op_push_i16(optype i);
    optype op_push_e16(optype i);
    optype op_push_r32(optype i);
    optype op_push_i32(optype i);
    optype op_push_e32(optype i);
    optype op_pop_r16(optype i);
    optype op_pop_e16(optype i);
    optype op_pop_e32(optype i);
    optype op_pop_r32(optype i);
    optype op_push_s16(optype i);
    optype op_push_s32(optype i);
    optype op_pop_s16(optype i);
    optype op_pop_s32(optype i);
    optype op_pusha(optype i);
    optype op_pushad(optype i);
    optype op_popa(optype i);
    optype op_popad(optype i);
    optype op_arith_r8r8(optype i);
    optype op_arith_r8i8(optype i);
    optype op_arith_r8e8(optype i);
    optype op_arith_e8r8(optype i);
    optype op_arith_e8i8(optype i);
    optype op_arith_r16r16(optype i);
    optype op_arith_r16i16(optype i);
    optype op_arith_r16e16(optype i);
    optype op_arith_e16r16(optype i);
    optype op_arith_e16i16(optype i);
    optype op_arith_r32r32(optype i);
    optype op_arith_r32i32(optype i);
    optype op_arith_r32e32(optype i);
    optype op_arith_e32r32(optype i);
    optype op_arith_e32i32(optype i);
    optype op_shift_r8cl(optype i);
    optype op_shift_r8i8(optype i);
    optype op_shift_e8cl(optype i);
    optype op_shift_e8i8(optype i);
    optype op_shift_r16cl(optype i);
    optype op_shift_r16i16(optype i);
    optype op_shift_e16cl(optype i);
    optype op_shift_e16i16(optype i);
    optype op_shift_r32cl(optype i);
    optype op_shift_r32i32(optype i);
    optype op_shift_e32cl(optype i);
    optype op_shift_e32i32(optype i);
    optype op_cmp_e8r8(optype i);
    optype op_cmp_r8r8(optype i);
    optype op_cmp_r8e8(optype i);
    optype op_cmp_r8i8(optype i);
    optype op_cmp_e8i8(optype i);
    optype op_cmp_e16r16(optype i);
    optype op_cmp_r16r16(optype i);
    optype op_cmp_r16e16(optype i);
    optype op_cmp_r16i16(optype i);
    optype op_cmp_e16i16(optype i);
    optype op_cmp_e32r32(optype i);
    optype op_cmp_r32r32(optype i);
    optype op_cmp_r32e32(optype i);
    optype op_cmp_r32i32(optype i);
    optype op_cmp_e32i32(optype i);
    optype op_test_e8r8(optype i);
    optype op_test_r8r8(optype i);
    optype op_test_r8e8(optype i);
    optype op_test_r8i8(optype i);
    optype op_test_e8i8(optype i);
    optype op_test_e16r16(optype i);
    optype op_test_r16r16(optype i);
    optype op_test_r16e16(optype i);
    optype op_test_r16i16(optype i);
    optype op_test_e16i16(optype i);
    optype op_test_e32r32(optype i);
    optype op_test_r32r32(optype i);
    optype op_test_r32e32(optype i);
    optype op_test_r32i32(optype i);
    optype op_test_e32i32(optype i);
    optype op_inc_r8(optype i);
    optype op_inc_e8(optype i);
    optype op_inc_r16(optype i);
    optype op_inc_e16(optype i);
    optype op_inc_r32(optype i);
    optype op_inc_e32(optype i);
    optype op_dec_r8(optype i);
    optype op_dec_e8(optype i);
    optype op_dec_r16(optype i);
    optype op_dec_e16(optype i);
    optype op_dec_r32(optype i);
    optype op_dec_e32(optype i);
    optype op_not_r8(optype i);
    optype op_not_e8(optype i);
    optype op_not_r16(optype i);
    optype op_not_e16(optype i);
    optype op_not_r32(optype i);
    optype op_not_e32(optype i);
    optype op_neg_r8(optype i);
    optype op_neg_e8(optype i);
    optype op_neg_r16(optype i);
    optype op_neg_e16(optype i);
    optype op_neg_r32(optype i);
    optype op_neg_e32(optype i);
    optype op_muldiv_r8(optype i);
    optype op_muldiv_e8(optype i);
    optype op_muldiv_r16(optype i);
    optype op_muldiv_e16(optype i);
    optype op_muldiv_r32(optype i);
    optype op_muldiv_e32(optype i);
    optype op_imul_r16r16i16(optype i);
    optype op_imul_r16e16i16(optype i);
    optype op_imul_r32r32i32(optype i);
    optype op_imul_r32e32i32(optype i);
    optype op_imul_r16r16(optype i);
    optype op_imul_r32r32(optype i);
    optype op_imul_r16e16(optype i);
    optype op_imul_r32e32(optype i);
    optype op_shrd_r16r16i8(optype i);
    optype op_shrd_r32r32i8(optype i);
    optype op_shrd_r16r16cl(optype i);
    optype op_shrd_r32r32cl(optype i);
    optype op_shrd_e16r16i8(optype i);
    optype op_shrd_e32r32i8(optype i);
    optype op_shrd_e16r16cl(optype i);
    optype op_shrd_e32r32cl(optype i);
    optype op_shld_r16r16i8(optype i);
    optype op_shld_r32r32i8(optype i);
    optype op_shld_r16r16cl(optype i);
    optype op_shld_r32r32cl(optype i);
    optype op_shld_e16r16i8(optype i);
    optype op_shld_e32r32i8(optype i);
    optype op_shld_e16r16cl(optype i);
    optype op_shld_e32r32cl(optype i);
    optype op_out_i8al(optype i);
    optype op_out_i8ax(optype i);
    optype op_out_i8eax(optype i);
    optype op_in_i8al(optype i);
    optype op_in_i8ax(optype i);
    optype op_in_i8eax(optype i);
    optype op_out_dxal(optype i);
    optype op_out_dxax(optype i);
    optype op_out_dxeax(optype i);
    optype op_in_dxal(optype i);
    optype op_in_dxax(optype i);
    optype op_in_dxeax(optype i);
    optype op_mov_r8i8(optype i);
    optype op_mov_r16i16(optype i);
    optype op_mov_r32i32(optype i);
    optype op_mov_r8e8(optype i);
    optype op_mov_r8r8(optype i);
    optype op_mov_e8r8(optype i);
    optype op_mov_e8i8(optype i);
    optype op_mov_r16e16(optype i);
    optype op_mov_r16r16(optype i);
    optype op_mov_e16r16(optype i);
    optype op_mov_e16i16(optype i);
    optype op_mov_r32e32(optype i);
    optype op_mov_r32r32(optype i);
    optype op_mov_e32r32(optype i);
    optype op_mov_e32i32(optype i);
    optype op_mov_s16r16(optype i);
    optype op_mov_s16e16(optype i);
    optype op_mov_e16s16(optype i);
    optype op_mov_r16s16(optype i);
    optype op_mov_r32s16(optype i);
    optype op_mov_eaxm32(optype i);
    optype op_mov_axm16(optype i);
    optype op_mov_alm8(optype i);
    optype op_mov_m32eax(optype i);
    optype op_mov_m16ax(optype i);
    optype op_mov_m8al(optype i);
    optype op_cmov_r16e16(optype i);
    optype op_cmov_r16r16(optype i);
    optype op_cmov_r32e32(optype i);
    optype op_cmov_r32r32(optype i);
    optype op_setcc_e8(optype i);
    optype op_setcc_r8(optype i);
    optype op_lea_r16e16(optype i);
    optype op_lea_r32e32(optype i);
    optype op_lds_r16e16(optype i);
    optype op_lds_r32e32(optype i);
    optype op_les_r16e16(optype i);
    optype op_les_r32e32(optype i);
    optype op_lss_r16e16(optype i);
    optype op_lss_r32e32(optype i);
    optype op_lfs_r16e16(optype i);
    optype op_lfs_r32e32(optype i);
    optype op_lgs_r16e16(optype i);
    optype op_lgs_r32e32(optype i);
    optype op_xchg_r8r8(optype i);
    optype op_xchg_r16r16(optype i);
    optype op_xchg_r32r32(optype i);
    optype op_xchg_r8e8(optype i);
    optype op_xchg_r16e16(optype i);
    optype op_xchg_r32e32(optype i);
    optype op_cmpxchg_r8r8(optype i);
    optype op_cmpxchg_e8r8(optype i);
    optype op_cmpxchg_r16r16(optype i);
    optype op_cmpxchg_e16r16(optype i);
    optype op_cmpxchg_r32r32(optype i);
    optype op_cmpxchg_e32r32(optype i);
    optype op_cmpxchg8b_e32(optype i);
    optype op_xadd_r8r8(optype i);
    optype op_xadd_r8e8(optype i);
    optype op_xadd_r16r16(optype i);
    optype op_xadd_r16e16(optype i);
    optype op_xadd_r32r32(optype i);
    optype op_xadd_r32e32(optype i);
    optype op_bound_r16e16(optype i);
    optype op_bound_r32e32(optype i);
    optype op_daa(optype i);
    optype op_das(optype i);
    optype op_aaa(optype i);
    optype op_aas(optype i);
    optype op_aam(optype i);
    optype op_aad(optype i);
    optype op_bt_r16(optype i);
    optype op_bts_r16(optype i);
    optype op_btc_r16(optype i);
    optype op_btr_r16(optype i);
    optype op_bt_r32(optype i);
    optype op_bts_r32(optype i);
    optype op_btc_r32(optype i);
    optype op_btr_r32(optype i);
    optype op_bt_e16(optype i);
    optype op_bt_e32(optype i);
    optype op_bts_e16(optype i);
    optype op_btc_e16(optype i);
    optype op_btr_e16(optype i);
    optype op_bts_e32(optype i);
    optype op_btc_e32(optype i);
    optype op_btr_e32(optype i);
    optype op_bsf_r16r16(optype i);
    optype op_bsf_r16e16(optype i);
    optype op_bsf_r32r32(optype i);
    optype op_bsf_r32e32(optype i);
    optype op_bsr_r16r16(optype i);
    optype op_bsr_r16e16(optype i);
    optype op_bsr_r32r32(optype i);
    optype op_bsr_r32e32(optype i);
    optype op_cli(optype i);
    optype op_sti(optype i);
    optype op_cld(optype i);
    optype op_std(optype i);
    optype op_cmc(optype i);
    optype op_clc(optype i);
    optype op_stc(optype i);
    optype op_hlt(optype i);
    optype op_cpuid(optype i);
    optype op_rdmsr(optype i);
    optype op_wrmsr(optype i);
    optype op_rdtsc(optype i);
    optype op_pushf(optype i);
    optype op_pushfd(optype i);
    optype op_popf(optype i);
    optype op_popfd(optype i);
    optype op_cbw(optype i);
    optype op_cwde(optype i);
    optype op_cwd(optype i);
    optype op_cdq(optype i);
    optype op_lahf(optype i);
    optype op_sahf(optype i);
    optype op_enter16(optype i);
    optype op_enter32(optype i);
    optype op_leave16(optype i);
    optype op_leave32(optype i);
    optype op_sgdt_e32(optype i);
    optype op_sidt_e32(optype i);
    optype op_str_sldt_e16(optype i);
    optype op_str_sldt_r16(optype i);
    optype op_lgdt_e16(optype i);
    optype op_lgdt_e32(optype i);
    optype op_lidt_e16(optype i);
    optype op_lidt_e32(optype i);
    optype op_smsw_r16(optype i);
    optype op_smsw_r32(optype i);
    optype op_smsw_e16(optype i);
    optype op_lmsw_r16(optype i);
    optype op_lmsw_e16(optype i);
    optype op_invlpg_e8(optype i);
    optype op_mov_r32cr(optype i);
    optype op_mov_crr32(optype i);
    optype op_mov_r32dr(optype i);
    optype op_mov_drr32(optype i);
    optype op_ltr_e16(optype i);
    optype op_ltr_r16(optype i);
    optype op_lldt_e16(optype i);
    optype op_lldt_r16(optype i);
    optype op_lar_r16e16(optype i);
    optype op_lar_r32e32(optype i);
    optype op_lar_r16r16(optype i);
    optype op_lar_r32r32(optype i);
    optype op_lsl_r16e16(optype i);
    optype op_lsl_r32e32(optype i);
    optype op_lsl_r16r16(optype i);
    optype op_lsl_r32r32(optype i);
    optype op_arpl_e16(optype i);
    optype op_arpl_r16(optype i);
    optype op_verr_e16(optype i);
    optype op_verr_r16(optype i);
    optype op_verw_e16(optype i);
    optype op_verw_r16(optype i);
    optype op_clts(optype i);
    optype op_wbinvd(optype i);
    optype op_prefetchh(optype i);
    optype op_movzx_r16r8(optype i);
    optype op_movzx_r32r8(optype i);
    optype op_movzx_r32r16(optype i);
    optype op_movzx_r16e8(optype i);
    optype op_movzx_r32e8(optype i);
    optype op_movzx_r32e16(optype i);
    optype op_movsx_r16r8(optype i);
    optype op_movsx_r32r8(optype i);
    optype op_movsx_r32r16(optype i);
    optype op_movsx_r16e8(optype i);
    optype op_movsx_r32e8(optype i);
    optype op_movsx_r32e16(optype i);
    optype op_xlat16(optype i);
    optype op_xlat32(optype i);
    optype op_bswap_r16(optype i);
    optype op_bswap_r32(optype i);
    optype op_fpu_mem(optype i);
    optype op_fpu_reg(optype i);
    optype op_fwait(optype i);
    optype op_sysenter(optype i);
    optype op_sysexit(optype i);
    optype op_sse_10_17(optype i);
    optype op_sse_28_2F(optype i);
    optype op_sse_38(optype i);
    optype op_sse_6638(optype i);
    optype op_sse_50_57(optype i);
    optype op_sse_58_5F(optype i);
    optype op_sse_60_67(optype i);
    optype op_sse_68_6F(optype i);
    optype op_sse_70_76(optype i);
    optype op_sse_7C_7D(optype i);
    optype op_sse_7E_7F(optype i);
    optype op_sse_C2_C6(optype i);
    optype op_sse_D0_D7(optype i);
    optype op_sse_D8_DF(optype i);
    optype op_sse_E0_E7(optype i);
    optype op_sse_E8_EF(optype i);
    optype op_sse_F1_F7(optype i);
    optype op_sse_F8_FE(optype i);
    optype op_ldmxcsr(optype i);
    optype op_stmxcsr(optype i);
    optype op_mfence(optype i);
    optype op_fxsave(optype i);
    optype op_fxrstor(optype i);
    optype op_emms(optype i);
    optype op_movsb16(optype i);
    optype op_movsb32(optype i);
    optype op_movsw16(optype i);
    optype op_movsw32(optype i);
    optype op_movsd16(optype i);
    optype op_movsd32(optype i);
    optype op_stosb16(optype i);
    optype op_stosb32(optype i);
    optype op_stosw16(optype i);
    optype op_stosw32(optype i);
    optype op_stosd16(optype i);
    optype op_stosd32(optype i);
    optype op_scasb16(optype i);
    optype op_scasb32(optype i);
    optype op_scasw16(optype i);
    optype op_scasw32(optype i);
    optype op_scasd16(optype i);
    optype op_scasd32(optype i);
    optype op_insb16(optype i);
    optype op_insb32(optype i);
    optype op_insw16(optype i);
    optype op_insw32(optype i);
    optype op_insd16(optype i);
    optype op_insd32(optype i);
    optype op_outsb16(optype i);
    optype op_outsb32(optype i);
    optype op_outsw16(optype i);
    optype op_outsw32(optype i);
    optype op_outsd16(optype i);
    optype op_outsd32(optype i);
    optype op_cmpsb16(optype i);
    optype op_cmpsb32(optype i);
    optype op_cmpsw16(optype i);
    optype op_cmpsw32(optype i);
    optype op_cmpsd16(optype i);
    optype op_cmpsd32(optype i);
    optype op_lodsb16(optype i);
    optype op_lodsb32(optype i);
    optype op_lodsw16(optype i);
    optype op_lodsw32(optype i);
    optype op_lodsd16(optype i);
    optype op_lodsd32(optype i);

  public:
    int  cpu_prot_set_cr(int cr, uint32_t v);
    void cpu_prot_set_dr(int id, uint32_t val);
    void cpu_prot_update_cpl(void);

  public:
    void     cpu_reload_cs_base(void);
    void     cpu_load_csip_real(uint16_t cs, uint32_t eip);
    void     cpu_load_csip_virtual(uint16_t cs, uint32_t eip);
    int      cpu_load_csip_protected(uint16_t cs, struct seg_desc *info, uint32_t eip);
    void     cpu_seg_load_virtual(int id, uint16_t sel);
    void     cpu_seg_load_real(int id, uint16_t sel);
    int      cpu_seg_load_protected(int id, uint16_t sel, struct seg_desc *info);
    int      cpu_seg_load_descriptor2(int table, uint32_t selector, struct seg_desc *seg, int exception, int code);
    int      cpu_seg_load_descriptor(uint32_t selector, struct seg_desc *seg, int exception, int code);
    int      cpu_seg_get_dpl(int seg);
    uint32_t cpu_seg_get_base(struct seg_desc *info);
    uint32_t cpu_seg_get_limit(struct seg_desc *info);
    uint32_t cpu_seg_gate_target_segment(struct seg_desc *info);
    uint32_t cpu_seg_gate_target_offset(struct seg_desc *info);
    uint32_t cpu_seg_gate_parameter_count(struct seg_desc *info);
    uint32_t cpu_seg_descriptor_address(int tbl, uint16_t sel);
    int      cpu_load_seg_value_mov(int seg, uint16_t val);

  public:
    int float32_eq_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_lt_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_le_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_neq_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_nlt_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_nle_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_eq_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_nge_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_ngt_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_false_quiet(float32 a, float32 b, float_status_t *status);
    int float32_neq_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_ge_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_gt_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_true_quiet(float32 a, float32 b, float_status_t *status);
    int float32_eq_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_lt_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_le_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_neq_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_nlt_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_nle_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_eq_unordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_nge_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_ngt_unordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_false_signalling(float32 a, float32 b, float_status_t *status);
    int float32_neq_ordered_signalling(float32 a, float32 b, float_status_t *status);
    int float32_ge_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_gt_ordered_quiet(float32 a, float32 b, float_status_t *status);
    int float32_true_signalling(float32 a, float32 b, float_status_t *status);
    int float64_eq_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_lt_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_le_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_neq_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_nlt_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_nle_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_eq_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_nge_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_ngt_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_false_quiet(float64 a, float64 b, float_status_t *status);
    int float64_neq_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_ge_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_gt_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_true_quiet(float64 a, float64 b, float_status_t *status);
    int float64_eq_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_lt_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_le_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_neq_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_nlt_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_nle_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_eq_unordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_nge_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_ngt_unordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_false_signalling(float64 a, float64 b, float_status_t *status);
    int float64_neq_ordered_signalling(float64 a, float64 b, float_status_t *status);
    int float64_ge_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_gt_ordered_quiet(float64 a, float64 b, float_status_t *status);
    int float64_true_signalling(float64 a, float64 b, float_status_t *status);

    int  cpu_sse_exception(void);
    int  cpu_mmx_check(void);
    void cpu_update_mxcsr(void);
    int  cpu_sse_handle_exceptions(void);

    uint32_t cpu_get_simd_linaddr(uint32_t i, struct decoded_instruction *j);

    uint32_t cpu_get_linaddr2(uint32_t i, struct decoded_instruction *j);

    int write_back_handler(void);
    int get_read_ptr(uint32_t flags, struct decoded_instruction *i, int dwords, int unaligned_exception);
    int get_write_ptr(uint32_t flags, struct decoded_instruction *i, int dwords, int unaligned_exception);
    int get_sse_read_ptr(uint32_t flags, struct decoded_instruction *i, int dwords, int unaligned_exception);
    int get_sse_write_ptr(uint32_t flags, struct decoded_instruction *i, int dwords, int unaligned_exception);
    int get_mmx_read_ptr(uint32_t flags, struct decoded_instruction *i, int dwords);
    int get_mmx_write_ptr(uint32_t flags, struct decoded_instruction *i, int dwords);
    int get_reg_read_ptr(uint32_t flags, struct decoded_instruction *i);
    int get_reg_write_ptr(uint32_t flags, struct decoded_instruction *i);

    void *get_mmx_reg_dest(int x);
    void *get_mmx_reg_src(int x);
    void *get_sse_reg_dest(int x);
    void *get_reg_dest(int x);

    void     punpckh(void *dst, void *src, int size, int copysize);
    uint16_t pack_i32_to_i16(uint32_t x);
    uint16_t pack_i16_to_u8(int16_t x);
    uint8_t  pack_i16_to_i8(uint16_t x);

    void     packssdw(void *dest, void *src, int dwordcount);
    void     punpckl(void *dst, void *src, int size, int copysize);
    void     psubsb(uint8_t *dest, uint8_t *src, int bytecount);
    void     psubsw(uint16_t *dest, uint16_t *src, int wordcount);
    void     pminub(uint8_t *dest, uint8_t *src, int bytecount);
    void     pmaxub(uint8_t *dest, uint8_t *src, int bytecount);
    void     pminsw(int16_t *dest, int16_t *src, int wordcount);
    void     pmaxsw(int16_t *dest, int16_t *src, int wordcount);
    void     paddsb(uint8_t *dest, uint8_t *src, int bytecount);
    void     paddsw(uint16_t *dest, uint16_t *src, int wordcount);
    void     pshuf(void *dest, void *src, int imm, int shift);
    void     pshufb(void *dest, void *src, int bytes);
    void     cpu_psraw(uint16_t *a, int shift, int mask, int wordcount);
    void     cpu_psrlw(uint16_t *a, int shift, int mask, int wordcount);
    void     cpu_psllw(uint16_t *a, int shift, int mask, int wordcount);
    void     cpu_psrad(uint32_t *a, int shift, int mask, int wordcount);
    void     cpu_psrld(uint32_t *a, int shift, int mask, int wordcount);
    void     cpu_pslld(uint32_t *a, int shift, int mask, int wordcount);
    void     cpu_psrlq(uint64_t *a, int shift, int mask, int wordcount);
    void     cpu_psllq(uint64_t *a, int shift, int mask, int wordcount);
    void     cpu_pslldq(uint64_t *a, int shift, int mask);
    void     cpu_psrldq(uint64_t *a, int shift, int mask);
    void     pcmpeqb(uint8_t *dest, uint8_t *src, int count);
    void     pcmpeqw(uint16_t *dest, uint16_t *src, int count);
    void     pcmpeqd(uint32_t *dest, uint32_t *src, int count);
    void     pcmpgtb(int8_t *dest, int8_t *src, int count);
    void     pcmpgtw(int16_t *dest, int16_t *src, int count);
    void     pcmpgtd(int32_t *dest, int32_t *src, int count);
    void     packuswb(void *dest, void *src, int wordcount);
    void     packsswb(void *dest, void *src, int wordcount);
    void     pmullw(uint16_t *dest, uint16_t *src, int wordcount, int shift);
    void     pmuluw(void *dest, void *src, int wordcount, int shift);
    void     pmuludq(void *dest, void *src, int dwordcount);
    int      pmovmskb(uint8_t *src, int bytecount);
    void     psubusb(uint8_t *dest, uint8_t *src, int bytecount);
    void     psubusw(uint16_t *dest, uint16_t *src, int wordcount);
    void     paddusb(uint8_t *dest, uint8_t *src, int bytecount);
    void     paddusw(uint16_t *dest, uint16_t *src, int wordcount);
    void     paddb(uint8_t *dest, uint8_t *src, int bytecount);
    void     paddw(uint16_t *dest, uint16_t *src, int wordcount);
    void     paddd(uint32_t *dest, uint32_t *src, int dwordcount);
    void     psubb(uint8_t *dest, uint8_t *src, int bytecount);
    void     psubw(uint16_t *dest, uint16_t *src, int wordcount);
    void     psubd(uint32_t *dest, uint32_t *src, int dwordcount);
    void     psubq(uint64_t *dest, uint64_t *src, int qwordcount);
    uint32_t cmpps(float32 dest, float32 src, int cmp);
    uint64_t cmppd(float64 dest, float64 src, int cmp);
    void     shufps(void *dest, void *src, int imm);
    void     shufpd(void *dest, void *src, int imm);
    void     pavgb(void *dest, void *src, int bytecount);
    void     pavgw(void *dest, void *src, int wordcount);
    void     pmaddwd(void *dest, void *src, int dwordcount);
    void     psadbw(void *dest, void *src, int qwordcount);
    void     pabsb(void *dest, void *src, int bytecount);
    void     pabsw(void *dest, void *src, int wordcount);
    void     pabsd(void *dest, void *src, int dwordcount);
    int      execute_0F10_17(struct decoded_instruction *i);
    int      execute_0F28_2F(struct decoded_instruction *i);
    float32  rsqrt(float32 a);
    float32  rcp(float32 a);
    int      execute_0F50_57(struct decoded_instruction *i);
    int      execute_0F68_6F(struct decoded_instruction *i);
    int      execute_0FE8_EF(struct decoded_instruction *i);
    void     pshift(void *dest, int opcode, int wordcount, int imm);
    int      execute_0F70_76(struct decoded_instruction *i);
    int      execute_0F60_67(struct decoded_instruction *i);
    uint32_t get_shift(void *x, int bytes);
    int      execute_0FD0_D7(struct decoded_instruction *i);
    int      execute_0FD8_DF(struct decoded_instruction *i);
    int      execute_0F7E_7F(struct decoded_instruction *i);
    int      execute_0FF8_FE(struct decoded_instruction *i);
    int      execute_0FC2_C6(struct decoded_instruction *i);
    int      execute_0F58_5F(struct decoded_instruction *i);
    int      execute_0FE0_E7(struct decoded_instruction *i);
    int      execute_0FF1_F7(struct decoded_instruction *i);
    int      cpu_emms(void);
    int      execute_0F7C_7D(struct decoded_instruction *i);
    int      execute_0F38(struct decoded_instruction *i);
    int      execute_660F38(struct decoded_instruction *i);

  public:
    int  cpu_smc_page_has_code(uint32_t phys);
    int  cpu_smc_has_code(uint32_t phys);
    void cpu_smc_set_code(uint32_t phys);
    void cpu_smc_invalidate(uint32_t lin, uint32_t phys);
    void cpu_smc_invalidate_page(uint32_t phys);

  public:
    uint16_t shift16RightJamming(uint16_t a, int count);
    uint32_t shift32RightJamming(uint32_t a, int count);
    uint64_t shift64RightJamming(uint64_t a, int count);
    void     shift64ExtraRightJamming(uint64_t a0, uint64_t a1, int count, uint64_t *z0Ptr, uint64_t *z1Ptr);
    void     add128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t *z0Ptr, uint64_t *z1Ptr);
    void     sub128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t *z0Ptr, uint64_t *z1Ptr);
    void     mul64To128(uint64_t a, uint64_t b, uint64_t *z0Ptr, uint64_t *z1Ptr);
    uint64_t estimateDiv128To64(uint64_t a0, uint64_t a1, uint64_t b);
    uint32_t estimateSqrt32(int16_t aExp, uint32_t a);
    int      countLeadingZeros16(uint16_t a);
    int      countLeadingZeros32(uint32_t a);
    int      countLeadingZeros64(uint64_t a);
    void     shift128Right(uint64_t a0, uint64_t a1, int count, uint64_t *z0Ptr, uint64_t *z1Ptr);
    void     shift128RightJamming(uint64_t a0, uint64_t a1, int count, uint64_t *z0Ptr, uint64_t *z1Ptr);
    void     shortShift128Left(uint64_t a0, uint64_t a1, int count, uint64_t *z0Ptr, uint64_t *z1Ptr);
    void     add192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t *z0Ptr,
                    uint64_t *z1Ptr, uint64_t *z2Ptr);
    void     sub192(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t *z0Ptr,
                    uint64_t *z1Ptr, uint64_t *z2Ptr);
    int      eq128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1);
    int      le128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1);
    int      lt128(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1);
    void     mul128By64To192(uint64_t a0, uint64_t a1, uint64_t b, uint64_t *z0Ptr, uint64_t *z1Ptr, uint64_t *z2Ptr);
    void     mul128To256(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1, uint64_t *z0Ptr, uint64_t *z1Ptr,
                         uint64_t *z2Ptr, uint64_t *z3Ptr);
    void shift128ExtraRightJamming(uint64_t a0, uint64_t a1, uint64_t a2, int count, uint64_t *z0Ptr, uint64_t *z1Ptr,
                                   uint64_t *z2Ptr);
    uint16_t      extractFloat16Frac(float16 a);
    int16_t       extractFloat16Exp(float16 a);
    int           extractFloat16Sign(float16 a);
    float16       packFloat16(int zSign, int zExp, uint16_t zSig);
    int           float16_is_nan(float16 a);
    int           float16_is_signaling_nan(float16 a);
    int           float16_is_denormal(float16 a);
    float16       float16_denormal_to_zero(float16 a);
    commonNaNT    float16ToCommonNaN(float16 a, float_status_t *status);
    float16       commonNaNToFloat16(commonNaNT a);
    uint32_t      extractFloat32Frac(float32 a);
    int16_t       extractFloat32Exp(float32 a);
    int           extractFloat32Sign(float32 a);
    float32       packFloat32(int zSign, int16_t zExp, uint32_t zSig);
    int           float32_is_nan(float32 a);
    int           float32_is_signaling_nan(float32 a);
    int           float32_is_denormal(float32 a);
    float32       float32_denormal_to_zero(float32 a);
    commonNaNT    float32ToCommonNaN(float32 a, float_status_t *status);
    float32       commonNaNToFloat32(commonNaNT a);
    float32       propagateFloat32NaN(float32 a, float_status_t *status);
    uint64_t      extractFloat64Frac(float64 a);
    int16_t       extractFloat64Exp(float64 a);
    int           extractFloat64Sign(float64 a);
    float64       packFloat64(int zSign, int16_t zExp, uint64_t zSig);
    int           float64_is_nan(float64 a);
    int           float64_is_signaling_nan(float64 a);
    int           float64_is_denormal(float64 a);
    float64       float64_denormal_to_zero(float64 a);
    commonNaNT    float64ToCommonNaN(float64 a, float_status_t *status);
    float64       commonNaNToFloat64(commonNaNT a);
    float64       propagateFloat64NaN(float64 a, float_status_t *status);
    uint64_t      extractFloatx80Frac(floatx80 a);
    int32_t       extractFloatx80Exp(floatx80 a);
    int           extractFloatx80Sign(floatx80 a);
    floatx80      packFloatx80(int zSign, int32_t zExp, uint64_t zSig);
    int           floatx80_is_nan(floatx80 a);
    int           floatx80_is_signaling_nan(floatx80 a);
    int           floatx80_is_unsupported(floatx80 a);
    commonNaNT    floatx80ToCommonNaN(floatx80 a, float_status_t *status);
    floatx80      commonNaNToFloatx80(commonNaNT a);
    floatx80      propagateFloatx80NaN(floatx80 a, float_status_t *status);
    uint64_t      extractFloat128Frac1(float128 a);
    uint64_t      extractFloat128Frac0(float128 a);
    int32_t       extractFloat128Exp(float128 a);
    int           extractFloat128Sign(float128 a);
    float128      packFloat128(int zSign, int32_t zExp, uint64_t zSig0, uint64_t zSig1);
    float128      packFloat128_simple(uint64_t zHi, uint64_t zLo);
    int           float128_is_nan(float128 a);
    int           float128_is_signaling_nan(float128 a);
    commonNaNT    float128ToCommonNaN(float128 a, float_status_t *status);
    float128      commonNaNToFloat128(commonNaNT a);
    float32       int32_to_float32(int32_t a, float_status_t *status);
    float64       int32_to_float64(int32_t a);
    float32       int64_to_float32(int64_t a, float_status_t *status);
    float64       int64_to_float64(int64_t a, float_status_t *status);
    float32       uint32_to_float32(uint32_t a, float_status_t *status);
    float64       uint32_to_float64(uint32_t a);
    float32       uint64_to_float32(uint64_t a, float_status_t *status);
    float64       uint64_to_float64(uint64_t a, float_status_t *status);
    int32_t       float32_to_int32(float32 a, float_status_t *status);
    int32_t       float32_to_int32_round_to_zero(float32 a, float_status_t *status);
    uint32_t      float32_to_uint32_round_to_zero(float32 a, float_status_t *status);
    int64_t       float32_to_int64(float32 a, float_status_t *status);
    int64_t       float32_to_int64_round_to_zero(float32 a, float_status_t *status);
    uint64_t      float32_to_uint64_round_to_zero(float32 a, float_status_t *status);
    uint64_t      float32_to_uint64(float32 a, float_status_t *status);
    uint32_t      float32_to_uint32(float32 a, float_status_t *status);
    float64       float32_to_float64(float32 a, float_status_t *status);
    float32       float32_round_to_int_with_scale(float32 a, uint8_t scale, float_status_t *status);
    float32       float32_frc(float32 a, float_status_t *status);
    float32       float32_getexp(float32 a, float_status_t *status);
    float32       float32_getmant(float32 a, float_status_t *status, int sign_ctrl, int interv);
    float32       float32_scalef(float32 a, float32 b, float_status_t *status);
    float32       addFloat32Sigs(float32 a, float32 b, int zSign, float_status_t *status);
    float32       subFloat32Sigs(float32 a, float32 b, int zSign, float_status_t *status);
    float32       float32_add(float32 a, float32 b, float_status_t *status);
    float32       float32_sub(float32 a, float32 b, float_status_t *status);
    float32       float32_mul(float32 a, float32 b, float_status_t *status);
    float32       float32_div(float32 a, float32 b, float_status_t *status);
    float32       float32_sqrt(float32 a, float_status_t *status);
    float_class_t float32_class(float32 a);
    int           float32_compare_internal(float32 a, float32 b, int quiet, float_status_t *status);
    float32       float32_min(float32 a, float32 b, float_status_t *status);
    float32       float32_max(float32 a, float32 b, float_status_t *status);
    float32       float32_minmax(float32 a, float32 b, int is_max, int is_abs, float_status_t *status);
    int32_t       float64_to_int32(float64 a, float_status_t *status);
    int32_t       float64_to_int32_round_to_zero(float64 a, float_status_t *status);
    uint32_t      float64_to_uint32_round_to_zero(float64 a, float_status_t *status);
    int64_t       float64_to_int64(float64 a, float_status_t *status);
    int64_t       float64_to_int64_round_to_zero(float64 a, float_status_t *status);
    uint64_t      float64_to_uint64_round_to_zero(float64 a, float_status_t *status);
    uint32_t      float64_to_uint32(float64 a, float_status_t *status);
    uint64_t      float64_to_uint64(float64 a, float_status_t *status);
    float32       float64_to_float32(float64 a, float_status_t *status);
    float64       float64_round_to_int_with_scale(float64 a, uint8_t scale, float_status_t *status);
    float64       float64_frc(float64 a, float_status_t *status);
    float64       float64_getexp(float64 a, float_status_t *status);
    float64       float64_getmant(float64 a, float_status_t *status, int sign_ctrl, int interv);
    float64       float64_scalef(float64 a, float64 b, float_status_t *status);
    float64       addFloat64Sigs(float64 a, float64 b, int zSign, float_status_t *status);
    float64       subFloat64Sigs(float64 a, float64 b, int zSign, float_status_t *status);
    float64       float64_add(float64 a, float64 b, float_status_t *status);
    float64       float64_sub(float64 a, float64 b, float_status_t *status);
    float64       float64_mul(float64 a, float64 b, float_status_t *status);
    float64       float64_div(float64 a, float64 b, float_status_t *status);
    float64       float64_sqrt(float64 a, float_status_t *status);
    float_class_t float64_class(float64 a);
    int           float64_compare_internal(float64 a, float64 b, int quiet, float_status_t *status);
    float64       float64_min(float64 a, float64 b, float_status_t *status);
    float64       float64_max(float64 a, float64 b, float_status_t *status);
    float64       float64_minmax(float64 a, float64 b, int is_max, int is_abs, float_status_t *status);
    floatx80      int32_to_floatx80(int32_t a);
    floatx80      int64_to_floatx80(int64_t a);
    floatx80      float32_to_floatx80(float32 a, float_status_t *status);
    floatx80      float64_to_floatx80(float64 a, float_status_t *status);
    int32_t       floatx80_to_int32(floatx80 a, float_status_t *status);
    int32_t       floatx80_to_int32_round_to_zero(floatx80 a, float_status_t *status);
    int64_t       floatx80_to_int64(floatx80 a, float_status_t *status);
    int64_t       floatx80_to_int64_round_to_zero(floatx80 a, float_status_t *status);
    float32       floatx80_to_float32(floatx80 a, float_status_t *status);
    float64       floatx80_to_float64(floatx80 a, float_status_t *status);
    floatx80      floatx80_round_to_int(floatx80 a, float_status_t *status);
    floatx80      addFloatx80Sigs(floatx80 a, floatx80 b, int zSign, float_status_t *status);
    floatx80      subFloatx80Sigs(floatx80 a, floatx80 b, int zSign, float_status_t *status);
    floatx80      floatx80_add(floatx80 a, floatx80 b, float_status_t *status);
    floatx80      floatx80_sub(floatx80 a, floatx80 b, float_status_t *status);
    floatx80      floatx80_mul(floatx80 a, floatx80 b, float_status_t *status);
    floatx80      floatx80_div(floatx80 a, floatx80 b, float_status_t *status);
    floatx80      floatx80_sqrt(floatx80 a, float_status_t *status);
    float128      floatx80_to_float128(floatx80 a, float_status_t *status);
    floatx80      float128_to_floatx80(float128 a, float_status_t *status);
    floatx80      floatx80_mul_with_float128(floatx80 a, float128 b, float_status_t *status);
    float128      addFloat128Sigs(float128 a, float128 b, int zSign, float_status_t *status);
    float128      subFloat128Sigs(float128 a, float128 b, int zSign, float_status_t *status);
    float128      float128_add(float128 a, float128 b, float_status_t *status);
    float128      float128_sub(float128 a, float128 b, float_status_t *status);
    float128      float128_mul(float128 a, float128 b, float_status_t *status);
    float128      float128_div(float128 a, float128 b, float_status_t *status);
    float128      int64_to_float128(int64_t a);
    int16_t       floatx80_to_int16(floatx80 a, float_status_t *status);
    int16_t       floatx80_to_int16_round_to_zero(floatx80 a, float_status_t *status);
    floatx80      floatx80_extract(floatx80 *input, float_status_t *status);
    floatx80      floatx80_scale(floatx80 a, floatx80 b, float_status_t *status);
    float_class_t floatx80_class(floatx80 a);
    int           floatx80_compare_internal(floatx80 a, floatx80 b, int quiet, float_status_t *status);
    float32       propagateFloat32NaN_two_args(float32 a, float32 b, float_status_t *status);
    float64       propagateFloat64NaN_two_args(float64 a, float64 b, float_status_t *status);
    floatx80      propagateFloatx80NaN_two_args(floatx80 a, floatx80 b, float_status_t *status);
    float128      propagateFloat128NaN(float128 a, float128 b, float_status_t *status);
    int32_t       roundAndPackInt32(int zSign, uint64_t exactAbsZ, float_status_t *status);
    int64_t       roundAndPackInt64(int zSign, uint64_t absZ0, uint64_t absZ1, float_status_t *status);
    uint64_t      roundAndPackUint64(int zSign, uint64_t absZ0, uint64_t absZ1, float_status_t *status);
    void          normalizeFloat16Subnormal(uint16_t aSig, int16_t *zExpPtr, uint16_t *zSigPtr);
    float16       roundAndPackFloat16(int zSign, int16_t zExp, uint16_t zSig, float_status_t *status);
    void          normalizeFloat32Subnormal(uint32_t aSig, int16_t *zExpPtr, uint32_t *zSigPtr);
    float32       roundAndPackFloat32(int zSign, int16_t zExp, uint32_t zSig, float_status_t *status);
    float32       normalizeRoundAndPackFloat32(int zSign, int16_t zExp, uint32_t zSig, float_status_t *status);
    void          normalizeFloat64Subnormal(uint64_t aSig, int16_t *zExpPtr, uint64_t *zSigPtr);
    float64       roundAndPackFloat64(int zSign, int16_t zExp, uint64_t zSig, float_status_t *status);
    float64       normalizeRoundAndPackFloat64(int zSign, int16_t zExp, uint64_t zSig, float_status_t *status);
    void          normalizeFloatx80Subnormal(uint64_t aSig, int32_t *zExpPtr, uint64_t *zSigPtr);
    floatx80      SoftFloatRoundAndPackFloatx80(int roundingPrecision, int zSign, int32_t zExp, uint64_t zSig0,
                                                uint64_t zSig1, float_status_t *status);
    floatx80      roundAndPackFloatx80(int roundingPrecision, int zSign, int32_t zExp, uint64_t zSig0, uint64_t zSig1,
                                       float_status_t *status);
    floatx80      normalizeRoundAndPackFloatx80(int roundingPrecision, int zSign, int32_t zExp, uint64_t zSig0,
                                                uint64_t zSig1, float_status_t *status);
    void          normalizeFloat128Subnormal(uint64_t aSig0, uint64_t aSig1, int32_t *zExpPtr, uint64_t *zSig0Ptr,
                                             uint64_t *zSig1Ptr);
    float128      roundAndPackFloat128(int zSign, int32_t zExp, uint64_t zSig0, uint64_t zSig1, uint64_t zSig2,
                                       float_status_t *status);
    float128      normalizeRoundAndPackFloat128(int zSign, int32_t zExp, uint64_t zSig0, uint64_t zSig1,
                                                float_status_t *status);
    float32       propagateFloat32MulAddNaN(float32 a, float32 b, float32 c, float_status_t *status);
    float64       propagateFloat64MulAddNaN(float64 a, float64 b, float64 c, float_status_t *status);
    float32       float32_muladd(float32 a, float32 b, float32 c, int flags, float_status_t *status);
    float64       float64_muladd(float64 a, float64 b, float64 c, int flags, float_status_t *status);
    float128      EvalPoly(float128 x, float128 *arr, int n, float_status_t *status);
    float128      EvenPoly(float128 x, float128 *arr, int n, float_status_t *status);
    float128      OddPoly(float128 x, float128 *arr, int n, float_status_t *status);
    float128      poly_ln(float128 x1, float_status_t *status);
    float128      poly_l2(float128 x, float_status_t *status);
    float128      poly_l2p1(float128 x, float_status_t *status);
    floatx80      fyl2x(floatx80 a, floatx80 b, float_status_t *status);
    floatx80      fyl2xp1(floatx80 a, floatx80 b, float_status_t *status);
    uint64_t      argument_reduction_kernel(uint64_t aSig0, int Exp, uint64_t *zSig0, uint64_t *zSig1);
    int           reduce_trig_arg(int expDiff, int *zSign_input, uint64_t *aSig0_input, uint64_t *aSig1_input);
    float128      poly_sin(float128 x, float_status_t *status);
    float128      poly_cos(float128 x, float_status_t *status);
    void          sincos_invalid(floatx80 *sin_a, floatx80 *cos_a, floatx80 a);
    void          sincos_tiny_argument(floatx80 *sin_a, floatx80 *cos_a, floatx80 a);
    floatx80      sincos_approximation(int neg, float128 r, uint64_t quotient, float_status_t *status);
    int           fsincos(floatx80 a, floatx80 *sin_a, floatx80 *cos_a, float_status_t *status);
    int           fsin(floatx80 *a, float_status_t *status);
    int           fcos(floatx80 *a, float_status_t *status);
    int           ftan(floatx80 *a_input, float_status_t *status);
    uint64_t      remainder_kernel(uint64_t aSig0, uint64_t bSig, int expDiff, uint64_t *zSig0, uint64_t *zSig1);
    int           do_fprem(floatx80 a, floatx80 b, floatx80 *r_input, uint64_t *q_input, int rounding_mode,
                           float_status_t *status);
    int           floatx80_ieee754_remainder(floatx80 a, floatx80 b, floatx80 *r, uint64_t *q, float_status_t *status);
    int           floatx80_remainder(floatx80 a, floatx80 b, floatx80 *r, uint64_t *q, float_status_t *status);
    float128      poly_atan(float128 x1, float_status_t *status);
    floatx80      fpatan(floatx80 a, floatx80 b, float_status_t *status);
    float128      poly_exp(float128 x, float_status_t *status);
    floatx80      f2xm1(floatx80 a, float_status_t *status);

  public:
    int cpu_push16(uint32_t data);
    int cpu_push32(uint32_t data);
    int cpu_pop16(uint16_t *dest);
    int cpu_pop16_dest32(uint32_t *dest);
    int cpu_pop32(uint32_t *dest);
    int cpu_pusha(void);
    int cpu_pushad(void);
    int cpu_popa(void);
    int cpu_popad(void);

  public:
    int movsb16(int flags);
    int movsb32(int flags);
    int movsw16(int flags);
    int movsw32(int flags);
    int movsd16(int flags);
    int movsd32(int flags);
    int stosb16(int flags);
    int stosb32(int flags);
    int stosw16(int flags);
    int stosw32(int flags);
    int stosd16(int flags);
    int stosd32(int flags);
    int scasb16(int flags);
    int scasb32(int flags);
    int scasw16(int flags);
    int scasw32(int flags);
    int scasd16(int flags);
    int scasd32(int flags);
    int insb16(int flags);
    int insb32(int flags);
    int insw16(int flags);
    int insw32(int flags);
    int insd16(int flags);
    int insd32(int flags);
    int outsb16(int flags);
    int outsb32(int flags);
    int outsw16(int flags);
    int outsw32(int flags);
    int outsd16(int flags);
    int outsd32(int flags);
    int cmpsb16(int flags);
    int cmpsb32(int flags);
    int cmpsw16(int flags);
    int cmpsw32(int flags);
    int cmpsd16(int flags);
    int cmpsd32(int flags);
    int lodsb16(int flags);
    int lodsb32(int flags);
    int lodsw16(int flags);
    int lodsw32(int flags);
    int lodsd16(int flags);
    int lodsd32(int flags);

  public:
    uint32_t                    hash_eip(uint32_t phys);
    void                        cpu_trace_flush(void);
    struct trace_info          *cpu_trace_get_entry(uint32_t phys);
    struct decoded_instruction *cpu_get_trace(void);

  private:    // array
    const floatx80  Zero          = {0, 0};
    const floatx80  IndefiniteNaN = {0xC000000000000000, 0xFFFF};
    const floatx80  Constant_1    = {0x8000000000000000, 0x3fff};
    const floatx80  Constant_L2T  = {0xd49a784bcd1b8afe, 0x4000};
    const floatx80  Constant_L2E  = {0xb8aa3b295c17f0bc, 0x3fff};
    const floatx80  Constant_PI   = {0xc90fdaa22168c235, 0x4000};
    const floatx80  Constant_LG2  = {0x9a209a84fbcff799, 0x3ffd};
    const floatx80  Constant_LN2  = {0xb17217f7d1cf79ac, 0x3ffe};
    const floatx80 *Constants[8]  = {&Constant_1,   &Constant_L2T, &Constant_L2E, &Constant_PI,
                                     &Constant_LG2, &Constant_LN2, &Zero,         &IndefiniteNaN};

  private:
    const uint8_t optable[0x100] = {
        0x83, 0x83, 0x03, 0x03, 0x10, 0x20, 0x00, 0x00, 0x83, 0x83, 0x03, 0x03, 0x10, 0x20, 0x00, 0x01, 0x83, 0x83,
        0x03, 0x03, 0x10, 0x20, 0x00, 0x00, 0x83, 0x83, 0x03, 0x03, 0x10, 0x20, 0x00, 0x00, 0x83, 0x83, 0x03, 0x03,
        0x10, 0x20, 0x01, 0x00, 0x83, 0x83, 0x03, 0x03, 0x10, 0x20, 0x01, 0x00, 0x83, 0x83, 0x03, 0x03, 0x10, 0x20,
        0x01, 0x00, 0x03, 0x03, 0x03, 0x03, 0x10, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x20, 0x23, 0x10, 0x13,
        0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x93, 0xA3, 0x13, 0x93, 0x03, 0x03, 0x83, 0x83, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04,
        0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x13, 0x13, 0x40, 0x00, 0x03, 0x03,
        0x13, 0x23, 0x30, 0x00, 0x20, 0x00, 0x00, 0x10, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x10, 0x10, 0x00, 0x00,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20,
        0x60, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x83, 0x83};
    const uint8_t optable0F[0x100] = {
        0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x00, 0x05, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x05, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x10, 0x03, 0x10,
        0x05, 0x05, 0x05, 0x05, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x05, 0x05,
        0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x13, 0x13, 0x13, 0x13, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00,
        0x00, 0x03, 0x13, 0x03, 0x05, 0x05, 0x00, 0x00, 0x05, 0x83, 0x13, 0x03, 0x03, 0x03, 0x83, 0x83, 0x03, 0x83,
        0x03, 0x03, 0x03, 0x03, 0x05, 0x05, 0x93, 0x83, 0x03, 0x03, 0x03, 0x03, 0x83, 0x83, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x05, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
        0x03, 0x03, 0x03, 0x05};


    int addr16_lut[24]  = {3, 3, 5, 5, 6, 7, 8, 3, 6, 7, 6, 7, 8, 8, 8, 8, 0, 0, 1, 1, 0, 0, 0, 0};
    int addr16_lut2[24] = {3, 3, 5, 5, 6, 7, 5, 3, 6, 7, 6, 7, 8, 8, 8, 8, 0, 0, 1, 1, 0, 0, 1, 0};
    int addr32_lut[8]   = {0, 0, 0, 0, 0, 1, 0, 0};
    int addr32_lut2[8]  = {0, 0, 0, 0, 1, 1, 0, 0};

    const int decode_sse10_17_tbl[8 * 4] = {
        MOVUPS_XGoXEo,   MOVUPS_XGoXEo,   MOVSD_XGqXEq,    MOVSS_XGdXEd,    MOVUPS_XEoXGo,   MOVUPS_XEoXGo,
        MOVSD_XEqXGq,    MOVSS_XEdXGd,    MOVHLPS_XGqXEq,  MOVLPS_XGqXEq,   MOVHLPS_XGqXEq,  MOVHLPS_XGqXEq,
        MOVSD_XEqXGq,    MOVSD_XEqXGq,    MOVSD_XEqXGq,    MOVSD_XEqXGq,    UNPCKLPS_XGoXEq, UNPCKLPD_XGoXEo,
        UNPCKLPS_XGoXEq, UNPCKLPS_XGoXEq, UNPCKHPS_XGoXEq, UNPCKHPD_XGoXEo, UNPCKHPS_XGoXEq, UNPCKHPS_XGoXEq,
        MOVHPS_XGqXEq,   MOVHPS_XGqXEq,   MOVHPS_XGqXEq,   MOVSHDUP_XGoXEo, MOVHPS_XEqXGq,   MOVHPS_XEqXGq,
        MOVHPS_XEqXGq,   MOVHPS_XEqXGq};
    const int decode_sse28_2F_tbl[8 * 4] = {
        MOVAPS_XGoXEo,  MOVAPS_XGoXEo,  MOVAPS_XGoXEo,   MOVAPS_XGoXEo,   MOVAPS_XEoXGo,   MOVAPS_XEoXGo,
        MOVAPS_XEoXGo,  MOVAPS_XEoXGo,  CVTPI2PS_XGqMEq, CVTPI2PD_XGoMEq, CVTSI2SD_XGqMEd, CVTSI2SS_XGdEd,
        MOVAPS_XEoXGo,  MOVAPS_XEoXGo,  MOVAPS_XEoXGo,   MOVAPS_XEoXGo,   CVTPS2PI_MGqXEq, CVTPD2PI_MGqXEo,
        CVTSD2SI_GdXEq, CVTSS2SI_GdXEd, CVTPS2PI_MGqXEq, CVTPD2PI_MGqXEo, CVTSD2SI_GdXEq,  CVTSS2SI_GdXEd,
        UCOMISS_XGdXEd, UCOMISD_XGqXEq, UCOMISS_XGdXEd,  UCOMISS_XGdXEd,  UCOMISS_XGdXEd,  UCOMISD_XGqXEq,
        UCOMISS_XGdXEd, UCOMISS_XGdXEd,
    };
    const int decode_sse50_57_tbl[8 * 4] = {
        MOVMSKPS_GdXEo, MOVMSKPD_GdXEo, MOVMSKPS_GdXEo, MOVMSKPS_GdXEo, SQRTPS_XGoXEo,  SQRTPD_XGoXEo, SQRTSD_XGqXEq,
        SQRTSS_XGdXEd,  RSQRTPS_XGoXEo, RSQRTSS_XGdXEd, RSQRTPS_XGoXEo, RSQRTPS_XGoXEo, RCPPS_XGoXEo,  RCPSS_XGdXEd,
        RCPPS_XGoXEo,   RCPPS_XGoXEo,   ANDPS_XGoXEo,   ANDPS_XGoXEo,   ANDPS_XGoXEo,   ANDPS_XGoXEo,  ANDNPS_XGoXEo,
        ANDNPS_XGoXEo,  ANDNPS_XGoXEo,  ANDNPS_XGoXEo,  ORPS_XGoXEo,    ORPS_XGoXEo,    ORPS_XGoXEo,   ORPS_XGoXEo,
        XORPS_XGoXEo,   XORPS_XGoXEo,   XORPS_XGoXEo,   XORPS_XGoXEo};
    const int decode_sse58_5F_tbl[8 * 4] = {
        ADDPS_XGoXEo,    ADDPD_XGoXEo,    ADDSD_XGqXEq,    ADDSS_XGdXEd,     MULPS_XGoXEo,    MULPD_XGoXEo,
        MULSD_XGqXEq,    MULSS_XGdXEd,    CVTPS2PD_XGoXEo, CVTPD2PS_XGoXEo,  CVTSD2SS_XGoXEq, CVTSS2SD_XGoXEd,
        CVTDQ2PS_XGoXEo, CVTPS2DQ_XGoXEo, CVTDQ2PS_XGoXEo, CVTTPS2DQ_XGoXEo, SUBPS_XGoXEo,    SUBPD_XGoXEo,
        SUBSD_XGqXEq,    SUBSS_XGdXEd,    MINPS_XGoXEo,    MINPD_XGoXEo,     MINSD_XGqXEq,    MINSS_XGdXEd,
        DIVPS_XGoXEo,    DIVPD_XGoXEo,    DIVSD_XGqXEq,    DIVSS_XGdXEd,     MAXPS_XGoXEo,    MAXPD_XGoXEo,
        MAXSD_XGqXEq,    MAXSS_XGdXEd};
    const int decode_sse60_67_tbl[8 * 2] = {PUNPCKLBW_MGqMEq, PUNPCKLBW_XGoXEo, PUNPCKLWD_MGqMEq, PUNPCKLWD_XGoXEo,
                                            PUNPCKLDQ_MGqMEq, PUNPCKLDQ_XGoXEo, PACKSSWB_MGqMEq,  PACKSSWB_XGoXEo,
                                            PCMPGTB_MGqMEq,   PCMPGTB_XGoXEo,   PCMPGTW_MGqMEq,   PCMPGTW_XGoXEo,
                                            PCMPGTD_MGqMEq,   PCMPGTD_XGoXEo,   PACKUSWB_MGqMEq,  PACKUSWB_XGoXEo};
    const int decode_sse68_6F_tbl[8 * 4] = {
        PUNPCKHBW_MGqMEq, PUNPCKHBW_XGoXEo, PUNPCKHBW_MGqMEq, PUNPCKHBW_MGqMEq,  PUNPCKHWD_MGqMEq, PUNPCKHWD_XGoXEo,
        PUNPCKHWD_MGqMEq, PUNPCKHWD_MGqMEq, PUNPCKHDQ_MGqMEq, PUNPCKHDQ_XGoXEo,  PUNPCKHDQ_MGqMEq, PUNPCKHDQ_MGqMEq,
        PACKSSDW_MGqMEq,  PACKSSDW_XGoXEo,  PACKSSDW_MGqMEq,  PACKSSDW_MGqMEq,   OP_68_6F_INVALID, PUNPCKLQDQ_XGoXEo,
        OP_68_6F_INVALID, OP_68_6F_INVALID, OP_68_6F_INVALID, PUNPCKHQDQ_XGoXEo, OP_68_6F_INVALID, OP_68_6F_INVALID,
        MOVD_MGdEd,       MOVD_XGdEd,       MOVD_MGdEd,       MOVD_MGdEd,        MOVQ_MGqMEq,      MOVDQA_XGoXEo,
        MOVQ_MGqMEq,      MOVDQU_XGoXEo};
    const int decode_sse70_76_tbl[7 * 4] = {
        PSHUFW_MGqMEqIb, PSHUFD_XGoXEoIb, PSHUFLW_XGoXEoIb, PSHUFHW_XGoXEoIb, PSHIFT_MGqIb,   PSHIFT_XEoIb,
        PSHIFT_MGqIb,    PSHIFT_MGqIb,    PSHIFT_MGqIb,     PSHIFT_XEoIb,     PSHIFT_MGqIb,   PSHIFT_MGqIb,
        PSHIFT_MGqIb,    PSHIFT_XEoIb,    PSHIFT_MGqIb,     PSHIFT_MGqIb,     PCMPEQB_MGqMEq, PCMPEQB_XGoXEo,
        PCMPEQB_MGqMEq,  PCMPEQB_MGqMEq,  PCMPEQW_MGqMEq,   PCMPEQW_XGoXEo,   PCMPEQW_MGqMEq, PCMPEQW_MGqMEq,
        PCMPEQD_MGqMEq,  PCMPEQD_XGoXEo,  PCMPEQD_MGqMEq,   PCMPEQD_MGqMEq};
    const int rm_table_pshift_mmx[24] = {0,
                                         0,
                                         PSHIFT_PSRLW,
                                         0,
                                         PSHIFT_PSRAW,
                                         0,
                                         PSHIFT_PSLLW,
                                         0,
                                         0,
                                         0,
                                         PSHIFT_PSRLD,
                                         0,
                                         PSHIFT_PSRAD,
                                         0,
                                         PSHIFT_PSLLD,
                                         0,
                                         0,
                                         0,
                                         PSHIFT_PSRLQ,
                                         0,
                                         0,
                                         0,
                                         PSHIFT_PSLLQ,
                                         0};
    const int rm_table_pshift_sse[24] = {0,
                                         0,
                                         PSHIFT_PSRLW,
                                         0,
                                         PSHIFT_PSRAW,
                                         0,
                                         PSHIFT_PSLLW,
                                         0,
                                         0,
                                         0,
                                         PSHIFT_PSRLD,
                                         0,
                                         PSHIFT_PSRAD,
                                         0,
                                         PSHIFT_PSLLD,
                                         0,
                                         0,
                                         0,
                                         PSHIFT_PSRLQ,
                                         PSHIFT_PSRLDQ,
                                         0,
                                         0,
                                         PSHIFT_PSLLQ,
                                         PSHIFT_PSLLDQ};

    const int decode_7C_7F[4]            = {HADDPD_XGoXEo, HADDPS_XGoXEo, HSUBPD_XGoXEo, HSUBPS_XGoXEo};
    const int decode_7E_7F[2 * 4]        = {MOVD_EdMGd,  MOVD_EdXGd,    MOVD_EdMGd,  MOVQ_XGqXEq,
                                            MOVQ_MEqMGq, MOVDQA_XEqXGq, MOVQ_MEqMGq, MOVDQU_XEqXGq};
    const int decode_sseC2_C6_tbl[5 * 4] = {CMPPS_XGoXEoIb,  CMPPD_XGoXEoIb,  CMPSD_XGqXEqIb,  CMPSS_XGdXEdIb,
                                            MOVNTI_EdGd,     MOVNTI_EdGd,     MOVNTI_EdGd,     MOVNTI_EdGd,
                                            PINSRW_MGqEdIb,  PINSRW_XGoEdIb,  PINSRW_MGqEdIb,  PINSRW_MGqEdIb,
                                            PEXTRW_GdMEqIb,  PEXTRW_GdXEoIb,  PEXTRW_GdMEqIb,  PEXTRW_GdMEqIb,
                                            SHUFPS_XGoXEoIb, SHUFPD_XGoXEoIb, SHUFPS_XGoXEoIb, SHUFPS_XGoXEoIb};
    const int decode_sseD0_D7_tbl[8 * 4] = {
        PSRLW_MGqMEq, PSRLW_XGoXEo,   PSRLW_MGqMEq,   PSRLW_MGqMEq,   PSRLD_MGqMEq,   PSRLD_XGoXEo,   PSRLD_MGqMEq,
        PSRLD_MGqMEq, PSRLQ_MGqMEq,   PSRLQ_XGoXEo,   PSRLQ_MGqMEq,   PSRLQ_MGqMEq,   PADDQ_MGqMEq,   PADDQ_XGoXEo,
        PADDQ_MGqMEq, PADDQ_MGqMEq,   PMULLW_MGqMEq,  PMULLW_XGoXEo,  PMULLW_MGqMEq,  PMULLW_MGqMEq,  MOVQ_XEqXGq,
        MOVQ_XEqXGq,  MOVDQ2Q_MGqXEo, MOVQ2DQ_XGoMEq, PMOVMSKB_GdMEq, PMOVMSKB_GdXEo, PMOVMSKB_GdMEq, PMOVMSKB_GdMEq};
    const int decode_sseD8_DF_tbl[8 * 2] = {PSUBUSB_MGqMEq, PSUBUSB_XGoXEo, PSUBUSW_MGqMEq, PSUBUSW_XGoXEo,
                                            PMINUB_MGqMEq,  PMINUB_XGoXEo,  PAND_MGqMEq,    PAND_XGoXEo,
                                            PADDUSB_MGqMEq, PADDUSB_XGoXEo, PADDUSW_MGqMEq, PADDUSW_XGoXEo,
                                            PMAXUB_MGqMEq,  PMAXUB_XGoXEo,  PANDN_MGqMEq,   PANDN_XGoXEo};
    const int decode_sseE0_E7_tbl[8 * 4] = {
        PAVGB_MGqMEq,     PAVGB_XGoXEo,     PAVGB_MGqMEq,    PAVGB_MGqMEq,    PSRAW_MGqMEq,   PSRAW_XGoXEo,
        PSRAW_MGqMEq,     PSRAW_MGqMEq,     PSRAD_MGqMEq,    PSRAD_XGoXEo,    PSRAD_MGqMEq,   PSRAD_MGqMEq,
        PAVGW_MGqMEq,     PAVGW_XGoXEo,     PAVGW_MGqMEq,    PAVGW_MGqMEq,    PMULHUW_MGqMEq, PMULHUW_XGoXEo,
        PMULHUW_MGqMEq,   PMULHUW_MGqMEq,   PMULHW_MGqMEq,   PMULHW_XGoXEo,   PMULHW_MGqMEq,  PMULHW_MGqMEq,
        CVTTPD2DQ_XGoXEo, CVTTPD2DQ_XGoXEo, CVTPD2DQ_XGoXEo, CVTDQ2PD_XGoXEq, MOVNTQ_MEqMGq,  MOVNTDQ_XEoXGo,
        MOVNTQ_MEqMGq,    MOVNTQ_MEqMGq};
    const int decode_sseE8_EF_tbl[8 * 2] = {PSUBSB_MGqMEq, PSUBSB_XGoXEo, PSUBSW_MGqMEq, PSUBSW_XGoXEo,
                                            PMINSW_MGqMEq, PMINSW_XGoXEo, POR_MGqMEq,    POR_XGoXEo,
                                            PADDSB_MGqMEq, PADDSB_XGoXEo, PADDSW_MGqMEq, PADDSW_XGoXEo,
                                            PMAXSW_MGqMEq, PMAXSW_XGoXEo, PXOR_MGqMEq,   PXOR_XGoXEo};
    const int decode_sseF1_F7_tbl[7 * 2] = {PSLLW_MGqMEq,    PSLLW_XGoXEo,    PSLLD_MGqMEq,    PSLLD_XGoXEo,
                                            PSLLQ_MGqMEq,    PSLLQ_XGoXEo,    PMULLUDQ_MGqMEq, PMULLUDQ_XGoXEo,
                                            PMADDWD_MGqMEq,  PMADDWD_XGoXEo,  PSADBW_MGqMEq,   PSADBW_XGoXEo,
                                            MASKMOVQ_MEqMGq, MASKMOVDQ_XEoXGo};
    const int decode_sseF8_FE_tbl[7 * 2] = {PSUBB_MGqMEq, PSUBB_XGoXEo, PSUBW_MGqMEq, PSUBW_XGoXEo, PSUBD_MGqMEq,
                                            PSUBD_XGoXEo, PSUBQ_MGqMEq, PSUBQ_XGoXEo, PADDB_MGqMEq, PADDB_XGoXEo,
                                            PADDW_MGqMEq, PADDW_XGoXEo, PADDD_MGqMEq, PADDD_XGoXEo};

  private:
    const int                  cpl_to_TLB_write[4] = {2, 2, 2, 6};
    struct decoded_instruction trace_cache[TRACE_CACHE_SIZE];
    struct trace_info          trace_info[TRACE_INFO_ENTRIES];

    const int countLeadingZeros8[256] = {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const float32  float32_negative_inf  = 0xff800000;
    const float32  float32_positive_inf  = 0x7f800000;
    const float32  float32_negative_zero = 0x80000000;
    const float32  float32_positive_zero = 0x00000000;
    const float32  float32_negative_one  = 0xbf800000;
    const float32  float32_positive_one  = 0x3f800000;
    const float32  float32_max_float     = 0x7f7fffff;
    const float32  float32_min_float     = 0xff7fffff;
    const float32  float32_default_nan   = 0xffc00000;
    const float64  float64_negative_inf  = 0xfff0000000000000ULL;
    const float64  float64_positive_inf  = 0x7ff0000000000000ULL;
    const float64  float64_negative_zero = 0x8000000000000000ULL;
    const float64  float64_positive_zero = 0x0000000000000000ULL;
    const float64  float64_negative_one  = 0xbff0000000000000ULL;
    const float64  float64_positive_one  = 0x3ff0000000000000ULL;
    const float64  float64_max_float     = 0x7fefffffffffffffULL;
    const float64  float64_min_float     = 0xffefffffffffffffULL;
    const float64  float64_default_nan   = 0xFFF8000000000000ULL;
    const floatx80 floatx80_default_nan  = {.fraction = 0xC000000000000000ULL, .exp = (0 << 15) | 0xFFFF};
    const float128 float128_default_nan  = {.lo = 0x0000000000000000ULL, .hi = 0xFFFF800000000000ULL};
    const floatx80 floatx80_one          = {.fraction = 0x8000000000000000ULL, .exp = (0 << 15) | 0x3fff};
    const float128 float128_one          = {0x0000000000000000ULL, 0x3fff000000000000ULL};
    const float128 float128_two          = {0x0000000000000000ULL, 0x4000000000000000ULL};
    const float128 float128_ln2inv2      = {0xe1777d0ffda0d23aULL, 0x400071547652b82fULL};
    const floatx80 floatx80_negone       = {.fraction = 0x8000000000000000ULL, .exp = (1 << 15) | 0x3fff};
    const floatx80 floatx80_neghalf      = {.fraction = 0x8000000000000000ULL, .exp = (1 << 15) | 0x3ffe};
    const float128 float128_ln2          = {.lo = 0xf35793c7673007e6ULL, .hi = 0x3ffe62e42fefa39eULL};
    const float128 float128_sqrt3        = {.lo = 0xa73b25742d7078b8ULL, .hi = 0x3fffbb67ae8584caULL};
    const floatx80 floatx80_pi           = {0xc90fdaa22168c235ULL, (0 << 15) | 0x4000};
    const float128 float128_pi2          = {.lo = 0x8469898CC5170416ULL, .hi = 0x3fff921fb54442d1ULL};
    const float128 float128_pi4          = {.lo = 0x8469898CC5170416ULL, .hi = 0x3ffe921fb54442d1ULL};
    const float128 float128_pi6          = {.lo = 0x58465BB32E0F580FULL, .hi = 0x3ffe0c152382d736ULL};

    float128 ln_arr[9] = {{0x0000000000000000, 0x3fff000000000000}, {0x5555555555555555, 0x3ffd555555555555},
                          {0x999999999999999a, 0x3ffc999999999999}, {0x2492492492492492, 0x3ffc249249249249},
                          {0xc71c71c71c71c71c, 0x3ffbc71c71c71c71}, {0x5d1745d1745d1746, 0x3ffb745d1745d174},
                          {0x3b13b13b13b13b14, 0x3ffb3b13b13b13b1}, {0x1111111111111111, 0x3ffb111111111111},
                          {0xe1e1e1e1e1e1e1e2, 0x3ffae1e1e1e1e1e1}};

    float128 sin_arr[11] = {
        {.lo = 0x0000000000000000, .hi = 0x3fff000000000000}, {.lo = 0x5555555555555555, .hi = 0xbffc555555555555},
        {.lo = 0x1111111111111111, .hi = 0x3ff8111111111111}, {.lo = 0xa01a01a01a01a01a, .hi = 0xbff2a01a01a01a01},
        {.lo = 0x38faac1c88e50017, .hi = 0x3fec71de3a556c73}, {.lo = 0x38fe747e4b837dc7, .hi = 0xbfe5ae64567f544e},
        {.lo = 0x97ca38331d23af68, .hi = 0x3fde6124613a86d0}, {.lo = 0xf11d8656b0ee8cb0, .hi = 0xbfd6ae7f3e733b81},
        {.lo = 0xa6b2605197771b00, .hi = 0x3fce952c77030ad4}, {.lo = 0x724ca1ec3b7b9675, .hi = 0xbfc62f49b4681415},
        {.lo = 0x18bef146fcee6e45, .hi = 0x3fbd71b8ef6dcf57}};

    float128 cos_arr[11] = {
        {.lo = 0x0000000000000000, .hi = 0x3fff000000000000}, {.lo = 0x0000000000000000, .hi = 0xbffe000000000000},
        {.lo = 0x5555555555555555, .hi = 0x3ffa555555555555}, {.lo = 0x6c16c16c16c16c17, .hi = 0xbff56c16c16c16c1},
        {.lo = 0xa01a01a01a01a01a, .hi = 0x3fefa01a01a01a01}, {.lo = 0xc72ef016d3ea6679, .hi = 0xbfe927e4fb7789f5},
        {.lo = 0x7b544da987acfe85, .hi = 0x3fe21eed8eff8d89}, {.lo = 0xd20badf145dfa3e5, .hi = 0xbfda93974a8c07c9},
        {.lo = 0xf11d8656b0ee8cb0, .hi = 0x3fd2ae7f3e733b81}, {.lo = 0x77bb004886a2c2ab, .hi = 0xbfca6827863b97d9},
        {.lo = 0x507a9cad2bf8f0bb, .hi = 0x3fc1e542ba402022}};


    float128 atan_arr[11] = {
        {.lo = 0x0000000000000000, .hi = 0x3fff000000000000}, {.lo = 0x5555555555555555, .hi = 0xbffd555555555555},
        {.lo = 0x999999999999999a, .hi = 0x3ffc999999999999}, {.lo = 0x2492492492492492, .hi = 0xbffc249249249249},
        {.lo = 0xc71c71c71c71c71c, .hi = 0x3ffbc71c71c71c71}, {.lo = 0x5d1745d1745d1746, .hi = 0xbffb745d1745d174},
        {.lo = 0x3b13b13b13b13b14, .hi = 0x3ffb3b13b13b13b1}, {.lo = 0x1111111111111111, .hi = 0xbffb111111111111},
        {.lo = 0xe1e1e1e1e1e1e1e2, .hi = 0x3ffae1e1e1e1e1e1}, {.lo = 0x86bca1af286bca1b, .hi = 0xbffaaf286bca1af2},
        {.lo = 0x8618618618618618, .hi = 0x3ffa861861861861}};

    float128 exp_arr[15] = {
        {.lo = 0x0000000000000000, .hi = 0x3fff000000000000}, {.lo = 0x0000000000000000, .hi = 0x3ffe000000000000},
        {.lo = 0x5555555555555555, .hi = 0x3ffc555555555555}, {.lo = 0x5555555555555555, .hi = 0x3ffa555555555555},
        {.lo = 0x1111111111111111, .hi = 0x3ff8111111111111}, {.lo = 0x6c16c16c16c16c17, .hi = 0x3ff56c16c16c16c1},
        {.lo = 0xa01a01a01a01a01a, .hi = 0x3ff2a01a01a01a01}, {.lo = 0xa01a01a01a01a01a, .hi = 0x3fefa01a01a01a01},
        {.lo = 0x38faac1c88e50017, .hi = 0x3fec71de3a556c73}, {.lo = 0xc72ef016d3ea6679, .hi = 0x3fe927e4fb7789f5},
        {.lo = 0x38fe747e4b837dc7, .hi = 0x3fe5ae64567f544e}, {.lo = 0x7b544da987acfe85, .hi = 0x3fe21eed8eff8d89},
        {.lo = 0x97ca38331d23af68, .hi = 0x3fde6124613a86d0}, {.lo = 0xd20badf145dfa3e5, .hi = 0x3fda93974a8c07c9},
        {.lo = 0xf11d8656b0ee8cb0, .hi = 0x3fd6ae7f3e733b81}};

  private:    // function table
    //
    const insn_handler_t jcc32[16] = {&CPU::op_jo32, &CPU::op_jno32, &CPU::op_jb32,  &CPU::op_jnb32,
                                      &CPU::op_jz32, &CPU::op_jnz32, &CPU::op_jbe32, &CPU::op_jnbe32,
                                      &CPU::op_js32, &CPU::op_jns32, &CPU::op_jp32,  &CPU::op_jnp32,
                                      &CPU::op_jl32, &CPU::op_jnl32, &CPU::op_jle32, &CPU::op_jnle32};


    const insn_handler_t jcc16[16] = {&CPU::op_jo16, &CPU::op_jno16, &CPU::op_jb16,  &CPU::op_jnb16,
                                      &CPU::op_jz16, &CPU::op_jnz16, &CPU::op_jbe16, &CPU::op_jnbe16,
                                      &CPU::op_js16, &CPU::op_jns16, &CPU::op_jp16,  &CPU::op_jnp16,
                                      &CPU::op_jl16, &CPU::op_jnl16, &CPU::op_jle16, &CPU::op_jnle16};

    decode_handler_t table[256] = {
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_push_sv,  &CPU::decode_pop_sv,
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_push_sv,  &CPU::decode_0F,
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_push_sv,  &CPU::decode_pop_sv,
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_push_sv,  &CPU::decode_pop_sv,
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_prefix,   &CPU::decode_27,
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_prefix,   &CPU::decode_2F,
        &CPU::decode_arith_00, &CPU::decode_arith_01, &CPU::decode_arith_02, &CPU::decode_arith_03,
        &CPU::decode_arith_04, &CPU::decode_arith_05, &CPU::decode_prefix,   &CPU::decode_37,
        &CPU::decode_38,       &CPU::decode_39,       &CPU::decode_3A,       &CPU::decode_3B,
        &CPU::decode_3C,       &CPU::decode_3D,       &CPU::decode_prefix,   &CPU::decode_3F,
        &CPU::decode_inc_rv,   &CPU::decode_inc_rv,   &CPU::decode_inc_rv,   &CPU::decode_inc_rv,
        &CPU::decode_inc_rv,   &CPU::decode_inc_rv,   &CPU::decode_inc_rv,   &CPU::decode_inc_rv,
        &CPU::decode_dec_rv,   &CPU::decode_dec_rv,   &CPU::decode_dec_rv,   &CPU::decode_dec_rv,
        &CPU::decode_dec_rv,   &CPU::decode_dec_rv,   &CPU::decode_dec_rv,   &CPU::decode_dec_rv,
        &CPU::decode_push_rv,  &CPU::decode_push_rv,  &CPU::decode_push_rv,  &CPU::decode_push_rv,
        &CPU::decode_push_rv,  &CPU::decode_push_rv,  &CPU::decode_push_rv,  &CPU::decode_push_rv,
        &CPU::decode_pop_rv,   &CPU::decode_pop_rv,   &CPU::decode_pop_rv,   &CPU::decode_pop_rv,
        &CPU::decode_pop_rv,   &CPU::decode_pop_rv,   &CPU::decode_pop_rv,   &CPU::decode_pop_rv,
        &CPU::decode_60,       &CPU::decode_61,       &CPU::decode_62,       &CPU::decode_63,
        &CPU::decode_prefix,   &CPU::decode_prefix,   &CPU::decode_prefix,   &CPU::decode_prefix,
        &CPU::decode_68,       &CPU::decode_69,       &CPU::decode_6A,       &CPU::decode_6B,
        &CPU::decode_6C,       &CPU::decode_6D,       &CPU::decode_6E,       &CPU::decode_6F,
        &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,
        &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,
        &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,
        &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,     &CPU::decode_jcc8,
        &CPU::decode_80,       &CPU::decode_81,       &CPU::decode_80,       &CPU::decode_83,
        &CPU::decode_84,       &CPU::decode_85,       &CPU::decode_86,       &CPU::decode_87,
        &CPU::decode_88,       &CPU::decode_89,       &CPU::decode_8A,       &CPU::decode_8B,
        &CPU::decode_8C,       &CPU::decode_8D,       &CPU::decode_8E,       &CPU::decode_8F,
        &CPU::decode_90,       &CPU::decode_xchg,     &CPU::decode_xchg,     &CPU::decode_xchg,
        &CPU::decode_xchg,     &CPU::decode_xchg,     &CPU::decode_xchg,     &CPU::decode_xchg,
        &CPU::decode_98,       &CPU::decode_99,       &CPU::decode_9A,       &CPU::decode_9B,
        &CPU::decode_9C,       &CPU::decode_9D,       &CPU::decode_9E,       &CPU::decode_9F,
        &CPU::decode_A0,       &CPU::decode_A1,       &CPU::decode_A2,       &CPU::decode_A3,
        &CPU::decode_A4,       &CPU::decode_A5,       &CPU::decode_A6,       &CPU::decode_A7,
        &CPU::decode_A8,       &CPU::decode_A9,       &CPU::decode_AA,       &CPU::decode_AB,
        &CPU::decode_AC,       &CPU::decode_AD,       &CPU::decode_AE,       &CPU::decode_AF,
        &CPU::decode_mov_rbib, &CPU::decode_mov_rbib, &CPU::decode_mov_rbib, &CPU::decode_mov_rbib,
        &CPU::decode_mov_rbib, &CPU::decode_mov_rbib, &CPU::decode_mov_rbib, &CPU::decode_mov_rbib,
        &CPU::decode_mov_rviv, &CPU::decode_mov_rviv, &CPU::decode_mov_rviv, &CPU::decode_mov_rviv,
        &CPU::decode_mov_rviv, &CPU::decode_mov_rviv, &CPU::decode_mov_rviv, &CPU::decode_mov_rviv,
        &CPU::decode_C0,       &CPU::decode_C1,       &CPU::decode_C2,       &CPU::decode_C3,
        &CPU::decode_C4,       &CPU::decode_C5,       &CPU::decode_C6,       &CPU::decode_C7,
        &CPU::decode_C8,       &CPU::decode_C9,       &CPU::decode_CA,       &CPU::decode_CB,
        &CPU::decode_CC,       &CPU::decode_CD,       &CPU::decode_CE,       &CPU::decode_CF,
        &CPU::decode_D0,       &CPU::decode_D1,       &CPU::decode_D2,       &CPU::decode_D3,
        &CPU::decode_D4,       &CPU::decode_D5,       &CPU::decode_invalid,  &CPU::decode_D7,
        &CPU::decode_fpu,      &CPU::decode_fpu,      &CPU::decode_fpu,      &CPU::decode_fpu,
        &CPU::decode_fpu,      &CPU::decode_fpu,      &CPU::decode_fpu,      &CPU::decode_fpu,
        &CPU::decode_E0,       &CPU::decode_E1,       &CPU::decode_E2,       &CPU::decode_E3,
        &CPU::decode_E4,       &CPU::decode_E5,       &CPU::decode_E6,       &CPU::decode_E7,
        &CPU::decode_E8,       &CPU::decode_E9,       &CPU::decode_EA,       &CPU::decode_EB,
        &CPU::decode_EC,       &CPU::decode_ED,       &CPU::decode_EE,       &CPU::decode_EF,
        &CPU::decode_prefix,   &CPU::decode_invalid,  &CPU::decode_prefix,   &CPU::decode_prefix,
        &CPU::decode_F4,       &CPU::decode_F5,       &CPU::decode_F6,       &CPU::decode_F7,
        &CPU::decode_F8,       &CPU::decode_F9,       &CPU::decode_FA,       &CPU::decode_FB,
        &CPU::decode_FC,       &CPU::decode_FD,       &CPU::decode_FE,       &CPU::decode_FF};




    decode_handler_t table0F[256] = {&CPU::decode_0F00,
                                     &CPU::decode_0F01,
                                     &CPU::decode_0F02,
                                     &CPU::decode_0F03,
                                     &CPU::decode_ud,
                                     &CPU::decode_ud,
                                     &CPU::decode_0F06,
                                     &CPU::decode_ud,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_0F09,
                                     &CPU::decode_ud,
                                     &CPU::decode_0F0B,
                                     &CPU::decode_ud,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_ud,
                                     &CPU::decode_ud,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_sse10_17,
                                     &CPU::decode_0F18,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_0F1F,
                                     &CPU::decode_0F20,
                                     &CPU::decode_0F21,
                                     &CPU::decode_0F22,
                                     &CPU::decode_0F23,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_sse28_2F,
                                     &CPU::decode_0F30,
                                     &CPU::decode_0F31,
                                     &CPU::decode_0F32,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_sysenter_sysexit,
                                     &CPU::decode_sysenter_sysexit,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_0F38,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_cmov,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse50_57,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse58_5F,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse60_67,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse68_6F,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_sse70_76,
                                     &CPU::decode_0F77,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_sse7C_7D,
                                     &CPU::decode_sse7C_7D,
                                     &CPU::decode_sse7E_7F,
                                     &CPU::decode_sse7E_7F,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_jccv,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_setcc,
                                     &CPU::decode_0FA0,
                                     &CPU::decode_0FA1,
                                     &CPU::decode_0FA2,
                                     &CPU::decode_0FA3,
                                     &CPU::decode_0FA4,
                                     &CPU::decode_0FA5,
                                     &CPU::decode_ud,
                                     &CPU::decode_ud,
                                     &CPU::decode_0FA8,
                                     &CPU::decode_0FA9,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_0FAB,
                                     &CPU::decode_0FAC,
                                     &CPU::decode_0FAD,
                                     &CPU::decode_0FAE,
                                     &CPU::decode_0FAF,
                                     &CPU::decode_0FB0,
                                     &CPU::decode_0FB1,
                                     &CPU::decode_0FB2,
                                     &CPU::decode_0FB3,
                                     &CPU::decode_0FB4,
                                     &CPU::decode_0FB5,
                                     &CPU::decode_0FB6,
                                     &CPU::decode_0FB7,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_0FBA,
                                     &CPU::decode_0FBB,
                                     &CPU::decode_0FBC,
                                     &CPU::decode_0FBD,
                                     &CPU::decode_0FBE,
                                     &CPU::decode_0FBF,
                                     &CPU::decode_0FC0,
                                     &CPU::decode_0FC1,
                                     &CPU::decode_sseC2_C6,
                                     &CPU::decode_sseC2_C6,
                                     &CPU::decode_sseC2_C6,
                                     &CPU::decode_sseC2_C6,
                                     &CPU::decode_sseC2_C6,
                                     &CPU::decode_0FC7,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_bswap,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD0_D7,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseD8_DF,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE0_E7,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_sseE8_EF,
                                     &CPU::decode_invalid0F,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF1_F7,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_sseF8_FE,
                                     &CPU::decode_ud};
};
#endif