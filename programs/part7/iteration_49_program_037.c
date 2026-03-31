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
    /* AVX-512 complex permute with multiple operands */
    #include <immintrin.h>
    
    __m512i src1 = _mm512_set1_epi64(1);
    __m512i src2 = _mm512_set1_epi64(2);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    
    /* This intrinsic typically expands to multiple operands:
       dest, mask, idx, src1, src2 + various immediates */
    __m512i result = _mm512_mask_permutex2var_epi64(src1, mask, idx, src2);
    
    /* Use result to prevent optimization */
    global_counter += _mm512_reduce_add_epi64(result);
    
#elif defined(__aarch64__)
    /* ARM SVE complex operations with multiple operands */
    #include <arm_sve.h>
    
    svint64_t vec1 = svdup_s64(1);
    svint64_t vec2 = svdup_s64(2);
    svbool_t pg = svptrue_b64();
    
    /* Complex SVE operation that may expand to many operands */
    svint64_t result = svadd_s64_z(pg, vec1, vec2);
    
    /* Extract and use result */
    int64_t temp[2];
    svst1_s64(pg, temp, result);
    global_counter += temp[0];
    
#else
    /* Generic inline assembly with 10 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result;
    
    asm volatile (
        "/* 10-operand test */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Function to trigger 11-operand case */
__attribute__((noinline, optimize("O3")))
void test_11_operand(void) {
#ifdef __x86_64__
    /* Complex AVX-512 masked gather with multiple operands */
    #include <immintrin.h>
    
    __m512i src = _mm512_set1_epi64(1);
    __m512i idx = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __mmask8 mask = 0xFF;
    long long base[64] = {0};
    
    /* This should generate many operands: dest, mask, idx, base, scale, src */
    __m512i result = _mm512_mask_i64gather_epi64(src, mask, idx, base, 8);
    
    global_counter += _mm512_reduce_add_epi64(result);
    
#elif defined(__aarch64__)
    /* ARM NEON complex lane operations */
    #include <arm_neon.h>
    
    int64x2_t a = vdupq_n_s64(1);
    int64x2_t b = vdupq_n_s64(2);
    int64x2_t c = vdupq_n_s64(3);
    int64x2_t d = vdupq_n_s64(4);
    
    /* Complex multi-lane operation */
    int64x2_t r1 = vaddq_s64(a, b);
    int64x2_t r2 = vaddq_s64(c, d);
    int64x2_t result = vaddq_s64(r1, r2);
    
    global_counter += vgetq_lane_s64(result, 0);
    
#else
    /* Generic inline assembly with exactly 11 operands */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10, k = 11;
    int result;
    
    asm volatile (
        "/* 11-operand test */\n\t"
        "add %0, %1, %2\n\t"
        "add %0, %0, %3\n\t"
        "add %0, %0, %4\n\t"
        "add %0, %0, %5\n\t"
        "add %0, %0, %6\n\t"
        "add %0, %0, %7\n\t"
        "add %0, %0, %8\n\t"
        "add %0, %0, %9\n\t"
        "add %0, %0, %10"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
          "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
        : "cc"
    );
    
    global_counter += result;
#endif
}

/* Additional test with complex vector operations that might merge */
__attribute__((noinline, optimize("O3")))
void test_complex_vector_chain(void) {
#ifdef __x86_64__
    #include <immintrin.h>
    
    /* Create a complex chain that might be optimized into a multi-operand pattern */
    __m512i v1 = _mm512_set1_epi32(global_counter);
    __m512i v2 = _mm512_set1_epi32(2);
    __m512i v3 = _mm512_set1_epi32(3);
    __m512i v4 = _mm512_set1_epi32(4);
    
    /* Chain of operations that might be combined */
    __m512i r1 = _mm512_add_epi32(v1, v2);
    __m512i r2 = _mm512_mullo_epi32(r1, v3);
    __m512i r3 = _mm512_slli_epi32(r2, 2);
    __m512i result = _mm512_sub_epi32(r3, v4);
    
    /* Use volatile store to ensure operations aren't optimized away */
    volatile __m512i* dummy = &result;
    (void)dummy;
    
#elif defined(__aarch64__)
    #include <arm_neon.h>
    
    int32x4_t v1 = vdupq_n_s32(global_counter);
    int32x4_t v2 = vdupq_n_s32(2);
    int32x4_t v3 = vdupq_n_s32(3);
    int32x4_t v4 = vdupq_n_s32(4);
    
    int32x4_t r1 = vaddq_s32(v1, v2);
    int32x4_t r2 = vmulq_s32(r1, v3);
    int32x4_t r3 = vshlq_n_s32(r2, 2);
    int32x4_t result = vsubq_s32(r3, v4);
    
    volatile int32x4_t* dummy = &result;
    (void)dummy;
#endif
}

int main(void) {
    printf("Testing multi-operand RTL patterns...\n");
    
    /* Call all test functions multiple times */
    for (int i = 0; i < 10; i++) {
        test_10_operand();
        test_11_operand();
        test_complex_vector_chain();
    }
    
    printf("Global counter: %d\n", global_counter);
    printf("Test completed.\n");
    
    return global_counter == 0 ? 0 : 1;
}
