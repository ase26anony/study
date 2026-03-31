/* Test program to cover unordered floating-point comparison condition codes
   in GCC's i386 backend (output_fp_compare function) */
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

/* SSE/AVX intrinsics */
#include <immintrin.h>

/* Prevent aggressive optimization */
volatile int sink = 0;

int main(void) {
    int results[32] = {0};
    int idx = 0;
    
    /* NaN values */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    /* Regular values */
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    
    /* ==================== SCALAR COMPARISONS ==================== */
    
    /* UNORDERED condition - using builtin */
    if (__builtin_isunordered(f1, nan_f)) {
        results[idx++] = 1;  /* Should be true */
    }
    
    /* UNORDERED condition - direct NaN comparison */
    if (f1 != f1) {  /* false for normal number */
        results[idx++] = 2;
    }
    if (nan_f != nan_f) {  /* true for NaN - generates UNEQ */
        results[idx++] = 3;
    }
    
    /* ORDERED condition - using builtin */
    if (!__builtin_isunordered(f1, f2)) {
        results[idx++] = 4;  /* Should be true */
    }
    
    /* UNEQ condition - equality with NaN */
    if (nan_f == nan_f) {  /* false - generates UNEQ */
        results[idx++] = 5;
    }
    
    /* UNLT condition - less than with NaN operand */
    if (nan_f < f1) {  /* false - generates UNLT */
        results[idx++] = 6;
    }
    
    /* UNLE condition - less or equal with NaN operand */
    if (nan_f <= f1) {  /* false - generates UNLE */
        results[idx++] = 7;
    }
    
    /* UNGT condition - greater than with NaN operand */
    if (nan_f > f1) {  /* false - generates UNGT */
        results[idx++] = 8;
    }
    
    /* UNGE condition - greater or equal with NaN operand */
    if (nan_f >= f1) {  /* false - generates UNGE */
        results[idx++] = 9;
    }
    
    /* LTGT condition - using builtin */
    if (__builtin_islessgreater(f1, f2)) {
        results[idx++] = 10;  /* true for 1.5 < 2.5 */
    }
    if (__builtin_islessgreater(nan_f, f1)) {
        results[idx++] = 11;  /* false for NaN */
    }
    
    /* Double precision comparisons */
    if (__builtin_isunordered(d1, nan_d)) {
        results[idx++] = 12;
    }
    if (nan_d == nan_d) {  /* UNEQ */
        results[idx++] = 13;
    }
    if (nan_d < d1) {  /* UNLT */
        results[idx++] = 14;
    }
    
    /* ==================== VECTOR COMPARISONS (SSE) ==================== */
    
    /* SSE single-precision */
    __m128 vf1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vf2 = _mm_setr_ps(5.0f, 2.0f, 3.0f, nan_f);
    __m128 vnan = _mm_set1_ps(nan_f);
    
    /* UNORDERED - _mm_cmpunord_ps */
    __m128 cmp_unord = _mm_cmpunord_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_unord);
    
    /* ORDERED - _mm_cmpord_ps */
    __m128 cmp_ord = _mm_cmpord_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_ord);
    
    /* UNEQ - _mm_cmpneq_ps (not equal, including unordered) */
    __m128 cmp_neq = _mm_cmpneq_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_neq);
    
    /* UNGE - _mm_cmpnlt_ps (not less than) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_nlt);
    
    /* UNGT - _mm_cmpnle_ps (not less or equal) */
    __m128 cmp_nle = _mm_cmpnle_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_nle);
    
    /* UNLE - _mm_cmpule_ps (unordered or less or equal) - note: not standard SSE */
    /* Using alternative: compare with NaN to generate unordered */
    __m128 cmp_ule = _mm_cmpunord_ps(vf1, vnan);
    sink = _mm_movemask_ps(cmp_ule);
    
    /* UNLT - _mm_cmpult_ps (unordered or less than) - note: not standard SSE */
    /* Using alternative */
    __m128 cmp_ult = _mm_cmpunord_ps(vf2, vnan);
    sink = _mm_movemask_ps(cmp_ult);
    
    /* SSE double-precision */
    __m128d vd1 = _mm_setr_pd(1.0, nan_d);
    __m128d vd2 = _mm_setr_pd(nan_d, 2.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_ord_d);
    
    /* ==================== VECTOR COMPARISONS (AVX) ==================== */
#ifdef __AVX__
    /* AVX single-precision */
    __m256 vf1_avx = _mm256_setr_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256 vf2_avx = _mm256_setr_ps(nan_f, 2.0f, 3.0f, nan_f, 5.0f, 6.0f, nan_f, 8.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vf1_avx, vf2_avx, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vf1_avx, vf2_avx, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_ord_avx);
    
    __m256 cmp_neq_avx = _mm256_cmp_ps(vf1_avx, vf2_avx, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_neq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vf1_avx, vf2_avx, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vf1_avx, vf2_avx, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp_nle_avx);
    
    /* AVX double-precision */
    __m256d vd1_avx = _mm256_setr_pd(1.0, nan_d, 3.0, nan_d);
    __m256d vd2_avx = _mm256_setr_pd(nan_d, 2.0, nan_d, 4.0);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vd1_avx, vd2_avx, _CMP_UNORD_Q);
    sink = _mm256_movemask_pd(cmp_unord_avx_d);
#endif
    
    /* ==================== LOOP TO PREVENT DEAD CODE ELIMINATION ==================== */
    
    float dynamic_nan = nan_f;
    double dynamic_nan_d = nan_d;
    
    for (int i = 0; i < 10; i++) {
        /* Vary NaN values through arithmetic */
        dynamic_nan = nan_f * (i + 1);
        dynamic_nan_d = nan_d / (i + 1);
        
        /* Generate various unordered conditions */
        if (__builtin_isunordered(f1 + i, dynamic_nan)) {
            results[idx++ % 32] = 20 + i;
        }
        
        if (dynamic_nan == dynamic_nan) {  /* UNEQ */
            results[idx++ % 32] = 30 + i;
        }
        
        if (dynamic_nan < (f1 + i)) {  /* UNLT */
            results[idx++ % 32] = 40 + i;
        }
        
        /* LTGT with varying values */
        if (__builtin_islessgreater(f1 + i, f2 + i)) {
            results[idx++ % 32] = 50 + i;
        }
    }
    
    /* ==================== AGGREGATE AND PRINT RESULTS ==================== */
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    
    printf("Result checksum: %d\n", sum);
    printf("Test completed - unordered FP comparisons generated\n");
    
    return 0;
}
