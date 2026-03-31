/* Test program to cover unordered floating-point comparison condition codes in i386.cc */
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent aggressive optimization */
volatile int sink = 0;

int main(void) {
    int result = 0;
    
    /* 1. Scalar comparisons using builtins and NaN */
    float f1 = 1.0f;
    float f_nan = NAN;
    float f_inf = INFINITY;
    double d1 = 2.0;
    double d_nan = __builtin_nan("");
    double d_inf = __builtin_inf();
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(f1, f_nan)) {
        result |= 1;
    }
    
    /* ORDERED: __builtin_isordered */
    if (__builtin_isordered(f1, f1)) {
        result |= 2;
    }
    
    /* UNEQ: x != x (or x == NaN) */
    if (f_nan != f_nan) {  /* Always false but generates UNEQ */
        result |= 4;
    }
    if (d_nan == d_nan) {  /* Always false, UNEQ */
        result |= 8;
    }
    
    /* UNLT: x < y with NaN operand */
    if (f_nan < f1) {  /* False, generates UNLT */
        result |= 16;
    }
    
    /* UNLE: x <= y with NaN operand */
    if (f_nan <= f1) {  /* False, generates UNLE */
        result |= 32;
    }
    
    /* UNGT: x > y with NaN operand */
    if (f_nan > f1) {  /* False, generates UNGT */
        result |= 64;
    }
    
    /* UNGE: x >= y with NaN operand */
    if (f_nan >= f1) {  /* False, generates UNGE */
        result |= 128;
    }
    
    /* LTGT: __builtin_islessgreater */
    if (__builtin_islessgreater(f1, f_inf)) {
        result |= 256;
    }
    if (__builtin_islessgreater(d_nan, d1)) {  /* False with NaN */
        result |= 512;
    }
    
    /* 2. SSE vector comparisons (128-bit) */
    __m128 vf1 = _mm_set1_ps(1.0f);
    __m128 vf_nan = _mm_set1_ps(NAN);
    __m128 vf2 = _mm_set_ps(2.0f, 3.0f, 4.0f, 5.0f);
    
    /* UNORDERED */
    __m128 cmp_unord = _mm_cmpunord_ps(vf1, vf_nan);
    sink = _mm_movemask_ps(cmp_unord);
    
    /* ORDERED */
    __m128 cmp_ord = _mm_cmpord_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_ord);
    
    /* UNEQ */
    __m128 cmp_ueq = _mm_cmpneq_ps(vf_nan, vf_nan);
    sink = _mm_movemask_ps(cmp_ueq);
    
    /* UNGE (nlt) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(vf_nan, vf1);
    sink = _mm_movemask_ps(cmp_nlt);
    
    /* UNGT (nle) */
    __m128 cmp_nle = _mm_cmpnle_ps(vf_nan, vf1);
    sink = _mm_movemask_ps(cmp_nle);
    
    /* UNLE (ule) - Note: _mm_cmpule_ps doesn't exist, use sequence */
    __m128 cmp_ule = _mm_cmple_ps(vf_nan, vf1);  /* Generates UNLE with NaN */
    sink = _mm_movemask_ps(cmp_ule);
    
    /* UNLT (ult) */
    __m128 cmp_ult = _mm_cmplt_ps(vf_nan, vf1);  /* Generates UNLT with NaN */
    sink = _mm_movemask_ps(cmp_ult);
    
    /* LTGT (une) - same as UNEQ for unordered */
    __m128 cmp_une = _mm_cmpneq_ps(vf1, vf2);
    sink = _mm_movemask_ps(cmp_une);
    
    /* 3. Double precision SSE comparisons */
    __m128d vd1 = _mm_set1_pd(1.0);
    __m128d vd_nan = _mm_set1_pd(NAN);
    
    __m128d cmp_unord_pd = _mm_cmpunord_pd(vd1, vd_nan);
    sink = _mm_movemask_pd(cmp_unord_pd);
    
    __m128d cmp_ord_pd = _mm_cmpord_pd(vd1, vd1);
    sink = _mm_movemask_pd(cmp_ord_pd);
    
    /* 4. AVX comparisons (256-bit) if AVX is enabled */
#ifdef __AVX__
    __m256 vf256_1 = _mm256_set1_ps(1.0f);
    __m256 vf256_nan = _mm256_set1_ps(NAN);
    
    /* UNORDERED */
    __m256 cmp_unord_256 = _mm256_cmp_ps(vf256_1, vf256_nan, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_unord_256);
    
    /* ORDERED */
    __m256 cmp_ord_256 = _mm256_cmp_ps(vf256_1, vf256_1, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_ord_256);
    
    /* UNEQ */
    __m256 cmp_ueq_256 = _mm256_cmp_ps(vf256_nan, vf256_nan, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_ueq_256);
    
    /* UNGE (nlt) */
    __m256 cmp_nlt_256 = _mm256_cmp_ps(vf256_nan, vf256_1, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp_nlt_256);
    
    /* UNGT (nle) */
    __m256 cmp_nle_256 = _mm256_cmp_ps(vf256_nan, vf256_1, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp_nle_256);
    
    /* UNLE (ule) */
    __m256 cmp_ule_256 = _mm256_cmp_ps(vf256_nan, vf256_1, _CMP_LE_UQ);
    sink = _mm256_movemask_ps(cmp_ule_256);
    
    /* UNLT (ult) */
    __m256 cmp_ult_256 = _mm256_cmp_ps(vf256_nan, vf256_1, _CMP_LT_UQ);
    sink = _mm256_movemask_ps(cmp_ult_256);
    
    /* LTGT (une) */
    __m256 cmp_une_256 = _mm256_cmp_ps(vf256_1, vf256_1, _CMP_NEQ_OQ);
    sink = _mm256_movemask_ps(cmp_une_256);
#endif
    
    /* 5. Loop to prevent dead code elimination and generate more comparisons */
    float arr[4] = {1.0f, NAN, 2.0f, INFINITY};
    double darr[4] = {1.0, NAN, 2.0, INFINITY};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Generate various condition codes through different comparisons */
            if (__builtin_isunordered(arr[i], arr[j])) {
                result++;
            }
            if (arr[i] != arr[j]) {  /* Can generate UNEQ with NaN */
                result++;
            }
            if (arr[i] < arr[j]) {   /* Can generate UNLT/UNLE with NaN */
                result++;
            }
            if (darr[i] > darr[j]) { /* Can generate UNGT/UNGE with NaN */
                result++;
            }
        }
    }
    
    printf("Result: %d\n", result);
    return result != 0;
}
