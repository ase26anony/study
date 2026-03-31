#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ========== V64QImode (64-byte integers) ========== */
int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask: select a[i] if (i % 3 == 0) */
    __m512i idx = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    __m512i mod3 = _mm512_and_si512(idx, _mm512_set1_epi8(3));
    mask = _mm512_cmpeq_epi8_mask(mod3, _mm512_setzero_si512());
    
    /* Critical blend operation for V64QImode */
    vresult = _mm512_mask_blend_epi8(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HImode (32-halfword integers) ========== */
int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate mask using comparison */
    __m512i threshold = _mm512_set1_epi16(100);
    mask = _mm512_cmpgt_epi16_mask(va, threshold);
    
    /* Blend operation for V32HImode */
    vresult = _mm512_mask_blend_epi16(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V32HFmode (32-half-precision floats) ========== */
#ifdef __AVX512FP16__
float test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    /* Generate mask */
    __m512h threshold = _mm512_set1_ph(10.0f);
    mask = _mm512_cmp_ph_mask(va, threshold, _CMP_GT_OQ);
    
    /* Blend operation for V32HFmode */
    vresult = _mm512_mask_blend_ph(mask, vb, va);
    
    _mm512_storeu_ph(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ========== V32BFmode (32-bfloat16) ========== */
#ifdef __AVX512BF16__
float test_v32bf_blend(void) {
    __bfloat16 a[32], b[32], result[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)i * 1.5f);
        b[i] = bfloat16_from_float((float)i * 2.5f);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* For bfloat16, we need to use integer blend since there's no direct BF16 blend */
    /* This should still trigger the V32BFmode case */
    __m512i vai = _mm512_castps_si512(_mm512_cvtne2ps_pbh(va, va));
    __m512i vbi = _mm512_castps_si512(_mm512_cvtne2ps_pbh(vb, vb));
    
    /* Generate mask */
    __m512i threshold = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
    mask = _mm512_cmpgt_epi16_mask(vai, threshold);
    
    /* Use epi16 blend for bfloat16 data */
    __m512i vresulti = _mm512_mask_blend_epi16(mask, vbi, vai);
    
    _mm512_storeu_si512((__m512i*)result, vresulti);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ========== V16SImode (16-dword integers) ========== */
int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 150;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate mask using bitwise operation */
    __m512i pattern = _mm512_set1_epi32(0x55555555);
    mask = _mm512_test_epi32_mask(va, pattern);
    
    /* Blend operation for V16SImode */
    vresult = _mm512_mask_blend_epi32(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DImode (8-qword integers) ========== */
long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 1000LL;
        b[i] = (long long)i * 2000LL;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate mask */
    __m512i threshold = _mm512_set1_epi64(3000);
    mask = _mm512_cmpgt_epi64_mask(va, threshold);
    
    /* Blend operation for V8DImode */
    vresult = _mm512_mask_blend_epi64(mask, vb, va);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V8DFmode (8-double-precision floats) ========== */
double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.1;
        b[i] = (double)i * 2.2;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    /* Generate mask using comparison */
    __m512d threshold = _mm512_set1_pd(3.0);
    mask = _mm512_cmp_pd_mask(va, threshold, _CMP_GT_OQ);
    
    /* Blend operation for V8DFmode */
    vresult = _mm512_mask_blend_pd(mask, vb, va);
    
    _mm512_storeu_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== V16SFmode (16-single-precision floats) ========== */
float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 1.5f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    /* Generate mask */
    __m512 threshold = _mm512_set1_ps(4.0f);
    mask = _mm512_cmp_ps_mask(va, threshold, _CMP_GT_OQ);
    
    /* Blend operation for V16SFmode */
    vresult = _mm512_mask_blend_ps(mask, vb, va);
    
    _mm512_storeu_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Mixed data types in loop structure ========== */
float test_mixed_blends(int iterations) {
    float total = 0.0f;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* V16SFmode blend */
        {
            float a[16], b[16], r[16];
            for (int i = 0; i < 16; i++) {
                a[i] = (float)(i + iter) * 0.3f;
                b[i] = (float)(i + iter) * 0.7f;
            }
            __m512 va = _mm512_loadu_ps(a);
            __m512 vb = _mm512_loadu_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
            __m512 vr = _mm512_mask_blend_ps(mask, vb, va);
            _mm512_storeu_ps(r, vr);
            
            for (int i = 0; i < 16; i++) {
                total += r[i];
            }
        }
        
        /* V8DFmode blend */
        {
            double a[8], b[8], r[8];
            for (int i = 0; i < 8; i++) {
                a[i] = (double)(i + iter) * 0.5;
                b[i] = (double)(i + iter) * 1.5;
            }
            __m512d va = _mm512_loadu_pd(a);
            __m512d vb = _mm512_loadu_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(2.0), _CMP_GT_OQ);
            __m512d vr = _mm512_mask_blend_pd(mask, vb, va);
            _mm512_storeu_pd(r, vr);
            
            for (int i = 0; i < 8; i++) {
                total += (float)r[i];
            }
        }
    }
    
    return total;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ========== Scalar fallback implementations ========== */
int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 3);
    }
    
    for (int i = 0; i < 64; i++) {
        result[i] = (i % 3 == 0) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

int scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 10);
        b[i] = (short)(i * 15);
    }
    
    for (int i = 0; i < 32; i++) {
        result[i] = (a[i] > 100) ? a[i] : b[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

float scalar_test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 1.5f;
    }
    
    for (int i = 0; i < 16; i++) {
        result[i] = (a[i] > 4.0f) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

double scalar_test_v8df_blend(void) {
    double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.1;
        b[i] = (double)i * 2.2;
    }
    
    for (int i = 0; i < 8; i++) {
        result[i] = (a[i] > 3.0) ? a[i] : b[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ========== Main driver ========== */
int main(void) {
    int total = 0;
    float ftotal = 0.0f;
    double dtotal = 0.0;
    long long lltotal = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using optimized blend operations.\n");
    
    /* Test all vector modes */
    total += test_v64qi_blend();
    total += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    ftotal += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    ftotal += test_v32bf_blend();
#endif
    
    total += test_v16si_blend();
    lltotal += test_v8di_blend();
    dtotal += test_v8df_blend();
    ftotal += test_v16sf_blend();
    
    /* Test mixed blends */
    ftotal += test_mixed_blends(4);
    
#else
    printf("AVX-512BW not available. Using scalar fallback.\n");
#endif
#else
    printf("AVX-512 not available. Using scalar fallback.\n");
#endif

#ifndef __AVX512F__
    /* Scalar fallback path */
    total += scalar_test_v64qi_blend();
    total += scalar_test_v32hi_blend();
    ftotal += scalar_test_v16sf_blend();
    dtotal += scalar_test_v8df_blend();
#endif

    /* Print results to prevent dead code elimination */
    printf("Integer total: %d\n", total);
    printf("Float total: %f\n", ftotal);
    printf("Double total: %f\n", dtotal);
    printf("Long long total: %lld\n", lltotal);
    
    /* Return combined checksum */
    return (int)(total + (int)ftotal + (int)dtotal + (int)lltotal) & 0xFF;
}
