#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== V64QImode (64-byte integers) ==================== */
static int test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    __m512i va, vb, vresult;
    __mmask64 mask;
    
    /* Initialize with alternating pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 2 + 1);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Create dynamic mask: elements where a[i] > 32 */
    mask = _mm512_cmp_epi8_mask(va, _mm512_set1_epi8(32), _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HImode (32-halfword integers) ==================== */
static int test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    __m512i va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 100 + 50);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask based on comparison */
    mask = _mm512_cmp_epi16_mask(va, _mm512_set1_epi16(1600), _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V32HFmode (32-half-precision floats) ==================== */
#ifdef __AVX512FP16__
static float test_v32hf_blend(void) {
    _Float16 a[32], b[32], result[32];
    __m512h va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.5f + 0.25f);
    }
    
    va = _mm512_loadu_ph(a);
    vb = _mm512_loadu_ph(b);
    
    /* Compare for mask generation */
    mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(result, vresult);
    
    /* Compute sum as float */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

/* ==================== V32BFmode (32-bfloat16) ==================== */
#ifdef __AVX512BF16__
static float test_v32bf_blend(void) {
    __bf16 a[32], b[32], result[32];
    __m512bh va, vb, vresult;
    __mmask32 mask;
    
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)i * 0.5f);
        b[i] = bfloat16_from_float((float)i * 0.5f + 0.25f);
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* For bfloat16, we need to use integer comparison since there's no direct bfloat16 compare */
    __m512i vai = _mm512_castsi512_si512(va);
    __m512i vbi = _mm512_castsi512_si512(vb);
    mask = _mm512_cmp_epi16_mask(vai, _mm512_set1_epi16(0x4000), _MM_CMPINT_GT);
    
    /* Blend using epi16 intrinsic for bfloat16 */
    vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(result[i]);
    }
    return sum;
}
#endif

/* ==================== V16SImode (16-dword integers) ==================== */
static int test_v16si_blend(void) {
    int a[16], b[16], result[16];
    __m512i va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 1000 + 500;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmp_epi32_mask(va, _mm512_set1_epi32(8000), _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512f_blendmv16si */
    vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DImode (8-qword integers) ==================== */
static long long test_v8di_blend(void) {
    long long a[8], b[8], result[8];
    __m512i va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 10000LL;
        b[i] = (long long)i * 10000LL + 5000LL;
    }
    
    va = _mm512_loadu_si512((const __m512i*)a);
    vb = _mm512_loadu_si512((const __m512i*)b);
    
    mask = _mm512_cmp_epi64_mask(va, _mm512_set1_epi64(40000LL), _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512f_blendmv8di */
    vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)result, vresult);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V8DFmode (8-double-precision floats) ==================== */
static double test_v8df_blend(void) {
    double a[8], b[8], result[8];
    __m512d va, vb, vresult;
    __mmask8 mask;
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)i * 1.5 + 0.75;
    }
    
    va = _mm512_loadu_pd(a);
    vb = _mm512_loadu_pd(b);
    
    mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(result, vresult);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== V16SFmode (16-single-precision floats) ==================== */
static float test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    __m512 va, vb, vresult;
    __mmask16 mask;
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.8f;
        b[i] = (float)i * 0.8f + 0.4f;
    }
    
    va = _mm512_loadu_ps(a);
    vb = _mm512_loadu_ps(b);
    
    mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(6.4f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(result, vresult);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Mixed Data Types in Loop ==================== */
static long long test_mixed_blends(void) {
    const int N = 1024;
    long long total_sum = 0;
    
    /* Test different data types in loops */
    {
        /* V16SF mode */
        float fa[N], fb[N], fr[N];
        for (int i = 0; i < N; i++) {
            fa[i] = (float)(i % 32) * 0.1f;
            fb[i] = (float)(i % 32) * 0.1f + 0.05f;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 va = _mm512_loadu_ps(&fa[i]);
            __m512 vb = _mm512_loadu_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(1.5f), _CMP_GT_OQ);
            __m512 vr = _mm512_mask_blend_ps(mask, va, vb);
            _mm512_storeu_ps(&fr[i], vr);
            
            for (int j = 0; j < 16 && (i + j) < N; j++) {
                total_sum += (long long)(fr[i + j] * 1000.0f);
            }
        }
    }
    
    {
        /* V8DF mode */
        double da[N], db[N], dr[N];
        for (int i = 0; i < N; i++) {
            da[i] = (double)(i % 16) * 0.2;
            db[i] = (double)(i % 16) * 0.2 + 0.1;
        }
        
        for (int i = 0; i < N; i += 8) {
            __m512d va = _mm512_loadu_pd(&da[i]);
            __m512d vb = _mm512_loadu_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(1.5), _CMP_GT_OQ);
            __m512d vr = _mm512_mask_blend_pd(mask, va, vb);
            _mm512_storeu_pd(&dr[i], vr);
            
            for (int j = 0; j < 8 && (i + j) < N; j++) {
                total_sum += (long long)(dr[i + j] * 1000.0);
            }
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallbacks ==================== */
static int scalar_test_v64qi_blend(void) {
    char a[64], b[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 2);
        b[i] = (char)(i * 2 + 1);
        result[i] = (a[i] > 32) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}

static int scalar_test_v32hi_blend(void) {
    short a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)(i * 100 + 50);
        result[i] = (a[i] > 1600) ? b[i] : a[i];
    }
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

/* ==================== Main Function ==================== */
int main(void) {
    long long total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    /* Run all vector mode tests */
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += (long long)test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (long long)test_v32bf_blend();
#endif
    
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += (long long)test_v8df_blend();
    total_checksum += (long long)test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
    printf("Vector tests completed. Total checksum: %lld\n", total_checksum);
    
#else
    printf("AVX-512BW not available. Running scalar tests...\n");
#endif
#else
    printf("AVX-512 not available. Running scalar tests...\n");
#endif

#ifndef __AVX512F__
    /* Fallback to scalar implementations */
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    printf("Scalar tests completed. Total checksum: %lld\n", total_checksum);
#endif

    return (int)(total_checksum % 1000);
}
