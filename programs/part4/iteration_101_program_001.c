/* Test program to cover 10/11 operand expansion in optabs.cc */
#include <stdio.h>
#include <string.h>
#include <x86intrin.h>

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* AVX types if available */
#ifdef __AVX__
typedef int v8si __attribute__((vector_size(32)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v4df __attribute__((vector_size(32)));
#endif

/* AVX-512 types if available */
#ifdef __AVX512F__
typedef int v16si __attribute__((vector_size(64)));
typedef float v16sf __attribute__((vector_size(64)));
typedef double v8df __attribute__((vector_size(64)));
#endif

/* Force no inlining to preserve operations */
__attribute__((noinline, noipa))
v4si test_10_operand_expansion(v4si a, v4si b, v4si c, v4si d, 
                               v4si mask1, v4si mask2, v4si mask3) {
    volatile v4si v1, v2, v3, v4, v5;
    
    /* Complex shuffle chain - may require many operands during expansion */
    v4si shuffle1 = __builtin_shuffle(a, b, mask1);
    asm volatile("" ::: "memory");
    
    v4si shuffle2 = __builtin_shuffle(c, d, mask2);
    asm volatile("" ::: "memory");
    
    /* Vector conditional with comparison - generates VEC_COND_EXPR */
    v4si cmp = (shuffle1 > shuffle2);
    v4si blend = cmp ? (shuffle1 + shuffle2) : (shuffle1 - shuffle2);
    asm volatile("" ::: "memory");
    
    /* Another shuffle with the result */
    v4si shuffle3 = __builtin_shuffle(blend, a, mask3);
    asm volatile("" ::: "memory");
    
    /* Complex arithmetic expression that may expand to multiple operations */
    v4si result = shuffle1 * shuffle2 + shuffle3 / (blend + 1);
    asm volatile("" ::: "memory");
    
    /* Store to volatile to force all operations */
    v1 = shuffle1;
    v2 = shuffle2;
    v3 = blend;
    v4 = shuffle3;
    v5 = result;
    
    return v1 + v2 + v3 + v4 + v5;
}

#ifdef __AVX__
__attribute__((noinline, noipa))
v8si test_11_operand_expansion(v8si a, v8si b, v8si c, v8si d,
                               v8si e, v8si mask1, v8si mask2) {
    volatile v8si v1, v2, v3, v4, v5, v6;
    
    /* Even more complex operation chain for AVX */
    v8si shuffle1 = __builtin_shufflevector(a, b, 
        0, 8, 1, 9, 2, 10, 3, 11);
    asm volatile("" ::: "memory");
    
    v8si shuffle2 = __builtin_shufflevector(c, d,
        4, 12, 5, 13, 6, 14, 7, 15);
    asm volatile("" ::: "memory");
    
    /* Multiple vector comparisons */
    v8si cmp1 = (shuffle1 > shuffle2);
    v8si cmp2 = (shuffle1 < e);
    asm volatile("" ::: "memory");
    
    /* Nested conditional */
    v8si blend1 = cmp1 ? (shuffle1 * 2) : (shuffle2 / 2);
    v8si blend2 = cmp2 ? (blend1 + e) : (blend1 - e);
    asm volatile("" ::: "memory");
    
    /* Final shuffle with mask */
    v8si shuffle3 = __builtin_shuffle(blend2, a, mask1);
    v8si shuffle4 = __builtin_shuffle(shuffle3, b, mask2);
    asm volatile("" ::: "memory");
    
    /* Complex arithmetic with many operands */
    v8si result = a * b + c * d - e + shuffle1 * shuffle2 / (blend1 + blend2);
    asm volatile("" ::: "memory");
    
    /* Store all to volatile */
    v1 = shuffle1;
    v2 = shuffle2;
    v3 = blend1;
    v4 = blend2;
    v5 = shuffle4;
    v6 = result;
    
    return v1 + v2 + v3 + v4 + v5 + v6;
}
#endif

/* Use intrinsics for x86-specific many-operand builtins */
#ifdef __SSE4_1__
__attribute__((noinline, noipa))
__m128d test_intrinsic_10_operand(__m128d a, __m128d b, __m128d c, 
                                  __m128d d, __m128i mask) {
    volatile __m128d v1, v2, v3, v4;
    
    /* Chain of intrinsics that may require many operands */
    __m128d blendv = _mm_blendv_pd(a, b, _mm_castsi128_pd(mask));
    asm volatile("" ::: "memory");
    
    __m128d round1 = _mm_round_pd(blendv, _MM_FROUND_TO_NEAREST_INT);
    __m128d round2 = _mm_round_pd(c, _MM_FROUND_TO_NEG_INF);
    asm volatile("" ::: "memory");
    
    __m128d dp = _mm_dp_pd(round1, round2, 0x33);
    asm volatile("" ::: "memory");
    
    __m128d result = _mm_add_pd(_mm_mul_pd(a, b), 
                               _mm_sub_pd(_mm_div_pd(c, d), dp));
    asm volatile("" ::: "memory");
    
    v1 = blendv;
    v2 = round1;
    v3 = round2;
    v4 = result;
    
    return _mm_add_pd(_mm_add_pd(v1, v2), _mm_add_pd(v3, v4));
}
#endif

int main() {
    /* Initialize vectors with pattern values */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    v4si d = {13, 14, 15, 16};
    v4si mask1 = {0, 2, 1, 3};
    v4si mask2 = {3, 1, 2, 0};
    v4si mask3 = {1, 3, 0, 2};
    
    printf("Testing 10-operand expansion...\n");
    v4si result1 = test_10_operand_expansion(a, b, c, d, mask1, mask2, mask3);
    
    /* Compute checksum to prevent elimination */
    int sum1 = 0;
    for (int i = 0; i < 4; i++) {
        sum1 += result1[i];
    }
    printf("Result checksum (10-op): %d\n", sum1);
    
#ifdef __AVX__
    /* Initialize AVX vectors */
    v8si avx_a = {1, 2, 3, 4, 5, 6, 7, 8};
    v8si avx_b = {9, 10, 11, 12, 13, 14, 15, 16};
    v8si avx_c = {17, 18, 19, 20, 21, 22, 23, 24};
    v8si avx_d = {25, 26, 27, 28, 29, 30, 31, 32};
    v8si avx_e = {33, 34, 35, 36, 37, 38, 39, 40};
    v8si avx_mask1 = {0, 2, 4, 6, 1, 3, 5, 7};
    v8si avx_mask2 = {7, 5, 3, 1, 6, 4, 2, 0};
    
    printf("\nTesting 11-operand expansion...\n");
    v8si result2 = test_11_operand_expansion(avx_a, avx_b, avx_c, avx_d, 
                                            avx_e, avx_mask1, avx_mask2);
    
    int sum2 = 0;
    for (int i = 0; i < 8; i++) {
        sum2 += result2[i];
    }
    printf("Result checksum (11-op): %d\n", sum2);
#endif
    
#ifdef __SSE4_1__
    /* Test with x86 intrinsics */
    __m128d da = _mm_set_pd(1.0, 2.0);
    __m128d db = _mm_set_pd(3.0, 4.0);
    __m128d dc = _mm_set_pd(5.0, 6.0);
    __m128d dd = _mm_set_pd(7.0, 8.0);
    __m128i dmask = _mm_set_epi64x(0xFFFFFFFFFFFFFFFF, 0);
    
    printf("\nTesting intrinsic 10-operand expansion...\n");
    __m128d result3 = test_intrinsic_10_operand(da, db, dc, dd, dmask);
    
    double sum3 = 0;
    double res3[2];
    _mm_storeu_pd(res3, result3);
    sum3 = res3[0] + res3[1];
    printf("Result checksum (intrinsic): %f\n", sum3);
#endif
    
    /* Final check to ensure all code executed */
    int final_check = sum1;
#ifdef __AVX__
    final_check += sum2;
#endif
#ifdef __SSE4_1__
    final_check += (int)sum3;
#endif
    
    return (final_check > 0) ? 0 : 1;
}
