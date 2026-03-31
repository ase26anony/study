/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force aggressive optimization on specific functions */
#define AGGRESSIVE_OPT __attribute__((optimize("O3", "no-inline")))

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Test 10-operand case using AVX-512 complex permute operation */
AGGRESSIVE_OPT
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 has implicit operands + mask + immediate */
    /* This expands to an RTL pattern with multiple operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    volatile __m512i* dummy = &result;
    (void)dummy;
}

/* Test 11-operand case using multiple operations combined */
AGGRESSIVE_OPT
void test_11_operand_x86(void) {
    /* Complex sequence that may combine into multi-operand pattern */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    __mmask16 m1 = 0xAAAA;
    __mmask16 m2 = 0x5555;
    
    /* Chain of operations that might be combined */
    __m512i t1 = _mm512_mask_add_epi32(a, m1, b, c);
    __m512i t2 = _mm512_mask_mul_epi32(t1, m2, c, d);
    __m512i result = _mm512_mask_permutexvar_epi32(t2, m1, _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0), t1);
    
    volatile __m512i* dummy = &result;
    (void)dummy;
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* Test 10-operand case using ARM NEON complex operations */
AGGRESSIVE_OPT
void test_10_operand_arm(void) {
    /* Complex NEON operation with table lookup - may generate multi-operand RTL */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Table lookup with multiple registers */
    uint8x16_t result = vqtbl4q_u8(
        (uint8x16x4_t){a, b, c, d},
        vdupq_n_u8(0)
    );
    
    volatile uint8x16_t* dummy = &result;
    (void)dummy;
}

/* Test 11-operand case using inline assembly */
AGGRESSIVE_OPT
void test_11_operand_arm(void) {
    uint64_t op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, result;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10;
    
    /* Inline assembly with exactly 11 operands */
    asm volatile (
        "/* Complex multi-operand operation */\n\t"
        "add %[res], %[a], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9),
          [j] "r" (op10)
        : "cc"
    );
    
    volatile uint64_t* dummy = &result;
    (void)dummy;
}

#endif /* __aarch64__ */

/* Generic fallback using complex inline assembly for 10 and 11 operands */
AGGRESSIVE_OPT
void test_10_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, result;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9;
    
    /* Inline assembly with 10 operands (9 inputs + 1 output) */
    asm volatile (
        "/* 10-operand test pattern */\n\t"
        "mov %[res], %[a]\n\t"
        "add %[res], %[res], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]"
        : [res] "=r" (result)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9)
        : "cc"
    );
    
    volatile long* dummy = &result;
    (void)dummy;
}

AGGRESSIVE_OPT
void test_11_operand_generic(void) {
    long op1, op2, op3, op4, op5, op6, op7, op8, op9, op10, result;
    
    op1 = 1; op2 = 2; op3 = 3; op4 = 4; op5 = 5;
    op6 = 6; op7 = 7; op8 = 8; op9 = 9; op10 = 10;
    
    /* Inline assembly with 11 operands (10 inputs + 1 output) */
    asm volatile (
        "/* 11-operand test pattern */\n\t"
        "mov %[res], %[a]\n\t"
        "add %[res], %[res], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]"
        : [res] "=r" (result)
        : [a] "r" (op1), [b] "r" (op2), [c] "r" (op3),
          [d] "r" (op4), [e] "r" (op5), [f] "r" (op6),
          [g] "r" (op7), [h] "r" (op8), [i] "r" (op9),
          [j] "r" (op10)
        : "cc"
    );
    
    volatile long* dummy = &result;
    (void)dummy;
}

/* OpenMP SIMD function that might generate complex RTL patterns */
#pragma omp declare simd
AGGRESSIVE_OPT
void simd_chain_operation(float* restrict a, float* restrict b, 
                          float* restrict c, float* restrict d, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        /* Complex chain that might be combined into multi-operand pattern */
        float t1 = a[i] + b[i];
        float t2 = t1 * c[i];
        float t3 = t2 - d[i];
        float t4 = t3 / (a[i] + 1.0f);
        float t5 = t4 * 2.0f;
        a[i] = t5;
    }
}

/* Main driver */
int main() {
    printf("Testing multi-operand RTL pattern generation...\n");
    
    /* Call architecture-specific tests */
#ifdef __x86_64__
    printf("Running x86_64 tests...\n");
    test_10_operand_x86();
    test_11_operand_x86();
#endif
    
#ifdef __aarch64__
    printf("Running AArch64 tests...\n");
    test_10_operand_arm();
    test_11_operand_arm();
#endif
    
    /* Always run generic tests */
    printf("Running generic tests...\n");
    test_10_operand_generic();
    test_11_operand_generic();
    
    /* Test SIMD chain */
    float a[16], b[16], c[16], d[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
        c[i] = i * 3.0f;
        d[i] = i * 4.0f;
    }
    
    simd_chain_operation(a, b, c, d, 16);
    
    /* Use results to prevent dead code elimination */
    volatile float sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += a[i];
    }
    
    printf("Test completed. Sum: %f\n", sum);
    return 0;
}
