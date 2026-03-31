#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

/* Helper to check if AVX is available at runtime */
#ifdef __AVX__
static int avx_available = 1;
#else
static int avx_available = 0;
#endif

/* Test all SSE comparison condition codes */
void test_sse_conditions(void) {
    printf("Testing SSE condition codes...\n");
    
    /* Create test vectors with various values including NaN */
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 d = _mm_setr_ps(0.0f, 0.0f, INFINITY, INFINITY);
    
    /* Results array to prevent optimization */
    volatile int results[8] = {0};
    int idx = 0;
    
    /* Test each condition code from the uncovered block */
    __m128 cmp_result;
    
    /* 1. UNORDERED (_CMP_UNORD_Q) - true if either operand is NaN */
    cmp_result = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 2. ORDERED (_CMP_ORD_Q) - true if neither operand is NaN */
    cmp_result = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 3. UNEQ (_CMP_UNEQ_UQ) - unordered or equal */
    cmp_result = _mm_cmp_ps(c, d, _CMP_UNEQ_UQ);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 4. UNGE (_CMP_NGE_UQ) - not greater than or equal (unordered) */
    cmp_result = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 5. UNGT (_CMP_NGT_UQ) - not greater than (unordered) */
    cmp_result = _mm_cmp_ps(a, b, _CMP_NGT_UQ);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 6. UNLE (_CMP_ULE_UQ) - unordered or less than or equal */
    cmp_result = _mm_cmp_ps(a, b, _CMP_ULE_UQ);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 7. UNLT (_CMP_ULT_UQ) - unordered or less than */
    cmp_result = _mm_cmp_ps(a, b, _CMP_ULT_UQ);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* 8. LTGT (_CMP_NEQ_UQ) - less than or greater than (unordered) */
    cmp_result = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    results[idx++] = _mm_movemask_ps(cmp_result);
    
    /* Use results in conditional to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        if (results[i] != 0) {
            sum += results[i];
        }
    }
    printf("SSE comparison mask sum: %d\n", sum);
    
    /* Test scalar versions too */
    __m128 s1 = _mm_set_ss(1.0f);
    __m128 s2 = _mm_set_ss(NAN);
    
    /* Force assembly output for scalar comparisons */
    __asm__ __volatile__ (
        "# SSE scalar comparison with UNORDERED\n"
        "vcmpeqss %1, %0, %0\n"
        : "+x" (s1)
        : "x" (s2)
    );
    
    /* Test double precision as well */
    __m128d da = _mm_setr_pd(1.0, NAN);
    __m128d db = _mm_setr_pd(NAN, 2.0);
    
    __m128d dcmp = _mm_cmp_pd(da, db, _CMP_UNORD_Q);
    int dmask = _mm_movemask_pd(dcmp);
    printf("Double UNORDERED mask: %d\n", dmask);
    
    dcmp = _mm_cmp_pd(da, db, _CMP_ORD_Q);
    dmask = _mm_movemask_pd(dcmp);
    printf("Double ORDERED mask: %d\n", dmask);
}

/* Test AVX comparison condition codes if available */
void test_avx_conditions(void) {
#ifdef __AVX__
    printf("Testing AVX condition codes...\n");
    
    /* Create 256-bit test vectors */
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, NAN, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, 7.0f, NAN);
    
    volatile int results[8] = {0};
    int idx = 0;
    
    /* Test all condition codes with AVX */
    __m256 cmp_result;
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    results[idx++] = _mm256_movemask_ps(cmp_result);
    
    /* Use results in complex expression */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum = (sum << 1) | (results[i] & 1);
    }
    printf("AVX comparison pattern: 0x%08x\n", sum);
    
    /* Test AVX double precision */
    __m256d da = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d db = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    __m256d dcmp = _mm256_cmp_pd(da, db, _CMP_UNORD_Q);
    int dmask = _mm256_movemask_pd(dcmp);
    printf("AVX Double UNORDERED mask: %d\n", dmask);
    
    /* Force assembly generation with inline asm */
    __asm__ __volatile__ (
        "# AVX comparison with multiple condition codes\n"
        "vcmpeqps %1, %0, %0\n\t"
        "vcmpltps %1, %0, %0\n"
        : "+x" (a)
        : "x" (b)
    );
#endif
}

/* Complex expression mixing comparisons and arithmetic */
void test_mixed_operations(void) {
    printf("Testing mixed operations...\n");
    
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 v3 = _mm_setr_ps(NAN, 0.0f, INFINITY, -INFINITY);
    
    /* Chain comparisons with different condition codes */
    __m128 cmp1 = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ps(v2, v3, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ps(v1, v3, _CMP_UNEQ_UQ);
    
    /* Blend results based on comparison masks */
    __m128 blended = _mm_blendv_ps(v1, v2, cmp1);
    blended = _mm_blendv_ps(blended, v3, cmp2);
    
    /* Arithmetic mixed with comparisons */
    __m128 add_result = _mm_add_ps(v1, v2);
    __m128 cmp4 = _mm_cmp_ps(add_result, v3, _CMP_NGE_UQ);
    blended = _mm_blendv_ps(blended, add_result, cmp4);
    
    /* Extract and use mask for control flow */
    int mask = _mm_movemask_ps(blended);
    if (mask & 1) {
        printf("First element blended from v1\n");
    }
    if (mask & 2) {
        printf("Second element blended from v2\n");
    }
    
    /* Test all condition codes in a loop */
    const int cond_codes[] = {
        _CMP_UNORD_Q,    /* UNORDERED */
        _CMP_ORD_Q,      /* ORDERED */
        _CMP_UNEQ_UQ,    /* UNEQ */
        _CMP_NGE_UQ,     /* UNGE */
        _CMP_NGT_UQ,     /* UNGT */
        _CMP_ULE_UQ,     /* UNLE */
        _CMP_ULT_UQ,     /* UNLT */
        _CMP_NEQ_UQ      /* LTGT */
    };
    
    const char* cond_names[] = {
        "UNORDERED", "ORDERED", "UNEQ", "UNGE", 
        "UNGT", "UNLE", "UNLT", "LTGT"
    };
    
    volatile int masks[8];
    for (int i = 0; i < 8; i++) {
        __m128 result = _mm_cmp_ps(v1, v3, cond_codes[i]);
        masks[i] = _mm_movemask_ps(result);
        printf("Condition %s mask: %d\n", cond_names[i], masks[i]);
    }
}

int main(void) {
    printf("Testing x86 condition code printing paths\n");
    printf("=========================================\n");
    
    /* Test SSE conditions */
    test_sse_conditions();
    
    /* Test mixed operations */
    test_mixed_operations();
    
    /* Test AVX if available */
    if (avx_available) {
        test_avx_conditions();
    } else {
        printf("AVX not available, skipping AVX tests\n");
    }
    
    /* Final computation using all test results */
    float final_result = 0.0f;
    __m128 final_vec = _mm_set1_ps(1.0f);
    
    /* One more comparison to ensure all paths are exercised */
    __m128 nan_vec = _mm_set1_ps(NAN);
    __m128 final_cmp = _mm_cmp_ps(final_vec, nan_vec, _CMP_UNORD_Q);
    
    /* Force the compiler to generate assembly for the comparison */
    __asm__ __volatile__ (
        "# Final comparison forcing condition code output\n"
        : "+x" (final_cmp)
        : 
    );
    
    int final_mask = _mm_movemask_ps(final_cmp);
    printf("Final comparison mask: %d\n", final_mask);
    
    return final_mask != 0 ? 0 : 1;
}
