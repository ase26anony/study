/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_result = 0;

#ifdef __x86_64__
#include <immintrin.h>
#include <x86intrin.h>

/* Function targeting 10-operand case using AVX-512 intrinsics */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86(void) {
    /* Complex AVX-512 operation with many operands */
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 expands to many operands:
       dest, mask, idx, src1, src2 */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    g_result += _mm512_reduce_add_epi64(result);
}

/* Another 10-operand pattern using blend with multiple sources */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_x86_v2(void) {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    __m512 c = _mm512_set1_ps(3.0f);
    __m512 d = _mm512_set1_ps(4.0f);
    __mmask16 mask = 0xAAAA;
    
    /* Complex blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, 
        _mm512_add_ps(a, b),
        _mm512_add_ps(c, d));
    
    /* Use result */
    float sum = _mm512_reduce_add_ps(result);
    g_result += (int)sum;
}

/* Function targeting 11-operand case */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_x86(void) {
    /* Use inline assembly with exactly 11 operands */
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g_val = 7, h = 8, i = 9, j = 10;
    uint64_t k = 11;
    uint64_t result;
    
    /* 11-operand inline asm: 10 inputs + 1 output */
    asm volatile (
        "/* Complex 11-operand pattern */\n\t"
        "mov %[a], %[res]\n\t"
        "add %[b], %[res]\n\t"
        "add %[c], %[res]\n\t"
        "add %[d], %[res]\n\t"
        "add %[e], %[res]\n\t"
        "add %[f], %[res]\n\t"
        "add %[g], %[res]\n\t"
        "add %[h], %[res]\n\t"
        "add %[i], %[res]\n\t"
        "add %[j], %[res]\n\t"
        "add %[k], %[res]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g_val), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_result += result;
}

/* Another 11-operand test using AVX-512 gather with complex addressing */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_x86_gather(void) {
    int base[64] = {0};
    __m512i vindex = _mm512_set_epi32(0, 4, 8, 12, 16, 20, 24, 28,
                                      32, 36, 40, 44, 48, 52, 56, 60);
    __mmask16 mask = 0xFFFF;
    int scale = 4;
    
    /* _mm512_mask_i32gather_epi32 has many operands */
    __m512i result = _mm512_mask_i32gather_epi32(
        _mm512_setzero_si512(),  // src
        mask,                    // mask
        vindex,                  // index
        base,                    // base addr
        scale                    // scale
    );
    
    /* Use result */
    g_result += _mm512_reduce_add_epi32(result);
}

#endif /* __x86_64__ */

#ifdef __aarch64__
#include <arm_neon.h>

/* ARM NEON version for 10-operand case */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_arm(void) {
    /* Complex NEON operation with table lookup */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t indices = vcombine_u8(
        vcreate_u8(0x0706050403020100),
        vcreate_u8(0x0F0E0D0C0B0A0908)
    );
    
    /* vqtbl3q_u8 with multiple registers */
    uint8x16x3_t table = {a, b, c};
    uint8x16_t result = vqtbl3q_u8(table, indices);
    
    /* Use result */
    g_result += vaddvq_u8(result);
}

/* ARM version for 11-operand case using inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_11_operand_arm(void) {
    uint64_t a = 1, b = 2, c = 3, d = 4, e = 5;
    uint64_t f = 6, g_val = 7, h = 8, i = 9, j = 10;
    uint64_t k = 11;
    uint64_t result;
    
    /* 11-operand inline asm for ARM */
    asm volatile (
        "/* ARM 11-operand pattern */\n\t"
        "mov %[res], %[a]\n\t"
        "add %[res], %[res], %[b]\n\t"
        "add %[res], %[res], %[c]\n\t"
        "add %[res], %[res], %[d]\n\t"
        "add %[res], %[res], %[e]\n\t"
        "add %[res], %[res], %[f]\n\t"
        "add %[res], %[res], %[g]\n\t"
        "add %[res], %[res], %[h]\n\t"
        "add %[res], %[res], %[i]\n\t"
        "add %[res], %[res], %[j]\n\t"
        "add %[res], %[res], %[k]"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g_val), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_result += result;
}

#endif /* __aarch64__ */

/* Generic fallback using complex inline assembly */
__attribute__((optimize("O3,no-inline")))
void test_10_operand_generic(void) {
    /* Generic inline assembly with 10 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long result;
    
    asm volatile (
        "/* Generic 10-operand pattern */\n\t"
        "mov %[res], %[op1]\n\t"
        "add %[res], %[res], %[op2]\n\t"
        "add %[res], %[res], %[op3]\n\t"
        "add %[res], %[res], %[op4]\n\t"
        "add %[res], %[res], %[op5]\n\t"
        "add %[res], %[res], %[op6]\n\t"
        "add %[res], %[res], %[op7]\n\t"
        "add %[res], %[res], %[op8]\n\t"
        "add %[res], %[res], %[op9]\n\t"
        "add %[res], %[res], %[op10]"
        : [res] "=r" (result)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10)
        : "cc"
    );
    
    g_result += result;
}

__attribute__((optimize("O3,no-inline")))
void test_11_operand_generic(void) {
    /* Generic inline assembly with 11 operands */
    long op1 = 1, op2 = 2, op3 = 3, op4 = 4, op5 = 5;
    long op6 = 6, op7 = 7, op8 = 8, op9 = 9, op10 = 10;
    long op11 = 11;
    long result;
    
    asm volatile (
        "/* Generic 11-operand pattern */\n\t"
        "mov %[res], %[op1]\n\t"
        "add %[res], %[res], %[op2]\n\t"
        "add %[res], %[res], %[op3]\n\t"
        "add %[res], %[res], %[op4]\n\t"
        "add %[res], %[res], %[op5]\n\t"
        "add %[res], %[res], %[op6]\n\t"
        "add %[res], %[res], %[op7]\n\t"
        "add %[res], %[res], %[op8]\n\t"
        "add %[res], %[res], %[op9]\n\t"
        "add %[res], %[res], %[op10]\n\t"
        "add %[res], %[res], %[op11]"
        : [res] "=r" (result)
        : [op1] "r" (op1), [op2] "r" (op2), [op3] "r" (op3),
          [op4] "r" (op4), [op5] "r" (op5), [op6] "r" (op6),
          [op7] "r" (op7), [op8] "r" (op8), [op9] "r" (op9),
          [op10] "r" (op10), [op11] "r" (op11)
        : "cc"
    );
    
    g_result += result;
}

/* Complex vector operation chain that might combine into multi-operand RTL */
__attribute__((optimize("O3,no-inline")))
void test_vector_chain(void) {
    /* Create a complex chain of operations */
    int data[64];
    for (int i = 0; i < 64; i++) {
        data[i] = i;
    }
    
    /* Complex computation chain */
    int sum = 0;
    for (int i = 0; i < 64; i += 8) {
        /* Multiple operations that might combine */
        int v1 = data[i] + data[i+1];
        int v2 = data[i+2] * data[i+3];
        int v3 = data[i+4] - data[i+5];
        int v4 = data[i+6] ^ data[i+7];
        
        sum += v1 + v2 + v3 + v4;
    }
    
    g_result += sum;
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call architecture-specific tests */
#ifdef __x86_64__
    printf("Running x86_64 tests...\n");
    test_10_operand_x86();
    test_10_operand_x86_v2();
    test_11_operand_x86();
    test_11_operand_x86_gather();
#elif defined(__aarch64__)
    printf("Running AArch64 tests...\n");
    test_10_operand_arm();
    test_11_operand_arm();
#else
    printf("Running generic tests...\n");
    test_10_operand_generic();
    test_11_operand_generic();
#endif
    
    /* Always run generic tests */
    test_vector_chain();
    
    printf("Result: %d\n", g_result);
    printf("Test completed.\n");
    
    return 0;
}
