/* Test program to cover unordered floating-point comparison condition codes in i386.cc */
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent aggressive optimization */
volatile float vf1, vf2;
volatile double vd1, vd2;

int main(void) {
    int result_sum = 0;
    
    /* NaN constants */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    /* Regular floating-point values */
    float f1 = 3.14f, f2 = 2.71f;
    double d1 = 1.618, d2 = 0.577;
    
    /* ==================== SCALAR COMPARISONS ==================== */
    
    /* 1. UNORDERED condition - using __builtin_isunordered */
    if (__builtin_isunordered(nan_f, f1)) {
        result_sum += 1;  /* Branch taken */
    }
    
    /* 2. ORDERED condition - using !__builtin_isunordered */
    if (!__builtin_isunordered(f1, f2)) {
        result_sum += 2;  /* Branch taken */
    }
    
    /* 3. UNEQ condition - (x != x) or comparison with NaN */
    if (nan_f != nan_f) {  /* Always true for NaN */
        result_sum += 4;
    }
    
    /* 4. UNLT condition - less-than with NaN operand */
    if (nan_f < f1) {  /* False (unordered) */
        result_sum += 8;  /* Not taken */
    }
    
    /* 5. UNLE condition - less-or-equal with NaN operand */
    if (nan_f <= f1) {  /* False (unordered) */
        result_sum += 16;  /* Not taken */
    }
    
    /* 6. UNGT condition - greater-than with NaN operand */
    if (nan_f > f1) {  /* False (unordered) */
        result_sum += 32;  /* Not taken */
    }
    
    /* 7. UNGE condition - greater-or-equal with NaN operand */
    if (nan_f >= f1) {  /* False (unordered) */
        result_sum += 64;  /* Not taken */
    }
    
    /* 8. LTGT condition - using __builtin_islessgreater */
    if (__builtin_islessgreater(f1, f2)) {  /* 3.14 > 2.71, so true */
        result_sum += 128;
    }
    
    /* Double precision variants */
    if (__builtin_isunordered(nan_d, d1)) result_sum += 256;
    if (!__builtin_isunordered(d1, d2)) result_sum += 512;
    if (nan_d != nan_d) result_sum += 1024;
    
    /* ==================== SSE VECTOR COMPARISONS ==================== */
    
    /* SSE single-precision (__m128) */
    __m128 a = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_set_ps(1.0f, 3.0f, 5.0f, nan_f);
    __m128 c;
    
    /* UNORDERED */
    c = _mm_cmpunord_ps(a, b);
    result_sum += _mm_movemask_ps(c);
    
    /* ORDERED */
    c = _mm_cmpord_ps(a, b);
    result_sum += _mm_movemask_ps(c);
    
    /* UNEQ (not equal) */
    c = _mm_cmpneq_ps(a, b);
    result_sum += _mm_movemask_ps(c);
    
    /* UNGE (not less than) */
    c = _mm_cmpnlt_ps(a, b);
    result_sum += _mm_movemask_ps(c);
    
    /* UNGT (not less or equal) */
    c = _mm_cmpnle_ps(a, b);
    result_sum += _mm_movemask_ps(c);
    
    /* UNLE (unordered or less or equal) - note: _mm_cmpule_ps doesn't exist,
       we simulate with OR of unordered and less-or-equal */
    __m128 unord = _mm_cmpunord_ps(a, b);
    __m128 le = _mm_cmple_ps(a, b);
    c = _mm_or_ps(unord, le);
    result_sum += _mm_movemask_ps(c);
    
    /* UNLT (unordered or less than) */
    __m128 lt = _mm_cmplt_ps(a, b);
    c = _mm_or_ps(unord, lt);
    result_sum += _mm_movemask_ps(c);
    
    /* SSE double-precision (__m128d) */
    __m128d ad = _mm_set_pd(nan_d, 1.0);
    __m128d bd = _mm_set_pd(2.0, nan_d);
    
    c = _mm_castpd_ps(_mm_cmpunord_pd(ad, bd));
    result_sum += _mm_movemask_ps(c);
    
    c = _mm_castpd_ps(_mm_cmpord_pd(ad, bd));
    result_sum += _mm_movemask_ps(c);
    
    /* ==================== AVX VECTOR COMPARISONS ==================== */
    
#ifdef __AVX__
    /* AVX single-precision (__m256) */
    __m256 av = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 bv = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256 cv;
    
    /* UNORDERED */
    cv = _mm256_cmp_ps(av, bv, _CMP_UNORD_Q);
    result_sum += _mm256_movemask_ps(cv);
    
    /* ORDERED */
    cv = _mm256_cmp_ps(av, bv, _CMP_ORD_Q);
    result_sum += _mm256_movemask_ps(cv);
    
    /* UNEQ */
    cv = _mm256_cmp_ps(av, bv, _CMP_NEQ_UQ);
    result_sum += _mm256_movemask_ps(cv);
    
    /* UNGE */
    cv = _mm256_cmp_ps(av, bv, _CMP_NLT_UQ);
    result_sum += _mm256_movemask_ps(cv);
    
    /* UNGT */
    cv = _mm256_cmp_ps(av, bv, _CMP_NLE_UQ);
    result_sum += _mm256_movemask_ps(cv);
    
    /* UNLE */
    cv = _mm256_cmp_ps(av, bv, _CMP_LE_OS);
    result_sum += _mm256_movemask_ps(cv);
    
    /* UNLT */
    cv = _mm256_cmp_ps(av, bv, _CMP_LT_OS);
    result_sum += _mm256_movemask_ps(cv);
    
    /* LTGT */
    cv = _mm256_cmp_ps(av, bv, _CMP_NEQ_OQ);
    result_sum += _mm256_movemask_ps(cv);
    
    /* AVX double-precision (__m256d) */
    __m256d adv = _mm256_set_pd(nan_d, 2.0, nan_d, 4.0);
    __m256d bdv = _mm256_set_pd(1.0, nan_d, 3.0, nan_d);
    
    cv = _mm256_castpd_ps(_mm256_cmp_pd(adv, bdv, _CMP_UNORD_Q));
    result_sum += _mm256_movemask_ps(cv);
#endif
    
    /* ==================== LOOP TO PREVENT DEAD CODE ELIMINATION ==================== */
    
    /* Use volatile variables to force runtime evaluation */
    vf1 = nan_f;
    vf2 = f1;
    vd1 = nan_d;
    vd2 = d1;
    
    for (int i = 0; i < 3; i++) {
        /* Mix of different comparison types in loop */
        if (__builtin_isunordered(vf1, vf2)) result_sum += i;
        if (!__builtin_isunordered(vf2, vf1)) result_sum += i*2;
        if (vd1 != vd1) result_sum += i*4;
        if (__builtin_islessgreater(vd1, vd2)) result_sum += i*8;
        
        /* Change values slightly */
        vf2 += 0.1f;
        vd2 += 0.1;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result_sum);
    
    return 0;
}
