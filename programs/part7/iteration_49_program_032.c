/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_result = 0;

/* Function attribute to force optimization and RTL expansion */
#define FORCE_EXPAND __attribute__((optimize("O3"), noinline, target("arch=native")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test function for 10-operand case using AVX-512 intrinsics */
FORCE_EXPAND
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands:
     * _mm512_mask_permutex2var_epi64 has 4 register operands + 1 immediate = 5 RTL operands
     * Combined with mask load/store and arithmetic, we can reach 10 operands */
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic expands to multiple RTL patterns, some with many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Additional operations to create complex RTL pattern */
    __m512i add_result = _mm512_add_epi64(result, src1);
    __m512i mul_result = _mm512_mullo_epi64(add_result, src2);
    
    /* Blend with mask - another multi-operand operation */
    __m512i blend_result = _mm512_mask_blend_epi64(mask, src1, mul_result);
    
    /* Store with mask - potentially creates complex RTL */
    _mm512_mask_storeu_epi64((void*)&g_result, mask, blend_result);
}

/* Test function for 11-operand case using inline assembly */
FORCE_EXPAND
void test_11_operand_x86(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3;
    
    /* Inline assembly with 11 operands: 8 inputs, 3 outputs */
    asm volatile (
        /* Complex operation with many operands */
        "mov %[a], %[out1]\n\t"
        "add %[b], %[out1]\n\t"
        "imul %[c], %[out1]\n\t"
        "mov %[d], %[out2]\n\t"
        "sub %[e], %[out2]\n\t"
        "mov %[f], %[out3]\n\t"
        "xor %[g], %[out3]\n\t"
        "or %[h], %[out1]\n\t"
        "and %[i], %[out2]\n\t"
        "shl $3, %[out3]\n\t"
        "add %[j], %[out1]\n\t"
        "sub %[k], %[out2]"
        : [out1] "=&r" (result1), [out2] "=&r" (result2), [out3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [f] "r" (f), [g] "r" (g), [h] "r" (h), [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_result += result1 + result2 + result3;
}

#elif defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>

/* Test function for 10-operand case using ARM NEON intrinsics */
FORCE_EXPAND
void test_10_operand_arm(void) {
    /* Complex NEON operations with table lookups and multiple registers */
    uint8x16_t src1 = vdupq_n_u8(1);
    uint8x16_t src2 = vdupq_n_u8(2);
    uint8x16_t src3 = vdupq_n_u8(3);
    uint8x16_t tbl_idx = vcombine_u8(
        vcreate_u8(0x0706050403020100),
        vcreate_u8(0x0F0E0D0C0B0A0908)
    );
    
    /* Table lookup with 3 source registers = many operands */
    uint8x16_t tbl_result = vqtbl3q_u8(
        vld1q_u8_x3((uint8_t*)&src1),
        tbl_idx
    );
    
    /* Multiple arithmetic operations chained together */
    uint8x16_t add_result = vaddq_u8(tbl_result, src2);
    uint8x16_t mul_result = vmulq_u8(add_result, src3);
    
    /* Complex permute/zip operations */
    uint8x16_t zip_lo = vzip1q_u8(mul_result, src1);
    uint8x16_t zip_hi = vzip2q_u8(mul_result, src2);
    
    uint8x16_t final_result = vaddq_u8(zip_lo, zip_hi);
    
    /* Store result */
    vst1q_u8((uint8_t*)&g_result, final_result);
}

/* Test function for 11-operand case using ARM inline assembly */
FORCE_EXPAND
void test_11_operand_arm(void) {
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    uint32_t out1, out2, out3;
    
    /* Initialize registers */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5;
    r5 = 6; r6 = 7; r7 = 8; r8 = 9; r9 = 10; r10 = 11;
    
    /* Inline assembly with 11 operands */
    asm volatile (
        /* Complex sequence using many registers */
        "add %[out1], %[r0], %[r1]\n\t"
        "mul %[out1], %[out1], %[r2]\n\t"
        "sub %[out2], %[r3], %[r4]\n\t"
        "and %[out2], %[out2], %[r5]\n\t"
        "orr %[out3], %[r6], %[r7]\n\t"
        "eor %[out3], %[out3], %[r8]\n\t"
        "add %[out1], %[out1], %[r9]\n\t"
        "sub %[out2], %[out2], %[r10]\n\t"
        "lsl %[out3], %[out3], #2"
        : [out1] "=&r" (out1), [out2] "=&r" (out2), [out3] "=&r" (out3)
        : [r0] "r" (r0), [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3),
          [r4] "r" (r4), [r5] "r" (r5), [r6] "r" (r6), [r7] "r" (r7),
          [r8] "r" (r8), [r9] "r" (r9), [r10] "r" (r10)
        : "cc"
    );
    
    g_result += out1 + out2 + out3;
}

#else
/* Generic fallback using complex inline assembly with many operands */

FORCE_EXPAND
void test_10_operand_generic(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result1, result2;
    
    /* Inline assembly with exactly 10 operands */
    asm volatile (
        "mov %[r1], %[a1]\n\t"
        "add %[r1], %[a2]\n\t"
        "imul %[r1], %[a3]\n\t"
        "mov %[r2], %[a4]\n\t"
        "sub %[r2], %[a5]\n\t"
        "and %[r2], %[a6]\n\t"
        "or %[r1], %[a7]\n\t"
        "xor %[r2], %[a8]\n\t"
        "add %[r1], %[a9]\n\t"
        "sub %[r2], %[a10]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10)
        : "cc"
    );
    
    g_result += result1 + result2;
}

FORCE_EXPAND
void test_11_operand_generic(void) {
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5, op6 = 6;
    long op7 = 7, op8 = 8, op9 = 9, op10 = 10, op11 = 11;
    long result1, result2, result3;
    
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "mov %[r1], %[a1]\n\t"
        "add %[r1], %[a2]\n\t"
        "mov %[r2], %[a3]\n\t"
        "sub %[r2], %[a4]\n\t"
        "mov %[r3], %[a5]\n\t"
        "xor %[r3], %[a6]\n\t"
        "imul %[r1], %[a7]\n\t"
        "and %[r2], %[a8]\n\t"
        "or %[r3], %[a9]\n\t"
        "add %[r1], %[a10]\n\t"
        "sub %[r2], %[a11]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a1] "r" (op1), [a2] "r" (op2), [a3] "r" (op3),
          [a4] "r" (op4), [a5] "r" (op5), [a6] "r" (op6),
          [a7] "r" (op7), [a8] "r" (op8), [a9] "r" (op9),
          [a10] "r" (op10), [a11] "r" (op11)
        : "cc"
    );
    
    g_result += result1 + result2 + result3;
}
#endif

/* Wrapper functions to call architecture-specific implementations */
void test_10_operand(void) {
#ifdef __x86_64__
    test_10_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    test_10_operand_arm();
#else
    test_10_operand_generic();
#endif
}

void test_11_operand(void) {
#ifdef __x86_64__
    test_11_operand_x86();
#elif defined(__aarch64__) || defined(__arm__)
    test_11_operand_arm();
#else
    test_11_operand_generic();
#endif
}

/* Main driver */
int main(void) {
    printf("Testing 10 and 11 operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", g_result);
    
    return 0;
}
