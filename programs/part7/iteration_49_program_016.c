/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int g_result = 0;

/* Function to test 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 intrinsic that often expands to multi-operand RTL */
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Extract and use result to prevent optimization */
    int64_t temp[8];
    _mm512_storeu_si512(temp, result);
    g_result += (int)temp[0];
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE intrinsic approach */
    #include <arm_neon.h>
    
    /* Create multiple vector registers */
    uint8x16_t v0 = vdupq_n_u8(1);
    uint8x16_t v1 = vdupq_n_u8(2);
    uint8x16_t v2 = vdupq_n_u8(3);
    uint8x16_t v3 = vdupq_n_u8(4);
    uint8x16_t v4 = vdupq_n_u8(5);
    
    /* Complex sequence that might combine into multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(v0, v1);
    uint8x16_t r2 = vaddq_u8(v2, v3);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    uint8x16_t r4 = vaddq_u8(r3, v4);
    
    /* Extract result */
    uint8_t temp[16];
    vst1q_u8(temp, r4);
    g_result += temp[0];
    
#else
    /* Generic inline assembly with 10 operands */
    int a=1, b=2, c=3, d=4, e=5, f=6, g_val=7, h=8, i=9, j=10;
    int result;
    
    asm volatile (
        /* Complex multi-operand operation */
        "add %[r], %[a], %[b]\n\t"
        "add %[r], %[r], %[c]\n\t"
        "add %[r], %[r], %[d]\n\t"
        "add %[r], %[r], %[e]\n\t"
        "add %[r], %[r], %[f]\n\t"
        "add %[r], %[r], %[g]\n\t"
        "add %[r], %[r], %[h]\n\t"
        "add %[r], %[r], %[i]\n\t"
        "add %[r], %[r], %[j]"
        : [r] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g_val), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    g_result += result;
#endif
}

/* Function to test 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Use inline assembly with exactly 11 operands */
    int64_t a=1, b=2, c=3, d=4, e=5, f=6, g_val=7, h=8, i=9, j=10, k=11;
    int64_t result1, result2;
    
    /* Complex inline assembly with 11 input/output operands */
    asm volatile (
        /* Multi-step computation using all 11 values */
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[r1], %[b]\n\t"
        "add %[r1], %[r1], %[c]\n\t"
        "mov %[r2], %[d]\n\t"
        "add %[r2], %[r2], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "imul %[r1], %[r1], %[g]\n\t"
        "imul %[r2], %[r2], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r1], %[r1], %[k]\n\t"
        "add %[r1], %[r1], %[r2]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g_val), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_result += (int)(result1 + result2);
    
#elif defined(__aarch64__)
    /* ARM-specific multi-operand approach */
    #include <arm_neon.h>
    
    /* Create many vector operands */
    int32x4_t v0 = vdupq_n_s32(1);
    int32x4_t v1 = vdupq_n_s32(2);
    int32x4_t v2 = vdupq_n_s32(3);
    int32x4_t v3 = vdupq_n_s32(4);
    int32x4_t v4 = vdupq_n_s32(5);
    int32x4_t v5 = vdupq_n_s32(6);
    int32x4_t v6 = vdupq_n_s32(7);
    int32x4_t v7 = vdupq_n_s32(8);
    
    /* Complex vector operation chain */
    int32x4_t t0 = vaddq_s32(v0, v1);
    int32x4_t t1 = vaddq_s32(v2, v3);
    int32x4_t t2 = vaddq_s32(v4, v5);
    int32x4_t t3 = vaddq_s32(v6, v7);
    
    int32x4_t r0 = vaddq_s32(t0, t1);
    int32x4_t r1 = vaddq_s32(t2, t3);
    int32x4_t final = vaddq_s32(r0, r1);
    
    /* Extract and use result */
    int32_t temp[4];
    vst1q_s32(temp, final);
    g_result += temp[0];
    
#else
    /* Generic 11-operand inline assembly */
    int a=1, b=2, c=3, d=4, e=5, f=6, g_val=7, h=8, i=9, j=10, k=11;
    int r1, r2;
    
    asm volatile (
        "mov %[r1], %[a]\n\t"
        "add %[r1], %[r1], %[b]\n\t"
        "mov %[r2], %[c]\n\t"
        "add %[r2], %[r2], %[d]\n\t"
        "add %[r1], %[r1], %[e]\n\t"
        "add %[r2], %[r2], %[f]\n\t"
        "imul %[r1], %[r1], %[g]\n\t"
        "imul %[r2], %[r2], %[h]\n\t"
        "add %[r1], %[r1], %[i]\n\t"
        "add %[r2], %[r2], %[j]\n\t"
        "add %[r1], %[r1], %[k]\n\t"
        "add %[r1], %[r1], %[r2]"
        : [r1] "=&r" (r1), [r2] "=&r" (r2)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g_val), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_result += r1;
#endif
}

/* Additional test using vector shuffle patterns */
__attribute__((noinline, optimize("O3")))
void test_vector_shuffle(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Complex AVX-512 operation that may generate multi-operand RTL */
    __m512i a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i b = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i idx = _mm512_set_epi32(1,3,5,7,9,11,13,15,0,2,4,6,8,10,12,14);
    __mmask16 mask = 0xAAAA;
    
    /* This intrinsic often expands to complex RTL with many operands */
    __m512i result = _mm512_mask_permutex2var_epi32(a, mask, idx, b);
    
    int temp[16];
    _mm512_storeu_si512(temp, result);
    g_result += temp[0];
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    test_vector_shuffle();
    
    printf("Result: %d\n", g_result);
    printf("Test completed.\n");
    
    return g_result == 0 ? 0 : 1;
}
