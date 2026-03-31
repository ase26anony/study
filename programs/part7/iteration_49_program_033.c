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
    /* AVX-512 intrinsic that often expands to many operands */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* Complex permute operation that may require many operands */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_extract_epi64(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM NEON intrinsic with multiple operands */
    #include <arm_neon.h>
    uint8x16_t a = vdupq_n_u8(1);
    uint8x16_t b = vdupq_n_u8(2);
    uint8x16_t c = vdupq_n_u8(3);
    uint8x16_t d = vdupq_n_u8(4);
    
    /* Complex vector operations that may combine into multi-operand pattern */
    uint8x16_t r1 = vaddq_u8(a, b);
    uint8x16_t r2 = vaddq_u8(c, d);
    uint8x16_t r3 = vaddq_u8(r1, r2);
    uint8x16_t result = vmulq_u8(r3, vdupq_n_u8(2));
    
    global_counter += vgetq_lane_u8(result, 0);
    
#else
    /* Generic inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        "/* 10-operand test */\n\t"
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
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Complex AVX-512 masked operation with immediate */
    #ifdef __AVX512F__
    #include <immintrin.h>
    __m512i src = _mm512_set1_epi32(1);
    __m512i a = _mm512_set1_epi32(2);
    __m512i b = _mm512_set1_epi32(3);
    __mmask16 mask = 0xAAAA;
    
    /* This intrinsic with blend and immediate may expand to many operands */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Additional operation to create complex pattern */
    result = _mm512_add_epi32(result, src);
    result = _mm512_slli_epi32(result, 2);
    
    global_counter += _mm512_extract_epi32(result, 0);
    #endif
    
#elif defined(__aarch64__)
    /* ARM SVE-like pattern with multiple vector registers */
    #include <arm_neon.h>
    
    /* Create complex operation chain */
    int32x4_t v1 = vdupq_n_s32(1);
    int32x4_t v2 = vdupq_n_s32(2);
    int32x4_t v3 = vdupq_n_s32(3);
    int32x4_t v4 = vdupq_n_s32(4);
    int32x4_t v5 = vdupq_n_s32(5);
    
    int32x4_t r1 = vaddq_s32(v1, v2);
    int32x4_t r2 = vaddq_s32(v3, v4);
    int32x4_t r3 = vmlaq_s32(v5, r1, r2);
    int32x4_t r4 = vqdmulhq_s32(r3, vdupq_n_s32(2));
    
    global_counter += vgetq_lane_s32(r4, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        "/* 11-operand test */\n\t"
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
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j), [k] "r" (k)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test with OpenMP SIMD to encourage pattern merging */
__attribute__((noinline, optimize("O3")))
void test_vector_chain(void) {
#ifdef __x86_64__
    #ifdef __AVX512F__
    #include <immintrin.h>
    
    /* Complex vector operation chain that might merge into multi-operand pattern */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_mullo_epi32(v3, v4);
    __m512i r3 = _mm512_slli_epi32(r1, 1);
    __m512i result = _mm512_maskz_add_epi32(0xFF, r2, r3);
    
    global_counter += _mm512_extract_epi32(result, 0);
    #endif
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_vector_chain();
    }
    
    printf("Result: %d\n", global_counter);
    return 0;
}
