#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Generate dynamic mask using comparison */
    mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Use result in computation to prevent elimination */
    __m512i vsum = _mm512_sad_epu8(vresult, _mm512_setzero_si512());
    uint64_t sum = (uint64_t)_mm512_reduce_add_epi64(vsum);
    
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static uint64_t test_v32hi_blend(void) {
    short a[32], b[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 200);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(500));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Compute sum of absolute differences */
    __m512i vabs = _mm512_abs_epi16(vresult);
    __m512i vsum = _mm512_sad_epu16(vabs, _mm512_setzero_si512());
    uint64_t sum = (uint64_t)_mm512_reduce_add_epi64(vsum);
    
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32], b[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    /* Compare for greater than */
    mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(16.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    /* Horizontal sum */
    __m512h vsum = _mm512_reduce_add_ph(vresult);
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (float)vresult[i];
    }
    
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bfloat16 a[32], b[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)i * 1.5f);
        b[i] = bfloat16_from_float((float)i * 2.5f);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* For bfloat16, we need to use integer comparison since there's no direct bfloat16 compare */
    __m512i vai = _mm512_castsi512_si512(va);
    __m512i vbi = _mm512_castsi512_si512(vb);
    mask = _mm512_cmpgt_epi16_mask(vai, _mm512_set1_epi16(0x4000)); /* ~2.0 in bfloat16 */
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Convert to float and sum */
    float sum = 0;
    __m512 vf = _mm512_cvtne2ps_pbh(vresult, vresult); /* Convert to float */
    for (int i = 0; i < 16; i++) {
        sum += vf[i];
    }
    
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static uint64_t test_v16si_blend(void) {
    int a[16], b[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 2000;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(5000));
    
    /* This should trigger gen_avx512f_blendmv16si */
    vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    /* Sum all elements */
    uint64_t sum = (uint64_t)_mm512_reduce_add_epi32(vresult);
    
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static uint64_t test_v8di_blend(void) {
    long long a[8], b[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 20000LL;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(30000));
    
    /* This should trigger gen_avx512f_blendmv8di */
    vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    /* Sum all elements */
    uint64_t sum = (uint64_t)_mm512_reduce_add_epi64(vresult);
    
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static double test_v8df_blend(void) {
    double a[8], b[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.234;
        b[i] = i * 2.345;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(4.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    /* Horizontal sum */
    double sum = _mm512_reduce_add_pd(vresult);
    
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
static float test_v16sf_blend(void) {
    float a[16], b[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1.5f;
        b[i] = i * 2.5f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(8.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Horizontal sum */
    float sum = _mm512_reduce_add_ps(vresult);
    
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static uint64_t test_mixed_blend_loop(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Test different blend operations in sequence */
        if (iter % 7 == 0) {
            /* V64QImode */
            char a[64], b[64];
            for (int i = 0; i < 64; i++) {
                a[i] = (char)((i + iter) & 0x7F);
                b[i] = (char)((i * 2 + iter) & 0x7F);
            }
            __m512i va = _mm512_loadu_si512((const __m512i*)a);
            __m512i vb = _mm512_loadu_si512((const __m512i*)b);
            __mmask64 mask = _mm512_cmpeq_epi8_mask(va, _mm512_set1_epi8(iter & 0x7F));
            __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
            total_sum += _mm512_reduce_add_epi64(_mm512_sad_epu8(vresult, _mm512_setzero_si512()));
        }
        else if (iter % 7 == 1) {
            /* V32HImode */
            short a[32], b[32];
            for (int i = 0; i < 32; i++) {
                a[i] = (short)((i * 100 + iter) & 0x7FFF);
                b[i] = (short)((i * 200 + iter) & 0x7FFF);
            }
            __m512i va = _mm512_loadu_si512((const __m512i*)a);
            __m512i vb = _mm512_loadu_si512((const __m512i*)b);
            __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(1000));
            __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
            total_sum += _mm512_reduce_add_epi64(_mm512_sad_epu16(vresult, _mm512_setzero_si512()));
        }
        else if (iter % 7 == 2) {
            /* V16SImode */
            int a[16], b[16];
            for (int i = 0; i < 16; i++) {
                a[i] = i * 1000 + iter;
                b[i] = i * 2000 + iter;
            }
            __m512i va = _mm512_loadu_si512((const __m512i*)a);
            __m512i vb = _mm512_loadu_si512((const __m512i*)b);
            __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(5000));
            __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
            total_sum += _mm512_reduce_add_epi32(vresult);
        }
        else if (iter % 7 == 3) {
            /* V8DImode */
            long long a[8], b[8];
            for (int i = 0; i < 8; i++) {
                a[i] = i * 10000LL + iter;
                b[i] = i * 20000LL + iter;
            }
            __m512i va = _mm512_loadu_si512((const __m512i*)a);
            __m512i vb = _mm512_loadu_si512((const __m512i*)b);
            __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(30000));
            __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
            total_sum += _mm512_reduce_add_epi64(vresult);
        }
        else if (iter % 7 == 4) {
            /* V16SFmode */
            float a[16], b[16];
            for (int i = 0; i < 16; i++) {
                a[i] = i * 1.5f + iter * 0.1f;
                b[i] = i * 2.5f + iter * 0.1f;
            }
            __m512 va = _mm512_loadu_ps(a);
            __m512 vb = _mm512_loadu_ps(b);
            __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(8.0f), _CMP_GT_OQ);
            __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
            total_sum += (uint64_t)_mm512_reduce_add_ps(vresult);
        }
        else if (iter % 7 == 5) {
            /* V8DFmode */
            double a[8], b[8];
            for (int i = 0; i < 8; i++) {
                a[i] = i * 1.234 + iter * 0.01;
                b[i] = i * 2.345 + iter * 0.01;
            }
            __m512d va = _mm512_loadu_pd(a);
            __m512d vb = _mm512_loadu_pd(b);
            __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(4.0), _CMP_GT_OQ);
            __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
            total_sum += (uint64_t)_mm512_reduce_add_pd(vresult);
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallbacks ==================== */
static uint64_t scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    uint64_t mask = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
        /* Simulate mask generation */
        if (a[i] > 32) mask |= (1ULL << i);
    }
    
    for (int i = 0; i < 64; i++) {
        result[i] = (mask & (1ULL << i)) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (unsigned char)result[i];
    }
    
    return sum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    uint32_t mask = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 200);
        if (a[i] > 500) mask |= (1U << i);
    }
    
    for (int i = 0; i < 32; i++) {
        result[i] = (mask & (1U << i)) ? b[i] : a[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (unsigned short)result[i];
    }
    
    return sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    uint64_t total_checksum = 0;
    
    printf("AVX-512 Blend Instruction Coverage Test\n");
    printf("=======================================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected. Using vectorized implementation.\n");
    
    /* Run all blend tests */
    total_checksum += test_v64qi_blend();
    printf("  V64QImode blend test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("  V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += (uint64_t)test_v32hf_blend();
    printf("  V32HFmode blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (uint64_t)test_v32bf_blend();
    printf("  V32BFmode blend test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("  V16SImode blend test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("  V8DImode blend test completed\n");
    
    total_checksum += (uint64_t)test_v8df_blend();
    printf("  V8DFmode blend test completed\n");
    
    total_checksum += (uint64_t)test_v16sf_blend();
    printf("  V16SFmode blend test completed\n");
    
    /* Mixed loop test */
    total_checksum += test_mixed_blend_loop(100);
    printf("  Mixed data type loop test completed\n");
    
#else
    printf("AVX-512BW not detected. Using scalar fallback.\n");
#endif
#else
    printf("AVX-512F not detected. Using scalar fallback.\n");
#endif

#ifndef __AVX512F__
    /* Scalar fallback path */
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    printf("  Scalar fallback tests completed\n");
#endif
    
    printf("\nTotal checksum: %lu\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
