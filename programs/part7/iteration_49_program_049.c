/* Test program to cover 10 and 11 operand cases in optabs.cc */
#include <stdio.h>
#include <stdint.h>

/* Prevent dead code elimination */
volatile int g_result = 0;

/* Function to trigger 10-operand case */
__attribute__((noinline, optimize("O3")))
void test_10_operand(void) {
#ifdef __x86_64__
    /* AVX-512 complex permute with mask - typically expands to many operands */
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* _mm512_mask_permutex2var_epi64 typically expands to complex RTL with many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent elimination */
    g_result += _mm512_extract_epi64(result, 0);
    
#elif defined(__aarch64__)
    /* ARM NEON complex operations */
    #include <arm_neon.h>
    
    /* Use multiple vector operations that might combine into complex pattern */
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex sequence that might generate multi-operand RTL */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    
    /* Multiple tbl operations with many operands */
    uint8x16_t idx1 = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8x16_t idx2 = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    
    uint8x16_t t1 = vqtbl1q_u8(r3, idx1);
    uint8x16_t t2 = vqtbl1q_u8(r3, idx2);
    uint8x16_t final = vaddq_u8(t1, t2);
    
    g_result += vgetq_lane_u8(final, 0);
    
#else
    /* Generic inline assembly with exactly 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    /* 10-operand asm statement */
    asm volatile (
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
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    g_result += result;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Complex AVX-512 masked operation with multiple sources and mask */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi32(1);
    __m512i src2 = _mm512_set1_epi32(2);
    __m512i src3 = _mm512_set1_epi32(3);
    __m512i idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0x5555;
    
    /* Complex sequence that might generate 11-operand RTL */
    __m512i temp1 = _mm512_mask_add_epi32(src1, mask1, src2, src3);
    __m512i temp2 = _mm512_mask_sub_epi32(src2, mask2, src3, src1);
    __m512i perm = _mm512_permutexvar_epi32(idx, temp1);
    __m512i blend = _mm512_mask_blend_epi32(mask1, temp2, perm);
    
    g_result += _mm512_extract_epi32(blend, 0);
    
#elif defined(__aarch64__)
    /* ARM SVE-like pattern with multiple operands */
    #include <arm_neon.h>
    
    /* Create complex operation chain */
    float32x4_t v1 = vdupq_n_f32(1.0f);
    float32x4_t v2 = vdupq_n_f32(2.0f);
    float32x4_t v3 = vdupq_n_f32(3.0f);
    float32x4_t v4 = vdupq_n_f32(4.0f);
    float32x4_t v5 = vdupq_n_f32(5.0f);
    
    float32x4_t r1 = vaddq_f32(v1, v2);
    float32x4_t r2 = vaddq_f32(v3, v4);
    float32x4_t r3 = vmlaq_f32(r1, r2, v5);  /* r1 + r2 * v5 */
    float32x4_t r4 = vfmaq_f32(v1, v2, v3);   /* v1 + v2 * v3 */
    
    /* Complex permute-like operation */
    const int idx_arr[4] = {3, 2, 1, 0};
    uint32x4_t idx = vld1q_u32((const uint32_t*)idx_arr);
    float32x4_t shuffled = vqtbl1q_f32(r3, idx);
    
    float32x4_t final = vaddq_f32(shuffled, r4);
    
    g_result += (int)vgetq_lane_f32(final, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    /* 11-operand asm statement */
    asm volatile (
        "mov %[r], %[a]\n\t"
        "add %[r], %[r], %[b]\n\t"
        "add %[r], %[r], %[c]\n\t"
        "add %[r], %[r], %[d]\n\t"
        "add %[r], %[r], %[e]\n\t"
        "add %[r], %[r], %[f]\n\t"
        "add %[r], %[r], %[g]\n\t"
        "add %[r], %[r], %[h]\n\t"
        "add %[r], %[r], %[i]\n\t"
        "add %[r], %[r], %[j]\n\t"
        "add %[r], %[r], %[k]"
        : [r] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    g_result += result;
#endif
}

/* Additional test with OpenMP SIMD pragmas to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    
    /* Complex vector operation chain that might merge into multi-operand RTL */
    #pragma omp simd
    for (int i = 0; i < 16; i++) {
        __m512i v1 = _mm512_set1_epi32(i);
        __m512i v2 = _mm512_set1_epi32(i * 2);
        __m512i v3 = _mm512_set1_epi32(i * 3);
        
        /* Complex masked operation */
        __mmask16 mask = (i % 2) ? 0xFFFF : 0x0000;
        __m512i result = _mm512_mask_add_epi32(v1, mask, v2, v3);
        
        /* Use volatile store to prevent optimization */
        volatile __m512i* ptr = (volatile __m512i*)&g_result;
    }
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call both test functions */
    test_10_operand();
    test_11_operand();
    
    /* Additional test for pattern merging */
    test_vector_chain();
    
    printf("Result: %d\n", g_result);
    printf("Test completed.\n");
    
    return 0;
}
