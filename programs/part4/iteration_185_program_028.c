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

/* Force assembly generation by using volatile asm statements */
#define FORCE_ASM_OUTPUT(vec) \
    __asm__ __volatile__("" : "+x" (vec) : : "memory")

/* Extract mask and use it to prevent dead code elimination */
#define USE_RESULT(mask) \
    do { \
        int __mask = mask; \
        if (__mask) __asm__ __volatile__("" : : "r" (__mask) : "memory"); \
    } while(0)

void test_sse_comparisons(void) {
    printf("=== Testing SSE Comparisons ===\n");
    
    /* Create test vectors with various values including NaN */
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 3.0f, 5.0f, NAN);
    __m128 v3 = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 v4 = _mm_setr_ps(0.0f, 0.0f, INFINITY, INFINITY);
    
    __m128 result;
    int mask;
    
    /* Test all condition codes from the uncovered block */
    
    /* 1. UNORDERED (_CMP_UNORD_Q) - unordered (NaN) comparison */
    result = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 2. ORDERED (_CMP_ORD_Q) - ordered (non-NaN) comparison */
    result = _mm_cmp_ps(v3, v4, _CMP_ORD_Q);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 3. UNEQ (_CMP_UNEQ_UQ) - unordered or equal */
    result = _mm_cmp_ps(v1, v2, _CMP_UNEQ_UQ);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 4. UNGE (_CMP_NGE_UQ) - not greater than or equal (unordered) */
    result = _mm_cmp_ps(v1, v2, _CMP_NGE_UQ);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 5. UNGT (_CMP_NGT_UQ) - not greater than (unordered) */
    result = _mm_cmp_ps(v1, v2, _CMP_NGT_UQ);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 6. UNLE (_CMP_ULE_UQ) - unordered or less than or equal */
    result = _mm_cmp_ps(v1, v2, _CMP_ULE_UQ);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 7. UNLT (_CMP_ULT_UQ) - unordered or less than */
    result = _mm_cmp_ps(v1, v2, _CMP_ULT_UQ);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* 8. LTGT (_CMP_NEQ_UQ) - less than or greater than (unordered) */
    result = _mm_cmp_ps(v1, v2, _CMP_NEQ_UQ);
    mask = _mm_movemask_ps(result);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(result);
    
    /* Test double precision variants as well */
    __m128d d1 = _mm_setr_pd(1.0, NAN);
    __m128d d2 = _mm_setr_pd(2.0, NAN);
    
    __m128d dresult;
    
    /* Double precision UNORDERED */
    dresult = _mm_cmp_pd(d1, d2, _CMP_UNORD_Q);
    mask = _mm_movemask_pd(dresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(dresult);
    
    /* Double precision ORDERED */
    dresult = _mm_cmp_pd(d1, d2, _CMP_ORD_Q);
    mask = _mm_movemask_pd(dresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(dresult);
    
    /* Scalar comparisons also trigger the same code paths */
    __m128 sresult;
    sresult = _mm_cmp_ss(v1, v2, _CMP_UNORD_Q);
    FORCE_ASM_OUTPUT(sresult);
    
    sresult = _mm_cmp_ss(v1, v2, _CMP_ORD_Q);
    FORCE_ASM_OUTPUT(sresult);
}

#ifdef __AVX__
void test_avx_comparisons(void) {
    printf("=== Testing AVX Comparisons ===\n");
    
    /* AVX 256-bit vectors */
    __m256 av1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 av2 = _mm256_setr_ps(1.0f, 3.0f, 5.0f, NAN, 9.0f, 10.0f, 11.0f, 12.0f);
    
    __m256 aresult;
    int mask;
    
    /* Test all condition codes with AVX */
    aresult = _mm256_cmp_ps(av1, av2, _CMP_UNORD_Q);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_ORD_Q);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_UNEQ_UQ);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_NGE_UQ);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_NGT_UQ);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_ULE_UQ);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_ULT_UQ);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    aresult = _mm256_cmp_ps(av1, av2, _CMP_NEQ_UQ);
    mask = _mm256_movemask_ps(aresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(aresult);
    
    /* AVX double precision */
    __m256d ad1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d ad2 = _mm256_setr_pd(2.0, NAN, 3.0, 5.0);
    
    __m256d adresult;
    
    adresult = _mm256_cmp_pd(ad1, ad2, _CMP_UNORD_Q);
    mask = _mm256_movemask_pd(adresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(adresult);
    
    adresult = _mm256_cmp_pd(ad1, ad2, _CMP_ORD_Q);
    mask = _mm256_movemask_pd(adresult);
    USE_RESULT(mask);
    FORCE_ASM_OUTPUT(adresult);
}
#endif

/* Complex expression mixing comparisons with arithmetic */
void test_mixed_operations(void) {
    printf("=== Testing Mixed Operations ===\n");
    
    __m128 a = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 c = _mm_setr_ps(NAN, 0.0f, INFINITY, -INFINITY);
    
    /* Complex expression: blend based on comparison result */
    __m128 cmp_result = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    __m128 blended = _mm_blendv_ps(a, b, cmp_result);
    FORCE_ASM_OUTPUT(blended);
    
    /* Chain multiple comparisons */
    __m128 cmp1 = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    __m128 cmp2 = _mm_cmp_ps(b, c, _CMP_UNEQ_UQ);
    __m128 combined = _mm_and_ps(cmp1, cmp2);
    FORCE_ASM_OUTPUT(combined);
    
    /* Use comparison in arithmetic */
    __m128 mask = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    __m128 masked_add = _mm_add_ps(a, _mm_and_ps(b, mask));
    FORCE_ASM_OUTPUT(masked_add);
    
    /* Multiple condition codes in one function */
    __m128 results[8];
    results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    results[2] = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);
    results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);
    results[5] = _mm_cmp_ps(a, b, _CMP_ULE_UQ);
    results[6] = _mm_cmp_ps(a, b, _CMP_ULT_UQ);
    results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    /* Combine all results to prevent elimination */
    __m128 final = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        final = _mm_add_ps(final, results[i]);
    }
    
    float sum[4];
    _mm_storeu_ps(sum, final);
    printf("Final sum: %f %f %f %f\n", sum[0], sum[1], sum[2], sum[3]);
}

int main(void) {
    printf("Testing x86 condition code printing in assembly output\n");
    
    test_sse_comparisons();
    
#ifdef __AVX__
    test_avx_comparisons();
#else
    printf("AVX not available, skipping AVX tests\n");
#endif
    
    test_mixed_operations();
    
    /* Force compiler to generate assembly for all paths */
    volatile int dummy = 0;
    if (dummy) {
        /* Dead code that uses all condition codes to ensure they're in the binary */
        __m128 v = _mm_set1_ps(0.0f);
        __asm__ __volatile__(
            "vcmpps %%xmm0, %%xmm1, %0\n\t"
            "vcmpps %%xmm0, %%xmm1, %1\n\t"
            "vcmpps %%xmm0, %%xmm1, %2\n\t"
            "vcmpps %%xmm0, %%xmm1, %3\n\t"
            "vcmpps %%xmm0, %%xmm1, %4\n\t"
            "vcmpps %%xmm0, %%xmm1, %5\n\t"
            "vcmpps %%xmm0, %%xmm1, %6\n\t"
            "vcmpps %%xmm0, %%xmm1, %7"
            : 
            : "i" (_CMP_UNORD_Q),
              "i" (_CMP_ORD_Q),
              "i" (_CMP_UNEQ_UQ),
              "i" (_CMP_NGE_UQ),
              "i" (_CMP_NGT_UQ),
              "i" (_CMP_ULE_UQ),
              "i" (_CMP_ULT_UQ),
              "i" (_CMP_NEQ_UQ)
            : "xmm0", "xmm1"
        );
    }
    
    return 0;
}
