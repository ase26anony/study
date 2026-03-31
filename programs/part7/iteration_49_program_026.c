/* Test program to cover 10 and 11 operand cases in optabs.cc (lines 8254-8263) */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force aggressive optimization on specific functions */
#define AGGRESSIVE_OPT __attribute__((optimize("O3", "no-inline")))

/* Prevent dead code elimination */
#define USE_RESULT(var) asm volatile("" : "+r"(var))

/* Generic fallback for architectures without specific intrinsics */
#ifndef __x86_64__
#ifndef __aarch64__
#define GENERIC_FALLBACK
#endif
#endif

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function targeting 10-operand case using AVX-512 complex permute */
AGGRESSIVE_OPT
void test_10_operand_x86(void) {
    /* Complex AVX-512 masked permute with multiple immediates */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic typically expands to RTL with many operands:
       dest, mask, idx, src1, src2, plus various immediates and modes */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    volatile __m512i* volatile_ptr = &result;
    USE_RESULT(volatile_ptr);
    
    /* Additional complex operation with multiple parameters */
    __m512i a = _mm512_set1_epi32(42);
    __m512i b = _mm512_set1_epi32(24);
    __m512i c = _mm512_set1_epi32(100);
    
    /* FMA with multiple sources and control */
    __m512 f1 = _mm512_set1_ps(1.0f);
    __m512 f2 = _mm512_set1_ps(2.0f);
    __m512 f3 = _mm512_set1_ps(3.0f);
    __m512 fma_result = _mm512_fmadd_ps(f1, f2, f3);
    
    USE_RESULT(fma_result);
}

/* Function targeting 11-operand case using multiple AVX-512 operations */
AGGRESSIVE_OPT
void test_11_operand_x86(void) {
    /* Complex inline assembly with 11 operands */
    __m512i v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11;
    
    v1 = _mm512_set1_epi32(1);
    v2 = _mm512_set1_epi32(2);
    v3 = _mm512_set1_epi32(3);
    v4 = _mm512_set1_epi32(4);
    v5 = _mm512_set1_epi32(5);
    v6 = _mm512_set1_epi32(6);
    v7 = _mm512_set1_epi32(7);
    v8 = _mm512_set1_epi32(8);
    v9 = _mm512_set1_epi32(9);
    v10 = _mm512_set1_epi32(10);
    v11 = _mm512_setzero_si512();
    
    /* Complex multi-operand inline assembly pattern */
    asm volatile (
        "vpaddd %{z%0, %1, %2|%0, %1, %2%{z%}}\n\t"
        "vpsubd %{z%3, %4, %5|%3, %4, %5%{z%}}\n\t"
        "vpmulld %{z%6, %7, %8|%6, %7, %8%{z%}}\n\t"
        "vpblendmd %{z%9, %10, %11, %k12|%9, %10, %11%{z%}, %k12}"
        : "=v"(v1), "=v"(v3), "=v"(v5), "=v"(v7), "=v"(v9), "=v"(v11)
        : "0"(v1), "v"(v2), "v"(v3), "v"(v4), "v"(v5), "v"(v6),
          "v"(v7), "v"(v8), "v"(v9), "v"(v10), "k"(0xFF)
        : "cc"
    );
    
    /* Use results */
    USE_RESULT(v1);
    USE_RESULT(v3);
    USE_RESULT(v5);
    USE_RESULT(v7);
    USE_RESULT(v9);
    USE_RESULT(v11);
    
    /* Additional complex masked gather with many parameters */
    long long base[64] = {0};
    __m512i vindex = _mm512_set1_epi64(8);
    __mmask8 gather_mask = 0x0F;
    
    __m512i gathered = _mm512_mask_i64gather_epi64(
        _mm512_setzero_si512(),
        gather_mask,
        vindex,
        base,
        8
    );
    
    USE_RESULT(gathered);
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* Function targeting 10-operand case using ARM NEON complex operations */
AGGRESSIVE_OPT
void test_10_operand_arm(void) {
    /* Complex NEON operations with multiple vector registers */
    uint8x16_t v1 = vdupq_n_u8(1);
    uint8x16_t v2 = vdupq_n_u8(2);
    uint8x16_t v3 = vdupq_n_u8(3);
    uint8x16_t v4 = vdupq_n_u8(4);
    uint8x16_t v5 = vdupq_n_u8(5);
    uint8x16_t v6 = vdupq_n_u8(6);
    uint8x16_t v7 = vdupq_n_u8(7);
    uint8x16_t v8 = vdupq_n_u8(8);
    uint8x16_t v9 = vdupq_n_u8(9);
    uint8x16_t v10 = vdupq_n_u8(10);
    
    /* Complex sequence that may combine into multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(v1, v2);
    uint8x16_t r2 = vsubq_u8(v3, v4);
    uint8x16_t r3 = vmulq_u8(v5, v6);
    uint8x16_t r4 = vmlaq_u8(v7, v8, v9);  /* v7 + v8 * v9 */
    
    /* Table lookup with multiple registers */
    uint8x16_t result = vqtbl4q_u8(
        vcreate4_u8(
            vreinterpretq_u64_u8(v1),
            vreinterpretq_u64_u8(v2),
            vreinterpretq_u64_u8(v3),
            vreinterpretq_u64_u8(v4)
        ),
        v10
    );
    
    USE_RESULT(r1);
    USE_RESULT(r2);
    USE_RESULT(r3);
    USE_RESULT(r4);
    USE_RESULT(result);
    
    /* Complex inline assembly with many operands */
    asm volatile (
        "add %0.16b, %1.16b, %2.16b\n\t"
        "sub %3.16b, %4.16b, %5.16b\n\t"
        "mul %6.16b, %7.16b, %8.16b\n\t"
        "mla %9.16b, %10.16b, %11.16b"
        : "=w"(v1), "=w"(v3), "=w"(v5), "=w"(v7)
        : "0"(v1), "w"(v2), "w"(v3), "w"(v4),
          "w"(v5), "w"(v6), "w"(v7), "w"(v8),
          "w"(v9)
        :
    );
}

/* Function targeting 11-operand case using ARM SVE-like patterns */
AGGRESSIVE_OPT
void test_11_operand_arm(void) {
    /* Use inline assembly with exactly 11 operands */
    uint64x2_t a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
    
    a1 = vdupq_n_u64(1);
    a2 = vdupq_n_u64(2);
    a3 = vdupq_n_u64(3);
    a4 = vdupq_n_u64(4);
    a5 = vdupq_n_u64(5);
    a6 = vdupq_n_u64(6);
    a7 = vdupq_n_u64(7);
    a8 = vdupq_n_u64(8);
    a9 = vdupq_n_u64(9);
    a10 = vdupq_n_u64(10);
    a11 = vdupq_n_u64(11);
    
    /* 11-operand inline assembly pattern */
    asm volatile (
        "/* Complex 11-operand pattern */\n\t"
        "add %0.2d, %1.2d, %2.2d\n\t"
        "sub %3.2d, %4.2d, %5.2d\n\t"
        "mul %6.2d, %7.2d, %8.2d\n\t"
        "mla %9.2d, %10.2d, %11.2d"
        : "=w"(a1), "=w"(a3), "=w"(a5), "=w"(a7), "=w"(a9), "=w"(a11)
        : "0"(a1), "w"(a2), "w"(a3), "w"(a4),
          "w"(a5), "w"(a6), "w"(a7), "w"(a8),
          "w"(a9), "w"(a10)
        : "cc"
    );
    
    USE_RESULT(a1);
    USE_RESULT(a3);
    USE_RESULT(a5);
    USE_RESULT(a7);
    USE_RESULT(a9);
    USE_RESULT(a11);
}

#endif /* __aarch64__ */

#ifdef GENERIC_FALLBACK
/* Generic fallback using complex inline assembly with many operands */
AGGRESSIVE_OPT
void test_10_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10;
    
    /* 10-operand inline assembly */
    asm volatile (
        "/* 10-operand test pattern */\n\t"
        "add %0, %1, %2\n\t"
        "sub %3, %4, %5\n\t"
        "mul %6, %7, %8\n\t"
        "add %9, %0, %3"
        : "=r"(op1), "=r"(op3), "=r"(op5), "=r"(op7), "=r"(op9)
        : "0"(op1), "r"(op2), "r"(op3), "r"(op4),
          "r"(op5), "r"(op6), "r"(op7), "r"(op8),
          "r"(op9)
        : "cc"
    );
    
    USE_RESULT(op1);
    USE_RESULT(op3);
    USE_RESULT(op5);
    USE_RESULT(op7);
    USE_RESULT(op9);
}

AGGRESSIVE_OPT
void test_11_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, op11;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10; op11 = 11;
    
    /* 11-operand inline assembly */
    asm volatile (
        "/* 11-operand test pattern */\n\t"
        "add %0, %1, %2\n\t"
        "sub %3, %4, %5\n\t"
        "mul %6, %7, %8\n\t"
        "div %9, %10, %11"
        : "=r"(op1), "=r"(op3), "=r"(op5), "=r"(op7), "=r"(op9), "=r"(op11)
        : "0"(op1), "r"(op2), "r"(op3), "r"(op4),
          "r"(op5), "r"(op6), "r"(op7), "r"(op8),
          "r"(op9), "r"(op10), "r"(op11)
        : "cc"
    );
    
    USE_RESULT(op1);
    USE_RESULT(op3);
    USE_RESULT(op5);
    USE_RESULT(op7);
    USE_RESULT(op9);
    USE_RESULT(op11);
}
#endif /* GENERIC_FALLBACK */

/* Wrapper functions that select appropriate implementation */
AGGRESSIVE_OPT
void test_10_operand(void) {
#ifdef __x86_64__
    test_10_operand_x86();
#elif defined(__aarch64__)
    test_10_operand_arm();
#else
    test_10_operand_generic();
#endif
}

AGGRESSIVE_OPT
void test_11_operand(void) {
#ifdef __x86_64__
    test_11_operand_x86();
#elif defined(__aarch64__)
    test_11_operand_arm();
#else
    test_11_operand_generic();
#endif
}

/* Main driver */
int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    
    printf("Tests completed (check RTL dumps for coverage)\n");
    
    /* Simple computation to prevent optimization */
    volatile int check = 0;
    for (int i = 0; i < 100; i++) {
        check += i;
    }
    
    return check == 0 ? 0 : 1;
}
