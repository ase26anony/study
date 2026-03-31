/* Test program to cover unordered floating-point comparison condition codes in i386.cc */
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent aggressive optimization */
volatile int sink;

int main(void) {
    int result = 0;
    
    /* NaN values */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    /* ==================== SCALAR COMPARISONS ==================== */
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;
    }
    
    /* ORDERED: __builtin_isordered */
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 2;
    }
    
    /* UNEQ: x != x (or comparison with NaN) */
    if (nan_f != nan_f) {
        result |= 4;
    }
    
    /* UNLT: x < y with NaN operand */
    if (nan_f < normal_f) {
        result |= 8;
    }
    
    /* UNLE: x <= y with NaN operand */
    if (nan_f <= normal_f) {
        result |= 16;
    }
    
    /* UNGT: x > y with NaN operand */
    if (nan_f > normal_f) {
        result |= 32;
    }
    
    /* UNGE: x >= y with NaN operand */
    if (nan_f >= normal_f) {
        result |= 64;
    }
    
    /* LTGT: __builtin_islessgreater */
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 128;
    }
    
    /* ==================== SSE VECTOR COMPARISONS ==================== */
    
    /* SSE single-precision */
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(nan_f, 2.0f, 3.0f, 4.0f);
    __m128 c;
    
    /* UNORDERED */
    c = _mm_cmpunord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    /* ORDERED */
    c = _mm_cmpord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    /* UNEQ */
    c = _mm_cmpneq_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    /* UNGE (nlt) */
    c = _mm_cmpnlt_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    /* UNGT (nle) */
    c = _mm_cmpnle_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    /* UNLE (ule) - Note: SSE doesn't have direct UNLE intrinsic, use combination */
    c = _mm_cmple_ps(a, b);  /* This generates LE, not UNLE directly */
    sink = _mm_movemask_ps(c);
    
    /* UNLT (ult) */
    c = _mm_cmplt_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    /* SSE double-precision */
    __m128d ad = _mm_setr_pd(normal_d, nan_d);
    __m128d bd = _mm_setr_pd(nan_d, normal_d);
    __m128d cd;
    
    /* UNORDERED */
    cd = _mm_cmpunord_pd(ad, bd);
    sink = _mm_movemask_pd(cd);
    
    /* ORDERED */
    cd = _mm_cmpord_pd(ad, bd);
    sink = _mm_movemask_pd(cd);
    
    /* UNEQ */
    cd = _mm_cmpneq_pd(ad, bd);
    sink = _mm_movemask_pd(cd);
    
    /* ==================== AVX VECTOR COMPARISONS ==================== */
    
#ifdef __AVX__
    /* AVX single-precision */
    __m256 av = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 bv = _mm256_setr_ps(nan_f, 2.0f, 3.0f, nan_f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256 cv;
    
    /* UNORDERED */
    cv = _mm256_cmp_ps(av, bv, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cv);
    
    /* ORDERED */
    cv = _mm256_cmp_ps(av, bv, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cv);
    
    /* UNEQ */
    cv = _mm256_cmp_ps(av, bv, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cv);
    
    /* UNGE (nlt) */
    cv = _mm256_cmp_ps(av, bv, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cv);
    
    /* UNGT (nle) */
    cv = _mm256_cmp_ps(av, bv, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cv);
    
    /* UNLE (ule) */
    cv = _mm256_cmp_ps(av, bv, _CMP_LE_OS);
    sink = _mm256_movemask_ps(cv);
    
    /* UNLT (ult) */
    cv = _mm256_cmp_ps(av, bv, _CMP_LT_OS);
    sink = _mm256_movemask_ps(cv);
    
    /* LTGT (une) */
    cv = _mm256_cmp_ps(av, bv, _CMP_NEQ_OQ);
    sink = _mm256_movemask_ps(cv);
    
    /* AVX double-precision */
    __m256d adv = _mm256_setr_pd(normal_d, nan_d, normal_d, nan_d);
    __m256d bdv = _mm256_setr_pd(nan_d, normal_d, nan_d, normal_d);
    __m256d cdv;
    
    /* UNORDERED */
    cdv = _mm256_cmp_pd(adv, bdv, _CMP_UNORD_Q);
    sink = _mm256_movemask_pd(cdv);
    
    /* ORDERED */
    cdv = _mm256_cmp_pd(adv, bdv, _CMP_ORD_Q);
    sink = _mm256_movemask_pd(cdv);
    
    /* UNEQ */
    cdv = _mm256_cmp_pd(adv, bdv, _CMP_NEQ_UQ);
    sink = _mm256_movemask_pd(cdv);
#endif
    
    /* ==================== LOOP TO PREVENT DEAD CODE ELIMINATION ==================== */
    
    float f1 = normal_f;
    float f2 = nan_f;
    int loop_result = 0;
    
    for (int i = 0; i < 10; i++) {
        /* Mix different comparison types in loop */
        if (__builtin_isunordered(f1, f2)) loop_result++;
        if (f1 != f1) loop_result++;  /* UNEQ */
        if (__builtin_islessgreater(f1, f2)) loop_result++;
        
        /* Alternate between NaN and normal values */
        f1 = (i % 2) ? normal_f : nan_f;
        f2 = (i % 3) ? normal_f : nan_f;
    }
    
    result += loop_result;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
