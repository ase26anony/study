/* Test program to cover unordered floating-point comparison condition codes in i386.cc */
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent aggressive optimization */
volatile float vf1, vf2;
volatile double vd1, vd2;

int main(void) {
    int result = 0;
    
    /* NaN constants */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    /* Regular floating point values */
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    
    /* ===== SCALAR COMPARISONS ===== */
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(nan_f, f1)) {
        result |= 1 << 0;
    }
    
    /* ORDERED: __builtin_isordered */
    if (__builtin_isordered(f1, f2)) {
        result |= 1 << 1;
    }
    
    /* UNEQ: x != x or x == NaN */
    if (nan_f != nan_f) {  /* Always true for NaN */
        result |= 1 << 2;
    }
    
    /* UNLT: x < y with NaN operand */
    if (!(nan_f < f1)) {  /* Generates unordered comparison */
        result |= 1 << 3;
    }
    
    /* UNLE: x <= y with NaN operand */
    if (!(nan_f <= f1)) {
        result |= 1 << 4;
    }
    
    /* UNGT: x > y with NaN operand */
    if (!(nan_f > f1)) {
        result |= 1 << 5;
    }
    
    /* UNGE: x >= y with NaN operand */
    if (!(nan_f >= f1)) {
        result |= 1 << 6;
    }
    
    /* LTGT: __builtin_islessgreater */
    if (__builtin_islessgreater(f1, f2)) {
        result |= 1 << 7;
    }
    
    /* ===== SSE VECTOR COMPARISONS (128-bit) ===== */
    
    /* Initialize SSE vectors */
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(1.0f, 3.0f, 5.0f, nan_f);
    __m128 vec_nan = _mm_set1_ps(nan_f);
    
    __m128d vec_d1 = _mm_set_pd(1.0, nan_d);
    __m128d vec_d2 = _mm_set_pd(nan_d, 2.0);
    __m128d vec_nand = _mm_set1_pd(nan_d);
    
    /* UNORDERED: _mm_cmpunord_ps/pd */
    __m128 cmp_unord_ps = _mm_cmpunord_ps(vec_f1, vec_f2);
    __m128d cmp_unord_pd = _mm_cmpunord_pd(vec_d1, vec_d2);
    
    /* ORDERED: _mm_cmpord_ps/pd */
    __m128 cmp_ord_ps = _mm_cmpord_ps(vec_f1, vec_f2);
    __m128d cmp_ord_pd = _mm_cmpord_pd(vec_d1, vec_d2);
    
    /* UNEQ: _mm_cmpneq_ps/pd (with NaN operands) */
    __m128 cmp_neq_ps = _mm_cmpneq_ps(vec_nan, vec_f1);
    __m128d cmp_neq_pd = _mm_cmpneq_pd(vec_nand, vec_d1);
    
    /* UNGE: _mm_cmpnlt_ps/pd */
    __m128 cmp_nlt_ps = _mm_cmpnlt_ps(vec_f1, vec_f2);
    __m128d cmp_nlt_pd = _mm_cmpnlt_pd(vec_d1, vec_d2);
    
    /* UNGT: _mm_cmpnle_ps/pd */
    __m128 cmp_nle_ps = _mm_cmpnle_ps(vec_f1, vec_f2);
    __m128d cmp_nle_pd = _mm_cmpnle_pd(vec_d1, vec_d2);
    
    /* UNLE: _mm_cmpule_ps - Note: there's no direct _mm_cmpule_ps in SSE,
       but we can simulate with combination of comparisons */
    __m128 cmp_ule_ps = _mm_cmple_ps(vec_f1, vec_f2);  /* This may generate UNLE with NaN */
    
    /* UNLT: _mm_cmpult_ps/pd */
    __m128 cmp_ult_ps = _mm_cmpult_ps(vec_f1, vec_f2);
    __m128d cmp_ult_pd = _mm_cmpult_pd(vec_d1, vec_d2);
    
    /* LTGT: Using _mm_cmpneq_ps/pd with non-NaN operands */
    __m128 vec_reg1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_reg2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 cmp_ltgt_ps = _mm_cmpneq_ps(vec_reg1, vec_reg2);
    
    /* ===== AVX VECTOR COMPARISONS (256-bit) if available ===== */
    
#ifdef __AVX__
    /* Initialize AVX vectors */
    __m256 vec256_f1 = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec256_f2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, nan_f, 3.0f, 2.0f, 1.0f);
    __m256 vec256_nan = _mm256_set1_ps(nan_f);
    
    __m256d vec256_d1 = _mm256_set_pd(1.0, nan_d, 3.0, 4.0);
    __m256d vec256_d2 = _mm256_set_pd(4.0, 3.0, nan_d, 1.0);
    
    /* Various unordered AVX comparisons */
    __m256 cmp256_unord = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_UNORD_Q);
    __m256 cmp256_ord = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_ORD_Q);
    __m256 cmp256_neq = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NEQ_UQ);
    __m256 cmp256_nlt = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NLT_UQ);
    __m256 cmp256_nle = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NLE_UQ);
    
    /* Store to volatile variables to prevent dead code elimination */
    vf1 = ((float*)&cmp256_unord)[0];
    vf2 = ((float*)&cmp256_ord)[0];
#endif
    
    /* ===== MIXED OPERATIONS TO TRIGGER DIFFERENT PATTERNS ===== */
    
    /* Loop to prevent optimization and use different values */
    for (int i = 0; i < 10; i++) {
        float dynamic_f = (i % 2 == 0) ? f1 : nan_f;
        double dynamic_d = (i % 3 == 0) ? d1 : nan_d;
        
        /* Generate various condition codes in loop */
        if (__builtin_isunordered(dynamic_f, f2)) {
            result += i;
        }
        
        if (dynamic_d != dynamic_d) {  /* UNEQ */
            result -= i;
        }
        
        /* Ordered comparison with potential NaN */
        if (__builtin_isless(dynamic_f, f2)) {  /* May generate UNLT with NaN */
            result ^= i;
        }
    }
    
    /* ===== FINAL OUTPUT ===== */
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Also print some vector comparison results */
    float cmp_result[4];
    _mm_store_ps(cmp_result, cmp_unord_ps);
    printf("Unordered cmp mask: 0x%08x 0x%08x 0x%08x 0x%08x\n",
           *(uint32_t*)&cmp_result[0], *(uint32_t*)&cmp_result[1],
           *(uint32_t*)&cmp_result[2], *(uint32_t*)&cmp_result[3]);
    
    return result != 0 ? 0 : 1;
}
