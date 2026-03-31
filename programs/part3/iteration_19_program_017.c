/* Test program to trigger 10/11 operand RTL expansion in optabs.cc */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization from eliminating our test cases */
#define NOINLINE __attribute__((noinline, noipa))

/* Vector types for potential multi-operand expansions */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex arithmetic that might expand to many operands */
NOINLINE uint64_t multi_operand_arithmetic(uint64_t a, uint64_t b, uint64_t c,
                                           uint64_t d, uint64_t e, uint64_t f,
                                           uint64_t g, uint64_t h, uint64_t i,
                                           uint64_t j) {
    /* Complex expression that might require many temporary operands */
    uint64_t t1 = (a * b) >> 32;
    uint64_t t2 = (c * d) >> 32;
    uint64_t t3 = (e * f) >> 32;
    uint64_t t4 = (g * h) >> 32;
    uint64_t t5 = (i * j) >> 32;
    
    /* Multi-precision calculation */
    uint64_t result = t1 + t2;
    result = result * t3;
    result = result + t4;
    result = result - t5;
    result = result * (a + b + c + d + e);
    
    return result;
}

/* Vector operations that might expand to many operands */
NOINLINE v4si vector_operations(v4si a, v4si b, v4si c, v4si d,
                                v4si e, v4si f, v4si g) {
    /* Complex vector expression */
    v4si t1 = a * b + c;
    v4si t2 = d * e - f;
    v4si t3 = g * a + b;
    v4si t4 = c * d - e;
    v4si t5 = f * g + a;
    
    v4si result = t1 * t2 + t3 - t4 * t5;
    result = result >> 2;
    result = result & (v4si){0x0F, 0x0F, 0x0F, 0x0F};
    
    return result;
}

#ifdef __x86_64__
#include <x86intrin.h>
#include <immintrin.h>

NOINLINE __m128i x86_multi_operand_intrinsic(__m128i a, __m128i b, __m128i c,
                                            __m128i d, __m128i e, __m128i f,
                                            __m128i g, __m128i h, __m128i i) {
    /* Complex chain of SSE/AVX operations */
    __m128i t1 = _mm_add_epi32(a, b);
    __m128i t2 = _mm_sub_epi32(c, d);
    __m128i t3 = _mm_mullo_epi32(e, f);
    __m128i t4 = _mm_and_si128(g, h);
    __m128i t5 = _mm_or_si128(i, a);
    
    __m128i result = _mm_add_epi32(t1, t2);
    result = _mm_sub_epi32(result, t3);
    result = _mm_and_si128(result, t4);
    result = _mm_or_si128(result, t5);
    
    /* Additional operations to increase operand count */
    result = _mm_slli_epi32(result, 2);
    result = _mm_srli_epi32(result, 1);
    
    return result;
}

/* Extended inline assembly with many operands */
NOINLINE uint64_t x86_extended_asm(uint64_t a, uint64_t b, uint64_t c,
                                   uint64_t d, uint64_t e, uint64_t f,
                                   uint64_t g, uint64_t h, uint64_t i,
                                   uint64_t j) {
    uint64_t result1, result2, result3;
    
    /* Complex inline assembly that might expand to many operands */
    asm volatile (
        "mov %[a], %[r1]\n\t"
        "add %[b], %[r1]\n\t"
        "imul %[c], %[r1]\n\t"
        "mov %[d], %[r2]\n\t"
        "sub %[e], %[r2]\n\t"
        "and %[f], %[r2]\n\t"
        "mov %[g], %[r3]\n\t"
        "or %[h], %[r3]\n\t"
        "xor %[i], %[r3]\n\t"
        "add %[r1], %[r2]\n\t"
        "sub %[r3], %[r2]\n\t"
        "mov %[r2], %[r1]\n\t"
        "imul %[j], %[r1]"
        : [r1] "=&r" (result1), [r2] "=&r" (result2), [r3] "=&r" (result3)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d),
          [e] "r" (e), [f] "r" (f), [g] "r" (g), [h] "r" (h),
          [i] "r" (i), [j] "r" (j)
        : "cc"
    );
    
    return result1 + result2 + result3;
}
#endif

#ifdef __aarch64__
#include <arm_neon.h>

NOINLINE uint64x2_t arm_multi_operand_intrinsic(uint64x2_t a, uint64x2_t b,
                                                uint64x2_t c, uint64x2_t d,
                                                uint64x2_t e, uint64x2_t f,
                                                uint64x2_t g, uint64x2_t h) {
    /* Complex NEON operations chain */
    uint64x2_t t1 = vaddq_u64(a, b);
    uint64x2_t t2 = vsubq_u64(c, d);
    uint64x2_t t3 = vmulq_u64(e, f);
    uint64x2_t t4 = vandq_u64(g, h);
    uint64x2_t t5 = vorrq_u64(a, b);
    
    uint64x2_t result = vaddq_u64(t1, t2);
    result = vmulq_u64(result, t3);
    result = vandq_u64(result, t4);
    result = vorrq_u64(result, t5);
    
    /* Additional operations */
    result = vshlq_n_u64(result, 2);
    result = vshrq_n_u64(result, 1);
    
    return result;
}
#endif

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t result = 0;
    
    /* Initialize test values */
    uint64_t vals[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test different code paths based on arguments */
    if (argc > 1) {
        /* Path 1: Complex arithmetic */
        for (int i = 0; i < 10; i++) {
            result += multi_operand_arithmetic(
                vals[0] + i, vals[1] + i, vals[2] + i,
                vals[3] + i, vals[4] + i, vals[5] + i,
                vals[6] + i, vals[7] + i, vals[8] + i,
                vals[9] + i
            );
        }
    } else {
        /* Path 2: Vector operations */
        v4si vec_a = {1, 2, 3, 4};
        v4si vec_b = {5, 6, 7, 8};
        v4si vec_c = {9, 10, 11, 12};
        v4si vec_d = {13, 14, 15, 16};
        v4si vec_e = {17, 18, 19, 20};
        v4si vec_f = {21, 22, 23, 24};
        v4si vec_g = {25, 26, 27, 28};
        
        v4si vec_result = vector_operations(vec_a, vec_b, vec_c, vec_d,
                                           vec_e, vec_f, vec_g);
        
        for (int i = 0; i < 4; i++) {
            result += vec_result[i];
        }
    }
    
#ifdef __x86_64__
    /* Test x86-specific paths */
    if (argc > 2) {
        __m128i xmm0 = _mm_set_epi32(1, 2, 3, 4);
        __m128i xmm1 = _mm_set_epi32(5, 6, 7, 8);
        __m128i xmm2 = _mm_set_epi32(9, 10, 11, 12);
        __m128i xmm3 = _mm_set_epi32(13, 14, 15, 16);
        __m128i xmm4 = _mm_set_epi32(17, 18, 19, 20);
        __m128i xmm5 = _mm_set_epi32(21, 22, 23, 24);
        __m128i xmm6 = _mm_set_epi32(25, 26, 27, 28);
        __m128i xmm7 = _mm_set_epi32(29, 30, 31, 32);
        __m128i xmm8 = _mm_set_epi32(33, 34, 35, 36);
        
        __m128i xmm_result = x86_multi_operand_intrinsic(xmm0, xmm1, xmm2, xmm3,
                                                        xmm4, xmm5, xmm6, xmm7,
                                                        xmm8);
        
        uint32_t temp[4];
        _mm_storeu_si128((__m128i*)temp, xmm_result);
        result += temp[0] + temp[1] + temp[2] + temp[3];
        
        /* Test extended inline assembly */
        result += x86_extended_asm(vals[0], vals[1], vals[2], vals[3],
                                  vals[4], vals[5], vals[6], vals[7],
                                  vals[8], vals[9]);
    }
#endif
    
#ifdef __aarch64__
    /* Test ARM-specific paths */
    if (argc > 3) {
        uint64x2_t neon_a = {1, 2};
        uint64x2_t neon_b = {3, 4};
        uint64x2_t neon_c = {5, 6};
        uint64x2_t neon_d = {7, 8};
        uint64x2_t neon_e = {9, 10};
        uint64x2_t neon_f = {11, 12};
        uint64x2_t neon_g = {13, 14};
        uint64x2_t neon_h = {15, 16};
        
        uint64x2_t neon_result = arm_multi_operand_intrinsic(neon_a, neon_b,
                                                            neon_c, neon_d,
                                                            neon_e, neon_f,
                                                            neon_g, neon_h);
        
        uint64_t temp[2];
        vst1q_u64(temp, neon_result);
        result += temp[0] + temp[1];
    }
#endif
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lu\n", (unsigned long)result);
    
    /* Additional loop to increase coverage with varying data */
    for (int i = 0; i < argc; i++) {
        uint64_t temp = multi_operand_arithmetic(
            i, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9
        );
        result ^= temp;
    }
    
    printf("Final result: %lu\n", (unsigned long)result);
    
    return 0;
}
