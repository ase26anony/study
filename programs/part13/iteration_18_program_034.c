/* Test program to cover unordered floating-point comparison condition codes in i386.cc */
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent aggressive optimization that might eliminate comparisons */
volatile float vf1, vf2;
volatile double vd1, vd2;

int main(void) {
    int results = 0;
    
    /* NaN constants */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    /* Regular numbers */
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    
    /* =============================== */
    /* Scalar unordered comparisons    */
    /* =============================== */
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(nan_f, f1)) results |= 1;
    if (__builtin_isunordered(f1, nan_f)) results |= 2;
    if (__builtin_isunordered(nan_d, d1)) results |= 4;
    
    /* ORDERED: __builtin_isordered (negation of unordered) */
    if (__builtin_isordered(f1, f2)) results |= 8;
    if (__builtin_isordered(d1, d2)) results |= 16;
    
    /* UNEQ: x == y where either is NaN */
    if (nan_f == nan_f) results |= 32;        /* Should be false, but tests UNEQ */
    if (!(nan_f != nan_f)) results |= 64;     /* Tests UNEQ through negation */
    
    /* UNLT: x < y with NaN operand */
    if (nan_f < f1) results |= 128;
    if (f1 < nan_f) results |= 256;
    
    /* UNLE: x <= y with NaN operand */
    if (nan_f <= f1) results |= 512;
    if (f1 <= nan_f) results |= 1024;
    
    /* UNGT: x > y with NaN operand */
    if (nan_f > f1) results |= 2048;
    if (f1 > nan_f) results |= 4096;
    
    /* UNGE: x >= y with NaN operand */
    if (nan_f >= f1) results |= 8192;
    if (f1 >= nan_f) results |= 16384;
    
    /* LTGT: __builtin_islessgreater */
    if (__builtin_islessgreater(nan_f, f1)) results |= 32768;
    if (__builtin_islessgreater(f1, nan_f)) results |= 65536;
    if (__builtin_islessgreater(f1, f2)) results |= 131072;
    
    /* =============================== */
    /* SSE vector comparisons (128-bit) */
    /* =============================== */
    
    /* Initialize SSE vectors */
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(5.0f, nan_f, 3.0f, 6.0f);
    __m128 vec_f3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_f4 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    __m128d vec_d1 = _mm_set_pd(nan_d, 1.0);
    __m128d vec_d2 = _mm_set_pd(2.0, nan_d);
    __m128d vec_d3 = _mm_set_pd(1.0, 2.0);
    __m128d vec_d4 = _mm_set_pd(2.0, 1.0);
    
    /* UNORDERED: _mm_cmpunord_ps/pd */
    __m128 res_unord_ps = _mm_cmpunord_ps(vec_f1, vec_f2);
    __m128d res_unord_pd = _mm_cmpunord_pd(vec_d1, vec_d2);
    
    /* ORDERED: _mm_cmpord_ps/pd */
    __m128 res_ord_ps = _mm_cmpord_ps(vec_f1, vec_f2);
    __m128d res_ord_pd = _mm_cmpord_pd(vec_d1, vec_d2);
    
    /* UNEQ: _mm_cmpneq_ps/pd (not equal, including unordered) */
    __m128 res_uneq_ps = _mm_cmpneq_ps(vec_f1, vec_f2);
    __m128d res_uneq_pd = _mm_cmpneq_pd(vec_d1, vec_d2);
    
    /* UNGE: _mm_cmpnlt_ps/pd (not less than) */
    __m128 res_unge_ps = _mm_cmpnlt_ps(vec_f1, vec_f2);
    __m128d res_unge_pd = _mm_cmpnlt_pd(vec_d1, vec_d2);
    
    /* UNGT: _mm_cmpnle_ps/pd (not less or equal) */
    __m128 res_ungt_ps = _mm_cmpnle_ps(vec_f1, vec_f2);
    __m128d res_ungt_pd = _mm_cmpnle_pd(vec_d1, vec_d2);
    
    /* UNLE: _mm_cmpule_ps/pd (unordered or less or equal) - SSE4.1 */
    #ifdef __SSE4_1__
    __m128 res_unle_ps = _mm_cmp_ps(vec_f1, vec_f2, _CMP_LE_OS);
    __m128d res_unle_pd = _mm_cmp_pd(vec_d1, vec_d2, _CMP_LE_OS);
    #endif
    
    /* UNLT: _mm_cmpult_ps/pd (unordered or less than) - SSE4.1 */
    #ifdef __SSE4_1__
    __m128 res_unlt_ps = _mm_cmp_ps(vec_f1, vec_f2, _CMP_LT_OS);
    __m128d res_unlt_pd = _mm_cmp_pd(vec_d1, vec_d2, _CMP_LT_OS);
    #endif
    
    /* LTGT: _mm_cmpneq_ps/pd with ordered operands */
    __m128 res_ltgt_ps = _mm_cmpneq_ps(vec_f3, vec_f4);
    __m128d res_ltgt_pd = _mm_cmpneq_pd(vec_d3, vec_d4);
    
    /* =============================== */
    /* AVX vector comparisons (256-bit) */
    /* =============================== */
    #ifdef __AVX__
    __m256 vec256_f1 = _mm256_set_ps(1.0f, nan_f, 3.0f, nan_f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec256_f2 = _mm256_set_ps(nan_f, 2.0f, nan_f, 4.0f, 8.0f, 7.0f, 6.0f, 5.0f);
    __m256 vec256_f3 = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec256_f4 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    
    __m256d vec256_d1 = _mm256_set_pd(nan_d, 2.0, nan_d, 4.0);
    __m256d vec256_d2 = _mm256_set_pd(1.0, nan_d, 3.0, nan_d);
    __m256d vec256_d3 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d vec256_d4 = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
    
    /* AVX unordered comparisons */
    __m256 res256_unord_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_UNORD_Q);
    __m256d res256_unord_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_UNORD_Q);
    
    __m256 res256_ord_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_ORD_Q);
    __m256d res256_ord_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_ORD_Q);
    
    __m256 res256_uneq_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NEQ_UQ);
    __m256d res256_uneq_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_NEQ_UQ);
    
    __m256 res256_unge_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NLT_US);
    __m256d res256_unge_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_NLT_US);
    
    __m256 res256_ungt_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NLE_US);
    __m256d res256_ungt_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_NLE_US);
    
    __m256 res256_unle_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_LE_OS);
    __m256d res256_unle_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_LE_OS);
    
    __m256 res256_unlt_ps = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_LT_OS);
    __m256d res256_unlt_pd = _mm256_cmp_pd(vec256_d1, vec256_d2, _CMP_LT_OS);
    
    __m256 res256_ltgt_ps = _mm256_cmp_ps(vec256_f3, vec256_f4, _CMP_NEQ_OQ);
    __m256d res256_ltgt_pd = _mm256_cmp_pd(vec256_d3, vec256_d4, _CMP_NEQ_OQ);
    #endif
    
    /* =============================== */
    /* Loop to prevent dead code elimination */
    /* =============================== */
    for (int i = 0; i < 10; i++) {
        vf1 = (i % 2) ? nan_f : (float)i;
        vf2 = (i % 3) ? nan_f : (float)(i * 2);
        vd1 = (i % 2) ? nan_d : (double)i;
        vd2 = (i % 3) ? nan_d : (double)(i * 2);
        
        /* Mix of ordered and unordered comparisons in loop */
        if (__builtin_isunordered(vf1, vf2)) results++;
        if (__builtin_islessgreater(vd1, vd2)) results++;
        if (vf1 != vf1) results++;  /* UNEQ test */
        if (vd1 == vd1) results++;  /* UNEQ test (false path) */
    }
    
    printf("Results: %d\n", results);
    return results != 0;
}
