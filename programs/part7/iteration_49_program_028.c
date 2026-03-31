/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_volatile = 0;

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function to trigger 10-operand case using AVX-512 intrinsics */
__attribute__((noinline, optimize("O3")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 has many operands in RTL:
       dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    g_volatile = _mm512_extract_epi64(result, 0);
}

/* Another 10-operand pattern using blend with immediate */
__attribute__((noinline, optimize("O3")))
void test_10_operand_x86_2(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    __mmask8 mask = 0xAA;
    
    /* _mm512_mask_blend_pd expands to many operands */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    g_volatile = (int)_mm512_cvtsd_f64(result);
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand_x86(void) {
    /* Use inline assembly with exactly 11 operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    uint64_t result1, result2, result3;
    
    /* Complex inline assembly with 11 total operands:
       3 outputs + 8 inputs = 11 operands */
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "imul %1, %6\n\t"
        "mov %2, %7\n\t"
        "sub %2, %8\n\t"
        "xor %0, %9\n\t"
        "or %1, %10\n\t"
        "and %2, %11"
        : "=r"(result1), "=r"(result2), "=r"(result3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    g_volatile = result1 + result2 + result3;
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* Function to trigger 10-operand case using ARM NEON */
__attribute__((noinline, optimize("O3")))
void test_10_operand_arm(void) {
    /* Complex vector table lookup with multiple operands */
    uint8x16_t data = vdupq_n_u8(1);
    uint8x16_t table1 = vdupq_n_u8(2);
    uint8x16_t table2 = vdupq_n_u8(3);
    uint8x16_t table3 = vdupq_n_u8(4);
    
    /* vtbl4_u8 and related operations can have many operands in RTL */
    uint8x8x4_t tbl = { 
        vget_low_u8(table1), 
        vget_high_u8(table1),
        vget_low_u8(table2),
        vget_high_u8(table2)
    };
    
    uint8x8_t result = vtbl4_u8(tbl, vget_low_u8(data));
    
    g_volatile = vget_lane_u8(result, 0);
}

/* Function to trigger 11-operand case for ARM */
__attribute__((noinline, optimize("O3")))
void test_11_operand_arm(void) {
    /* Inline assembly with 11 operands for ARM */
    uint64_t in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5, in6 = 6;
    uint64_t in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    uint64_t out1, out2, out3;
    
    asm volatile (
        "add %0, %3, %4\n\t"
        "mul %1, %5, %6\n\t"
        "sub %2, %7, %8\n\t"
        "eor %0, %0, %9\n\t"
        "orr %1, %1, %10\n\t"
        "and %2, %2, %11\n\t"
        "lsl %0, %0, #2\n\t"
        "lsr %1, %1, #1"
        : "=r"(out1), "=r"(out2), "=r"(out3)
        : "r"(in1), "r"(in2), "r"(in3), "r"(in4), 
          "r"(in5), "r"(in6), "r"(in7), "r"(in8), "r"(in9)
        : "cc"
    );
    
    g_volatile = out1 + out2 + out3;
}

#endif /* __aarch64__ */

/* Generic fallback using complex inline assembly with many operands */
__attribute__((noinline, optimize("O3")))
void test_10_operand_generic(void) {
    /* Generic inline assembly with 10 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9;
    long result1, result2;
    
    asm volatile (
        "mov %0, %2\n\t"
        "add %0, %3\n\t"
        "mov %1, %4\n\t"
        "sub %1, %5\n\t"
        "imul %0, %6\n\t"
        "xor %1, %7\n\t"
        "or %0, %8\n\t"
        "and %1, %9"
        : "=r"(result1), "=r"(result2)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h)
        : "cc"
    );
    
    g_volatile = result1 + result2;
}

__attribute__((noinline, optimize("O3")))
void test_11_operand_generic(void) {
    /* Generic inline assembly with 11 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result1, result2, result3;
    
    asm volatile (
        "mov %0, %3\n\t"
        "add %0, %4\n\t"
        "mov %1, %5\n\t"
        "sub %1, %6\n\t"
        "mov %2, %7\n\t"
        "xor %2, %8\n\t"
        "imul %0, %9\n\t"
        "or %1, %10\n\t"
        "and %2, %11"
        : "=r"(result1), "=r"(result2), "=r"(result3)
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g), "r"(h), "r"(i)
        : "cc"
    );
    
    g_volatile = result1 + result2 + result3;
}

/* Main test driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
#ifdef __x86_64__
    printf("Using x86_64 intrinsics and assembly\n");
    test_10_operand_x86();
    test_10_operand_x86_2();
    test_11_operand_x86();
#elif defined(__aarch64__)
    printf("Using AArch64 NEON and assembly\n");
    test_10_operand_arm();
    test_11_operand_arm();
#else
    printf("Using generic inline assembly\n");
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    printf("Result: %d\n", g_volatile);
    return 0;
}
