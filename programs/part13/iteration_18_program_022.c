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
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    /* Regular values */
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    
    /* ==================== SCALAR COMPARISONS ==================== */
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(nan_f, f1)) {
        result |= 1;
    }
    if (__builtin_isunordered(d1, nan_d)) {
        result |= 2;
    }
    
    /* ORDERED: !__builtin_isunordered */
    if (!__builtin_isunordered(f1, f2)) {
        result |= 4;
    }
    
    /* UNEQ: x == y, but either is NaN */
    if (nan_f != nan_f) {  /* UNEQ with NaN != NaN */
        result |= 8;
    }
    if (__builtin_isunordered(nan_f, nan_f) && !(nan_f < nan_f) && !(nan_f > nan_f)) {
        result |= 16;  /* Another way to get UNEQ */
    }
    
    /* UNLT: x < y, unordered */
    if (nan_f < f1) {  /* This generates UNLT when nan_f is NaN */
        result |= 32;
    }
    
    /* UNLE: x <= y, unordered */
    if (nan_f <= f1) {
        result |= 64;
    }
    
    /* UNGT: x > y, unordered */
    if (nan_f > f1) {
        result |= 128;
    }
    
    /* UNGE: x >= y, unordered */
    if (nan_f >= f1) {
        result |= 256;
    }
    
    /* LTGT: __builtin_islessgreater */
    if (__builtin_islessgreater(nan_f, f1)) {
        result |= 512;
    }
    if (__builtin_islessgreater(f1, nan_f)) {
        result |= 1024;
    }
    
    /* ==================== VECTOR COMPARISONS (SSE) ==================== */
    
    /* SSE single-precision */
    __m128 vf1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vf2 = _mm_set_ps(5.0f, 2.0f, 3.0f, nan_f);
    __m128 vf_nan = _mm_set1_ps(nan_f);
    
    /* UNORDERED */
    __m128 cmp_unord = _mm_cmpunord_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_unord);
    
    /* ORDERED */
    __m128 cmp_ord = _mm_cmpord_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_ord);
    
    /* UNEQ */
    __m128 cmp_ueq = _mm_cmpneq_ps(vf1, vf2);  /* Note: uneq mapping */
    sink = _mm_movemask_ps(cmp_ueq);
    
    /* UNGE (nlt) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_nlt);
    
    /* UNGT (nle) */
    __m128 cmp_nle = _mm_cmpnle_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_nle);
    
    /* UNLE (ule) */
    __m128 cmp_ule = _mm_cmpule_ps(vf_nan, vf1);
    sink = _mm_movemask_ps(cmp_ule);
    
    /* UNLT (ult) */
    __m128 cmp_ult = _mm_cmpult_ps(vf_nan, vf1);
    sink = _mm_movemask_ps(cmp_ult);
    
    /* LTGT (une) - same as UNEQ for some contexts */
    __m128 cmp_une = _mm_cmpneq_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_une);
    
    /* SSE double-precision */
    __m128d vd1 = _mm_set_pd(nan_d, 1.0);
    __m128d vd2 = _mm_set_pd(2.0, nan_d);
    
    /* Repeat for double precision */
    __m128d cmp_unord_d = _mm_cmpunord_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_ord_d);
    
    __m128d cmp_ueq_d = _mm_cmpneq_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_ueq_d);
    
    /* ==================== AVX COMPARISONS ==================== */
    
#ifdef __AVX__
    /* AVX single-precision */
    __m256 vf256_1 = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 vf256_2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, nan_f, 4.0f, 3.0f, 2.0f, 1.0f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_ord_256);
    
    __m256 cmp_ueq_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_ueq_256);
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp_nlt_256);
    
    __m256 cmp_nle_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp_nle_256);
    
    __m256 cmp_ule_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_LE_UQ);
    sink = _mm256_movemask_ps(cmp_ule_256);
    
    __m256 cmp_ult_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_LT_UQ);
    sink = _mm256_movemask_ps(cmp_ult_256);
    
    __m256 cmp_une_256 = _mm256_cmp_ps(vf256_1, vf256_2, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_une_256);
    
    /* AVX double-precision */
    __m256d vd256_1 = _mm256_set_pd(nan_d, 2.0, 3.0, nan_d);
    __m256d vd256_2 = _mm256_set_pd(4.0, nan_d, 2.0, 1.0);
    
    __m256d cmp_unord_256d = _mm256_cmp_pd(vd256_1, vd256_2, _CMP_UNORD_Q);
    sink = _mm256_movemask_pd(cmp_unord_256d);
#endif
    
    /* ==================== LOOP TO PREVENT DEAD CODE ELIMINATION ==================== */
    
    float dynamic_nan = nan_f;
    for (int i = 0; i < 10; i++) {
        /* Generate dynamic NaN through arithmetic */
        dynamic_nan = (dynamic_nan * 2.0f) / 0.0f;
        
        /* Use various unordered comparisons in loop */
        if (__builtin_isunordered(dynamic_nan, (float)i)) {
            result += i;
        }
        
        if (dynamic_nan != dynamic_nan) {
            result -= i;
        }
        
        if (dynamic_nan < i) {
            result ^= i;
        }
    }
    
    /* ==================== FINAL OUTPUT ==================== */
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
