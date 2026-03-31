/* Test program to cover unordered floating-point comparison condition codes in i386.cc */
#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

/* Prevent aggressive optimization */
volatile float vf1, vf2;
volatile double vd1, vd2;

int main(void) {
    int results = 0;
    
    /* NaN definitions */
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    /* Regular float/double variables */
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    
    /* ============================= */
    /* 1. UNORDERED comparisons      */
    /* ============================= */
    
    /* UNORDERED: __builtin_isunordered */
    if (__builtin_isunordered(nan_f, f1)) results |= 1;
    if (__builtin_isunordered(f2, nan_f)) results |= 2;
    if (__builtin_isunordered(nan_d, d1)) results |= 4;
    
    /* ORDERED: !__builtin_isunordered */
    if (!__builtin_isunordered(f1, f2)) results |= 8;
    if (!__builtin_isunordered(d1, d2)) results |= 16;
    
    /* UNEQ: x == y (unordered allowed) */
    if (nan_f == nan_f) results |= 32;           /* UNEQ with NaN */
    if (__builtin_isunordered(f1, f1)) results |= 64; /* Another UNORDERED */
    
    /* UNLT: x < y (unordered allowed) */
    if (nan_f < f1) results |= 128;
    if (f2 < nan_f) results |= 256;
    
    /* UNLE: x <= y (unordered allowed) */
    if (nan_f <= f1) results |= 512;
    if (f2 <= nan_f) results |= 1024;
    
    /* UNGT: x > y (unordered allowed) */
    if (nan_f > f1) results |= 2048;
    if (f2 > nan_f) results |= 4096;
    
    /* UNGE: x >= y (unordered allowed) */
    if (nan_f >= f1) results |= 8192;
    if (f2 >= nan_f) results |= 16384;
    
    /* LTGT: __builtin_islessgreater */
    if (__builtin_islessgreater(nan_f, f1)) results |= 32768;
    if (__builtin_islessgreater(f1, nan_f)) results |= 65536;
    if (__builtin_islessgreater(f1, f2)) results |= 131072;
    
    /* ============================= */
    /* 2. SSE intrinsics (128-bit)   */
    /* ============================= */
    
    /* Initialize SSE vectors */
    __m128 sse_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 sse_f2 = _mm_set_ps(5.0f, nan_f, 3.0f, 6.0f);
    __m128 sse_f3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 sse_f4 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    
    /* UNORDERED: _mm_cmpunord_ps */
    __m128 cmp_unord = _mm_cmpunord_ps(sse_f1, sse_f2);
    
    /* ORDERED: _mm_cmpord_ps */
    __m128 cmp_ord = _mm_cmpord_ps(sse_f3, sse_f4);
    
    /* UNEQ: _mm_cmpneq_ps (not equal, unordered allowed) */
    __m128 cmp_uneq = _mm_cmpneq_ps(sse_f1, sse_f2);
    
    /* UNGE: _mm_cmpnlt_ps (not less than) */
    __m128 cmp_nlt = _mm_cmpnlt_ps(sse_f1, sse_f2);
    
    /* UNGT: _mm_cmpnle_ps (not less or equal) */
    __m128 cmp_nle = _mm_cmpnle_ps(sse_f1, sse_f2);
    
    /* UNLE: _mm_cmpule_ps (unordered or less or equal) */
    /* Note: There's no direct _mm_cmpule_ps in standard SSE.
       We'll use combination: (x <= y) OR (x != x) OR (y != y) */
    __m128 cmp_unord1 = _mm_cmpunord_ps(sse_f1, sse_f1);
    __m128 cmp_unord2 = _mm_cmpunord_ps(sse_f2, sse_f2);
    __m128 cmp_ule = _mm_or_ps(_mm_cmple_ps(sse_f1, sse_f2),
                              _mm_or_ps(cmp_unord1, cmp_unord2));
    
    /* UNLT: _mm_cmpult_ps (unordered or less than) */
    __m128 cmp_ult = _mm_or_ps(_mm_cmplt_ps(sse_f1, sse_f2),
                              _mm_or_ps(cmp_unord1, cmp_unord2));
    
    /* LTGT: _mm_cmpneq_ps (not equal, both ordered) */
    __m128 cmp_ltgt = _mm_and_ps(_mm_cmpneq_ps(sse_f3, sse_f4),
                                _mm_cmpord_ps(sse_f3, sse_f4));
    
    /* Store results to prevent elimination */
    float store[4];
    _mm_storeu_ps(store, cmp_unord);
    _mm_storeu_ps(store + 4, cmp_ord);
    _mm_storeu_ps(store + 8, cmp_uneq);
    
    /* ============================= */
    /* 3. SSE2 double precision      */
    /* ============================= */
    
    __m128d sse_d1 = _mm_set_pd(nan_d, 1.0);
    __m128d sse_d2 = _mm_set_pd(2.0, nan_d);
    
    /* Various unordered comparisons for doubles */
    __m128d cmp_unord_d = _mm_cmpunord_pd(sse_d1, sse_d2);
    __m128d cmp_ord_d = _mm_cmpord_pd(sse_d1, sse_d2);
    __m128d cmp_neq_d = _mm_cmpneq_pd(sse_d1, sse_d2);
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(sse_d1, sse_d2);
    __m128d cmp_nle_d = _mm_cmpnle_pd(sse_d1, sse_d2);
    
    double store_d[4];
    _mm_storeu_pd(store_d, cmp_unord_d);
    _mm_storeu_pd(store_d + 2, cmp_ord_d);
    
    /* ============================= */
    /* 4. AVX intrinsics (256-bit)   */
    /* ============================= */
    
#ifdef __AVX__
    __m256 avx_f1 = _mm256_set_ps(1.0f, nan_f, 3.0f, nan_f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_f2 = _mm256_set_ps(nan_f, 2.0f, nan_f, 4.0f, 9.0f, 10.0f, 11.0f, 12.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(avx_f1, avx_f2, _CMP_UNORD_Q);
    __m256 cmp_ord_avx = _mm256_cmp_ps(avx_f1, avx_f2, _CMP_ORD_Q);
    __m256 cmp_neq_avx = _mm256_cmp_ps(avx_f1, avx_f2, _CMP_NEQ_UQ);
    __m256 cmp_nlt_avx = _mm256_cmp_ps(avx_f1, avx_f2, _CMP_NLT_UQ);
    __m256 cmp_nle_avx = _mm256_cmp_ps(avx_f1, avx_f2, _CMP_NLE_UQ);
    
    float store_avx[8];
    _mm256_storeu_ps(store_avx, cmp_unord_avx);
#endif
    
    /* ============================= */
    /* 5. Loop with varying values   */
    /* ============================= */
    
    for (int i = 0; i < 10; i++) {
        volatile float a = (i & 1) ? nan_f : (float)i;
        volatile float b = (i & 2) ? nan_f : (float)(i * 2);
        
        /* Trigger various condition codes */
        if (__builtin_isunordered(a, b)) results++;
        if (!__builtin_isunordered(a, b)) results++;
        if (a == b) results++;
        if (a < b) results++;
        if (a <= b) results++;
        if (a > b) results++;
        if (a >= b) results++;
        if (__builtin_islessgreater(a, b)) results++;
    }
    
    /* ============================= */
    /* 6. Complex expressions        */
    /* ============================= */
    
    /* Mixed NaN propagation */
    float f3 = nan_f * 2.0f;
    float f4 = inf_f / inf_f;  /* Creates NaN */
    
    if (__builtin_isunordered(f3, f4)) results += 1000;
    if (f3 != f3) results += 2000;  /* UNEQ */
    
    /* Ternary operator forcing condition code generation */
    int r1 = (nan_f < f1) ? 1 : 0;
    int r2 = (f1 <= nan_f) ? 2 : 0;
    int r3 = (__builtin_islessgreater(nan_d, d1)) ? 4 : 0;
    
    results += r1 + r2 + r3;
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d\n", results);
    printf("Store[0]: %f\n", store[0]);
    printf("Store_d[0]: %f\n", store_d[0]);
    
    return (results > 0) ? 0 : 1;
}
