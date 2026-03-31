/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent dead code elimination */
volatile int global_counter = 0;

/* Function to trigger 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with mask - can generate 10 operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
    /* Alternative: Complex inline assembly with 10 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    long result_asm;
    
    asm volatile (
        /* Complex multi-operand operation simulating 10 operands */
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
        : [res] "=r" (result_asm)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    global_counter += result_asm;
    
#elif defined(__aarch64__)
    /* ARM NEON/SVE complex operations */
    #ifdef __ARM_NEON
    #include <arm_neon.h>
    
    /* Complex vector operation chain that might generate multi-operand RTL */
    uint64x2_t v1 = vdupq_n_u64(1);
    uint64x2_t v2 = vdupq_n_u64(2);
    uint64x2_t v3 = vdupq_n_u64(3);
    uint64x2_t v4 = vdupq_n_u64(4);
    
    /* Multiple operations chained together */
    uint64x2_t r1 = vaddq_u64(v1, v2);
    uint64x2_t r2 = vaddq_u64(v3, v4);
    uint64x2_t r3 = vaddq_u64(r1, r2);
    
    global_counter += vgetq_lane_u64(r3, 0);
    #endif
    
    /* Generic multi-operand inline assembly for ARM */
    register long x0 asm("x0") = 1;
    register long x1 asm("x1") = 2;
    register long x2 asm("x2") = 3;
    register long x3 asm("x3") = 4;
    register long x4 asm("x4") = 5;
    register long x5 asm("x5") = 6;
    register long x6 asm("x6") = 7;
    register long x7 asm("x7") = 8;
    register long x8 asm("x8") = 9;
    register long x9 asm("x9") = 10;
    
    asm volatile (
        "add %[x0], %[x0], %[x1]\n\t"
        "add %[x0], %[x0], %[x2]\n\t"
        "add %[x0], %[x0], %[x3]\n\t"
        "add %[x0], %[x0], %[x4]\n\t"
        "add %[x0], %[x0], %[x5]\n\t"
        "add %[x0], %[x0], %[x6]\n\t"
        "add %[x0], %[x0], %[x7]\n\t"
        "add %[x0], %[x0], %[x8]\n\t"
        "add %[x0], %[x0], %[x9]"
        : [x0] "+r" (x0)
        : [x1] "r" (x1), [x2] "r" (x2), [x3] "r" (x3),
          [x4] "r" (x4), [x5] "r" (x5), [x6] "r" (x6),
          [x7] "r" (x7), [x8] "r" (x8), [x9] "r" (x9)
        : "cc"
    );
    
    global_counter += x0;
#else
    /* Generic fallback with complex expression */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10;
    int result = a + b + c + d + e + f + g + h + i + j;
    global_counter += result;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Inline assembly with exactly 11 operands */
    long a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    long result_asm;
    
    asm volatile (
        /* Operation using all 11 operands */
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
        "add %[k], %[res]\n\t"
        : [res] "=r" (result_asm)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result_asm;
    
    /* AVX-512 masked gather with complex addressing - can generate 11 operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    double src[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    long long idx[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    __m512d vresult;
    __mmask8 mask = 0xFF;
    
    /* Complex gather operation */
    vresult = _mm512_mask_i64gather_pd(_mm512_setzero_pd(), mask,
                                      _mm512_loadu_si512(idx),
                                      src, 8);
    
    global_counter += (int)_mm512_cvtsd_f64(_mm512_castpd512_pd128(vresult));
    #endif
    
#elif defined(__aarch64__)
    /* ARM inline assembly with 11 operands */
    register long x0 asm("x0") = 1;
    register long x1 asm("x1") = 2;
    register long x2 asm("x2") = 3;
    register long x3 asm("x3") = 4;
    register long x4 asm("x4") = 5;
    register long x5 asm("x5") = 6;
    register long x6 asm("x6") = 7;
    register long x7 asm("x7") = 8;
    register long x8 asm("x8") = 9;
    register long x9 asm("x9") = 10;
    register long x10 asm("x10") = 11;
    
    asm volatile (
        "add %[x0], %[x0], %[x1]\n\t"
        "add %[x0], %[x0], %[x2]\n\t"
        "add %[x0], %[x0], %[x3]\n\t"
        "add %[x0], %[x0], %[x4]\n\t"
        "add %[x0], %[x0], %[x5]\n\t"
        "add %[x0], %[x0], %[x6]\n\t"
        "add %[x0], %[x0], %[x7]\n\t"
        "add %[x0], %[x0], %[x8]\n\t"
        "add %[x0], %[x0], %[x9]\n\t"
        "add %[x0], %[x0], %[x10]"
        : [x0] "+r" (x0)
        : [x1] "r" (x1), [x2] "r" (x2), [x3] "r" (x3),
          [x4] "r" (x4), [x5] "r" (x5), [x6] "r" (x6),
          [x7] "r" (x7), [x8] "r" (x8), [x9] "r" (x9),
          [x10] "r" (x10)
        : "cc"
    );
    
    global_counter += x0;
    
    #ifdef __ARM_NEON
    /* Complex NEON operation chain */
    #include <arm_neon.h>
    uint32x4_t v1 = vdupq_n_u32(1);
    uint32x4_t v2 = vdupq_n_u32(2);
    uint32x4_t v3 = vdupq_n_u32(3);
    uint32x4_t v4 = vdupq_n_u32(4);
    uint32x4_t v5 = vdupq_n_u32(5);
    
    uint32x4_t r1 = vaddq_u32(v1, v2);
    uint32x4_t r2 = vaddq_u32(v3, v4);
    uint32x4_t r3 = vaddq_u32(r1, r2);
    uint32x4_t r4 = vaddq_u32(r3, v5);
    
    global_counter += vgetq_lane_u32(r4, 0);
    #endif
#else
    /* Generic fallback */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result = a + b + c + d + e + f + g + h + i + j + k;
    global_counter += result;
#endif
}

/* Complex vector operation that might generate multi-operand RTL patterns */
__attribute__((noinline, optimize("O3")))
void complex_vector_operations(void) {
#ifdef __AVX512F__
    #include <immintrin.h>
    
    /* This complex sequence might generate patterns with many operands */
    __m512i a = _mm512_set1_epi32(1);
    __m512i b = _mm512_set1_epi32(2);
    __m512i c = _mm512_set1_epi32(3);
    __m512i d = _mm512_set1_epi32(4);
    
    /* Chain of operations */
    __m512i r1 = _mm512_add_epi32(a, b);
    __m512i r2 = _mm512_add_epi32(c, d);
    __m512i r3 = _mm512_add_epi32(r1, r2);
    __m512i r4 = _mm512_slli_epi32(r3, 2);
    
    global_counter += _mm512_extract_epi32(r4, 0);
#endif
}

int main(void) {
    printf("Testing multi-operand RTL pattern generation...\n");
    
    /* Call all test functions */
    test_10_operand();
    test_11_operand();
    complex_vector_operations();
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter > 0 ? 0 : 1;
}
