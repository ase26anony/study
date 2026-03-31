#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef __AVX__
#include <avxintrin.h>
#endif

/* Helper function to check if AVX is available at runtime */
static int avx_supported(void) {
#ifdef __AVX__
    return __builtin_cpu_supports("avx");
#else
    return 0;
#endif
}

/* Test all SSE comparison condition codes */
void test_sse_conditions(void) {
    printf("Testing SSE condition codes...\n");
    
    /* Create test vectors with various values including NaN */
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 d = _mm_setr_ps(0.0f, 0.0f, INFINITY, INFINITY);
    
    /* Variables to store comparison results */
    __m128 cmp_results[8];
    int masks[8];
    
    /* Test all condition codes from the uncovered block */
    cmp_results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);    /* UNORDERED */
    cmp_results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);      /* ORDERED */
    cmp_results[2] = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);    /* UNEQ */
    cmp_results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);     /* UNGE */
    cmp_results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);     /* UNGT */
    cmp_results[5] = _mm_cmp_ps(a, b, _CMP_ULE_UQ);     /* UNLE */
    cmp_results[6] = _mm_cmp_ps(a, b, _CMP_ULT_UQ);     /* UNLT */
    cmp_results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);     /* LTGT */
    
    /* Extract masks to prevent dead code elimination */
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm_movemask_ps(cmp_results[i]);
    }
    
    /* Use results in control flow */
    int final_result = 0;
    for (int i = 0; i < 8; i++) {
        if (masks[i] != 0) {
            final_result |= (1 << i);
        }
    }
    
    /* Test scalar comparisons as well */
    __m128 scalar_cmp = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    int scalar_mask = _mm_movemask_ps(scalar_cmp);
    final_result |= (scalar_mask << 8);
    
    /* Test double precision comparisons */
    __m128d ad = _mm_setr_pd(1.0, NAN);
    __m128d bd = _mm_setr_pd(NAN, 2.0);
    
    __m128d cmp_double[4];
    cmp_double[0] = _mm_cmp_pd(ad, bd, _CMP_UNORD_Q);
    cmp_double[1] = _mm_cmp_pd(ad, bd, _CMP_ORD_Q);
    cmp_double[2] = _mm_cmp_pd(ad, bd, _CMP_UNEQ_UQ);
    cmp_double[3] = _mm_cmp_pd(ad, bd, _CMP_NEQ_UQ);
    
    for (int i = 0; i < 4; i++) {
        int dmask = _mm_movemask_pd(cmp_double[i]);
        final_result |= (dmask << (12 + i));
    }
    
    /* Force assembly output with inline asm */
    __m128 temp = _mm_add_ps(a, b);
    float result_array[4];
    _mm_storeu_ps(result_array, temp);
    
    /* Use inline assembly to ensure condition codes appear in output */
    __asm__ __volatile__ (
        "vmovups %1, %%xmm0\n\t"
        "vmovups %2, %%xmm1\n\t"
        "vcmpps $3, %%xmm0, %%xmm1, %%xmm2\n\t"  /* UNORDERED */
        "vcmpps $7, %%xmm0, %%xmm1, %%xmm3\n\t"  /* ORDERED */
        "vmovups %%xmm2, %0\n\t"
        : "=m" (result_array)
        : "m" (a), "m" (b)
        : "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    printf("SSE test result: 0x%08x\n", final_result);
}

#ifdef __AVX__
/* Test AVX comparison condition codes */
void test_avx_conditions(void) {
    printf("Testing AVX condition codes...\n");
    
    /* Create 256-bit test vectors */
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, NAN, 9.0f);
    
    /* Test all condition codes with AVX */
    __m256 cmp_results[8];
    int masks[8];
    
    cmp_results[0] = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);    /* UNORDERED */
    cmp_results[1] = _mm256_cmp_ps(a, b, _CMP_ORD_Q);      /* ORDERED */
    cmp_results[2] = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);    /* UNEQ */
    cmp_results[3] = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);     /* UNGE */
    cmp_results[4] = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);     /* UNGT */
    cmp_results[5] = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);     /* UNLE */
    cmp_results[6] = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);     /* UNLT */
    cmp_results[7] = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);     /* LTGT */
    
    /* Extract masks and use in control flow */
    int final_result = 0;
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm256_movemask_ps(cmp_results[i]);
        if (masks[i] != 0) {
            final_result |= (1 << i);
            
            /* Use comparison results in blend operations */
            __m256 blended = _mm256_blendv_ps(a, b, cmp_results[i]);
            float blend_store[8];
            _mm256_storeu_ps(blend_store, blended);
            
            /* Prevent optimization */
            if (blend_store[0] > 0) {
                final_result ^= 0x1000;
            }
        }
    }
    
    /* Test AVX double precision */
    __m256d ad = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d bd = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    __m256d cmp_double[4];
    cmp_double[0] = _mm256_cmp_pd(ad, bd, _CMP_UNORD_Q);
    cmp_double[1] = _mm256_cmp_pd(ad, bd, _CMP_ORD_Q);
    cmp_double[2] = _mm256_cmp_pd(ad, bd, _CMP_UNEQ_UQ);
    cmp_double[3] = _mm256_cmp_pd(ad, bd, _CMP_NEQ_UQ);
    
    for (int i = 0; i < 4; i++) {
        int dmask = _mm256_movemask_pd(cmp_double[i]);
        final_result |= (dmask << (16 + i));
    }
    
    /* Complex expression mixing comparisons and arithmetic */
    __m256 cmp1 = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    __m256 cmp2 = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    __m256 arith1 = _mm256_add_ps(a, b);
    __m256 arith2 = _mm256_mul_ps(a, b);
    
    /* Blend based on comparison results */
    __m256 result1 = _mm256_blendv_ps(arith1, arith2, cmp1);
    __m256 result2 = _mm256_blendv_ps(result1, a, cmp2);
    
    float final_store[8];
    _mm256_storeu_ps(final_store, result2);
    
    /* Use in conditional */
    for (int i = 0; i < 8; i++) {
        if (!isnan(final_store[i]) && final_store[i] != 0.0f) {
            final_result += (int)final_store[i];
        }
    }
    
    printf("AVX test result: 0x%08x\n", final_result);
}
#endif

/* Test mixed-width comparisons and edge cases */
void test_edge_cases(void) {
    printf("Testing edge cases...\n");
    
    /* Test with all zeros */
    __m128 zero = _mm_setzero_ps();
    __m128 neg_zero = _mm_setr_ps(0.0f, -0.0f, 0.0f, -0.0f);
    
    __m128 cmp1 = _mm_cmp_ps(zero, neg_zero, _CMP_UNEQ_UQ);  /* Should be false */
    __m128 cmp2 = _mm_cmp_ps(zero, neg_zero, _CMP_EQ_OQ);    /* Should be true */
    
    /* Test with infinities */
    __m128 inf = _mm_set1_ps(INFINITY);
    __m128 neg_inf = _mm_set1_ps(-INFINITY);
    __m128 finite = _mm_set1_ps(100.0f);
    
    __m128 cmp3 = _mm_cmp_ps(inf, finite, _CMP_NLE_UQ);      /* UNGT variant */
    __m128 cmp4 = _mm_cmp_ps(neg_inf, finite, _CMP_NGE_UQ);  /* UNGE variant */
    
    /* Mix comparisons */
    int mask1 = _mm_movemask_ps(cmp1);
    int mask2 = _mm_movemask_ps(cmp2);
    int mask3 = _mm_movemask_ps(cmp3);
    int mask4 = _mm_movemask_ps(cmp4);
    
    printf("Edge case masks: %d %d %d %d\n", mask1, mask2, mask3, mask4);
}

int main(void) {
    printf("Starting condition code coverage test...\n");
    
    /* Always test SSE */
    test_sse_conditions();
    
    /* Test AVX if supported */
    if (avx_supported()) {
#ifdef __AVX__
        test_avx_conditions();
#endif
    } else {
        printf("AVX not supported, skipping AVX tests\n");
    }
    
    /* Test edge cases */
    test_edge_cases();
    
    printf("Test completed.\n");
    return 0;
}
