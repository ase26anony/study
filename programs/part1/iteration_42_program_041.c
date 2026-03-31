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
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    char result[64] __attribute__((aligned(64)));
    
    // Initialize with alternating patterns
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)((i + 64) % 128);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate dynamic mask using comparison
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    // Blend based on mask - this should trigger gen_avx512bw_blendmv64qi
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    // Compute checksum to prevent optimization
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)result[i];
    }
    return checksum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    short result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)((i + 16) * 150);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    // Blend - should trigger gen_avx512bw_blendmv32hi
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)result[i];
    }
    return checksum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    _Float16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)((i + 8) * 2.0f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    // Generate mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    // Blend - should trigger gen_avx512bw_blendmv32hf
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)(result[i] * 1000);
    }
    return checksum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bf16 a[32] __attribute__((aligned(64)));
    __bf16 b[32] __attribute__((aligned(64)));
    __bf16 result[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (__bf16)(i * 1.2f);
        b[i] = (__bf16)((i + 16) * 1.8f);
    }
    
    // Load as epi16 for bfloat16 emulation
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask - use integer comparison since bfloat16 doesn't have direct comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_NE);
    
    // Blend using epi16 - should trigger gen_avx512bw_blendmv32bf
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)result[i];
    }
    return checksum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    int result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = (i + 8) * 2000;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    // Blend - should trigger gen_avx512f_blendmv16si
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)result[i];
    }
    return checksum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    long long result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = (long long)i * 10000LL;
        b[i] = (long long)(i + 4) * 30000LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GT);
    
    // Blend - should trigger gen_avx512f_blendmv8di
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)result[i];
    }
    return checksum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    double result[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.25;
        b[i] = (double)(i + 2) * 2.75;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    // Generate mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
    
    // Blend - should trigger gen_avx512f_blendmv8df
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)(result[i] * 1000.0);
    }
    return checksum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    float result[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)(i + 4) * 1.5f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    // Generate mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_NEQ_OQ);
    
    // Blend - should trigger gen_avx512f_blendmv16sf
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)(result[i] * 1000.0f);
    }
    return checksum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_checksum = 0;
    
    // Process arrays with different data types
    for (int iter = 0; iter < 10; iter++) {
        // Float array processing
        float fa[16] __attribute__((aligned(64)));
        float fb[16] __attribute__((aligned(64)));
        for (int i = 0; i < 16; i++) {
            fa[i] = (float)(i + iter) * 0.7f;
            fb[i] = (float)(i * 2 + iter) * 1.3f;
        }
        
        __m512 fva = _mm512_load_ps(fa);
        __m512 fvb = _mm512_load_ps(fb);
        __mmask16 fmask = _mm512_cmp_ps_mask(fva, fvb, _CMP_GT_OQ);
        __m512 fresult = _mm512_mask_blend_ps(fmask, fva, fvb);
        _mm512_store_ps(fa, fresult);
        
        for (int i = 0; i < 16; i++) {
            total_checksum += (uint32_t)(fa[i] * 100.0f);
        }
        
        // Double array processing
        double da[8] __attribute__((aligned(64)));
        double db[8] __attribute__((aligned(64)));
        for (int i = 0; i < 8; i++) {
            da[i] = (double)(i + iter) * 0.9;
            db[i] = (double)(i * 3 + iter) * 2.1;
        }
        
        __m512d dva = _mm512_load_pd(da);
        __m512d dvb = _mm512_load_pd(db);
        __mmask8 dmask = _mm512_cmp_pd_mask(dva, dvb, _CMP_LT_OQ);
        __m512d dresult = _mm512_mask_blend_pd(dmask, dva, dvb);
        _mm512_store_pd(da, dresult);
        
        for (int i = 0; i < 8; i++) {
            total_checksum += (uint64_t)(da[i] * 100.0);
        }
        
        // Integer array processing
        int ia[16] __attribute__((aligned(64)));
        int ib[16] __attribute__((aligned(64)));
        for (int i = 0; i < 16; i++) {
            ia[i] = (i + iter) * 100;
            ib[i] = (i * 2 + iter) * 200;
        }
        
        __m512i iva = _mm512_load_si512((__m512i*)ia);
        __m512i ivb = _mm512_load_si512((__m512i*)ib);
        __mmask16 imask = _mm512_cmp_epi32_mask(iva, ivb, _MM_CMPINT_EQ);
        __m512i iresult = _mm512_mask_blend_epi32(imask, iva, ivb);
        _mm512_store_si512((__m512i*)ia, iresult);
        
        for (int i = 0; i < 16; i++) {
            total_checksum += (uint32_t)ia[i];
        }
    }
    
    return total_checksum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend_scalar(void) {
    char a[64];
    char b[64];
    char result[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)((i + 64) % 128);
        result[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)result[i];
    }
    return checksum;
}

static uint64_t test_v32hi_blend_scalar(void) {
    short a[32];
    short b[32];
    short result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 100);
        b[i] = (short)((i + 16) * 150);
        result[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)result[i];
    }
    return checksum;
}

/* Main driver function */
int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Using vectorized implementations.\n");
    
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
#endif
    
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += test_v8df_blend();
    total_checksum += test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
#else
    printf("AVX-512BW not available. Using scalar implementations.\n");
    total_checksum += test_v64qi_blend_scalar();
    total_checksum += test_v32hi_blend_scalar();
#endif
#else
    printf("AVX-512 not available. Using scalar implementations.\n");
    total_checksum += test_v64qi_blend_scalar();
    total_checksum += test_v32hi_blend_scalar();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
