#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <math.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // This should trigger gen_avx512bw_blendmv64qi
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    // Use result in computation to prevent elimination
    char res[64];
    _mm512_storeu_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)res[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // This should trigger gen_avx512bw_blendmv32hi
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    short res[32];
    _mm512_storeu_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)res[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
    _Float16 a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // This should trigger gen_avx512bw_blendmv32hf
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 res[32];
    _mm512_storeu_ph(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode: 32-bfloat16 */
static float test_v32bf_blend(void) {
    __bf16 a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = bfloat16_from_float((float)(i * 1.2f));
        b[i] = bfloat16_from_float((float)(i * 2.2f));
    }
    
    __m512bh va = _mm512_loadu_si512((__m512i*)a);
    __m512bh vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate mask - use integer comparison since bfloat16 doesn't have direct comparison
    __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
    
    // This should trigger gen_avx512bw_blendmv32bf
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)va, (__m512i)vb);
    
    __bf16 res[32];
    _mm512_storeu_si512((__m512i*)res, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(res[i]);
    }
    return sum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // This should trigger gen_avx512f_blendmv16si
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    int res[16];
    _mm512_storeu_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)res[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_loadu_si512((__m512i*)a);
    __m512i vb = _mm512_loadu_si512((__m512i*)b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_NE);
    
    // This should trigger gen_avx512f_blendmv8di
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long res[8];
    _mm512_storeu_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)res[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
    double a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    // Generate dynamic mask
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // This should trigger gen_avx512f_blendmv8df
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    double res[8];
    _mm512_storeu_pd(res, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = i * 1.3f;
    }
    
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GE_OQ);
    
    // This should trigger gen_avx512f_blendmv16sf
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    float res[16];
    _mm512_storeu_ps(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    // Process arrays with different data types
    for (int iter = 0; iter < 10; iter++) {
        // Float array processing (V16SFmode)
        float fa[16], fb[16];
        for (int i = 0; i < 16; i++) {
            fa[i] = (float)((i + iter) * 1.1f);
            fb[i] = (float)((i + iter) * 2.2f);
        }
        __m512 fva = _mm512_loadu_ps(fa);
        __m512 fvb = _mm512_loadu_ps(fb);
        __mmask16 fmask = _mm512_cmp_ps_mask(fva, fvb, _CMP_GT_OQ);
        __m512 fresult = _mm512_mask_blend_ps(fmask, fva, fvb);
        float fres[16];
        _mm512_storeu_ps(fres, fresult);
        for (int i = 0; i < 16; i++) total_sum += (uint64_t)fres[i];
        
        // Double array processing (V8DFmode)
        double da[8], db[8];
        for (int i = 0; i < 8; i++) {
            da[i] = (double)((i + iter) * 1.5);
            db[i] = (double)((i + iter) * 3.0);
        }
        __m512d dva = _mm512_loadu_pd(da);
        __m512d dvb = _mm512_loadu_pd(db);
        __mmask8 dmask = _mm512_cmp_pd_mask(dva, dvb, _CMP_LT_OQ);
        __m512d dresult = _mm512_mask_blend_pd(dmask, dva, dvb);
        double dres[8];
        _mm512_storeu_pd(dres, dresult);
        for (int i = 0; i < 8; i++) total_sum += (uint64_t)dres[i];
        
        // Integer array processing (V16SImode)
        int ia[16], ib[16];
        for (int i = 0; i < 16; i++) {
            ia[i] = (i + iter) * 3;
            ib[i] = (i + iter) * 5;
        }
        __m512i iva = _mm512_loadu_si512((__m512i*)ia);
        __m512i ivb = _mm512_loadu_si512((__m512i*)ib);
        __mmask16 imask = _mm512_cmp_epi32_mask(iva, ivb, _MM_CMPINT_EQ);
        __m512i iresult = _mm512_mask_blend_epi32(imask, iva, ivb);
        int ires[16];
        _mm512_storeu_si512((__m512i*)ires, iresult);
        for (int i = 0; i < 16; i++) total_sum += ires[i];
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t scalar_test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    char res[64];
    for (int i = 0; i < 64; i++) {
        res[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)res[i];
    }
    return sum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32], b[32];
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    short res[32];
    for (int i = 0; i < 32; i++) {
        res[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)res[i];
    }
    return sum;
}

static float scalar_test_v16sf_blend(void) {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = i * 1.3f;
    }
    
    float res[16];
    for (int i = 0; i < 16; i++) {
        res[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

static double scalar_test_v8df_blend(void) {
    double a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    double res[8];
    for (int i = 0; i < 8; i++) {
        res[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

static uint64_t scalar_test_v16si_blend(void) {
    int a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    int res[16];
    for (int i = 0; i < 16; i++) {
        res[i] = (a[i] == b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)res[i];
    }
    return sum;
}

static uint64_t scalar_test_v8di_blend(void) {
    long long a[8], b[8];
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    long long res[8];
    for (int i = 0; i < 8; i++) {
        res[i] = (a[i] != b[i]) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)res[i];
    }
    return sum;
}

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using vectorized blend operations.\n");
    
    total_checksum += test_v64qi_blend();
    printf("V64QImode blend checksum added\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode blend checksum added\n");
    
#ifdef __AVX512FP16__
    total_checksum += (uint64_t)test_v32hf_blend();
    printf("V32HFmode blend checksum added\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += (uint64_t)test_v32bf_blend();
    printf("V32BFmode blend checksum added\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode blend checksum added\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode blend checksum added\n");
    
    total_checksum += (uint64_t)test_v8df_blend();
    printf("V8DFmode blend checksum added\n");
    
    total_checksum += (uint64_t)test_v16sf_blend();
    printf("V16SFmode blend checksum added\n");
    
    total_checksum += test_mixed_blends();
    printf("Mixed blends checksum added\n");
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks.\n");
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks.\n");
#endif

#ifndef __AVX512F__
    // Use scalar fallbacks
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += (uint64_t)scalar_test_v16sf_blend();
    total_checksum += (uint64_t)scalar_test_v8df_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
#endif

    printf("Final aggregate checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
