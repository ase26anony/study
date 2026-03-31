#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    alignas(64) int8_t a[64], b[64], result[64];
    
    /* Initialize with alternating patterns */
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 2);
        b[i] = (int8_t)(i * 3 + 1);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmpgt_epi8_mask(va, _mm512_set1_epi8(32));
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Compute checksum to prevent optimization */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)result[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    alignas(64) int16_t a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 150 + 50);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask using greater-than comparison */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(1600));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)result[i];
    }
    return sum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    alignas(64) _Float16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)(i * 0.75f + 0.25f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Generate mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(va, _mm512_set1_ph(8.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h vresult = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        /* Convert to integer for checksum */
        sum += (uint16_t)(result[i] * 1000);
    }
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    alignas(64) __bfloat16 a[32], b[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        /* Create simple bfloat16 values */
        uint16_t val_a = (i * 0x40) & 0x7FFF;
        uint16_t val_b = ((i * 0x60) + 0x20) & 0x7FFF;
        a[i] = *(__bfloat16*)&val_a;
        b[i] = *(__bfloat16*)&val_b;
    }
    
    __m512bh va = _mm512_load_si512((__m512i*)a);
    __m512bh vb = _mm512_load_si512((__m512i*)b);
    
    /* For bfloat16, we need to use integer blend since there's no direct bfloat16 blend */
    __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh vresult = _mm512_mask_blend_epi16(mask, 
        (__m512i)va, (__m512i)vb);
    
    _mm512_store_si512((__m512i*)result, (__m512i)vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += *(uint16_t*)&result[i];
    }
    return sum;
}
#endif

#endif /* __AVX512BW__ */

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    alignas(64) int32_t a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 1500 + 500;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask using comparison */
    __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(8000));
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)result[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    alignas(64) int64_t a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 15000LL + 5000LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate mask using comparison */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(va, _mm512_set1_epi64(30000));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_si512((__m512i*)result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)result[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    alignas(64) double a[8], b[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.25 + 0.75;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Generate mask using comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(6.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_store_pd(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    alignas(64) float a[16], b[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.75f;
        b[i] = i * 1.125f + 0.375f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Generate mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(6.0f), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_store_ps(result, vresult);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    alignas(64) float fa[N], fb[N], fresult[N];
    alignas(64) double da[N/2], db[N/2], dresult[N/2];
    alignas(64) int32_t ia[N], ib[N], iresult[N];
    alignas(64) int16_t sa[N*2], sb[N*2], sresult[N*2];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        fa[i] = (i % 32) * 0.1f;
        fb[i] = (i % 32) * 0.15f + 0.05f;
        ia[i] = (i % 32) * 10;
        ib[i] = (i % 32) * 15 + 5;
    }
    for (int i = 0; i < N/2; i++) {
        da[i] = (i % 16) * 0.2;
        db[i] = (i % 16) * 0.3 + 0.1;
    }
    for (int i = 0; i < N*2; i++) {
        sa[i] = (i % 64) * 2;
        sb[i] = (i % 64) * 3 + 1;
    }
    
    uint64_t total_sum = 0;
    
    /* Process in chunks of vector size */
    for (int i = 0; i < N; i += 16) {
        __m512 va = _mm512_load_ps(&fa[i]);
        __m512 vb = _mm512_load_ps(&fb[i]);
        __mmask16 mask = _mm512_cmp_ps_mask(va, _mm512_set1_ps(1.5f), _CMP_GT_OQ);
        __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
        _mm512_store_ps(&fresult[i], vresult);
        
        /* Use result in computation */
        for (int j = 0; j < 16 && i+j < N; j++) {
            total_sum += (uint64_t)(fresult[i+j] * 100);
        }
    }
    
    for (int i = 0; i < N/2; i += 8) {
        __m512d va = _mm512_load_pd(&da[i]);
        __m512d vb = _mm512_load_pd(&db[i]);
        __mmask8 mask = _mm512_cmp_pd_mask(va, _mm512_set1_pd(1.5), _CMP_GT_OQ);
        __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
        _mm512_store_pd(&dresult[i], vresult);
        
        for (int j = 0; j < 8 && i+j < N/2; j++) {
            total_sum += (uint64_t)(dresult[i+j] * 100);
        }
    }
    
#ifdef __AVX512BW__
    for (int i = 0; i < N*2; i += 32) {
        __m512i va = _mm512_load_si512((__m512i*)&sa[i]);
        __m512i vb = _mm512_load_si512((__m512i*)&sb[i]);
        __mmask32 mask = _mm512_cmpgt_epi16_mask(va, _mm512_set1_epi16(32));
        __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
        _mm512_store_si512((__m512i*)&sresult[i], vresult);
        
        for (int j = 0; j < 32 && i+j < N*2; j++) {
            total_sum += sresult[i+j];
        }
    }
#endif
    
    for (int i = 0; i < N; i += 16) {
        __m512i va = _mm512_load_si512((__m512i*)&ia[i]);
        __m512i vb = _mm512_load_si512((__m512i*)&ib[i]);
        __mmask16 mask = _mm512_cmpgt_epi32_mask(va, _mm512_set1_epi32(160));
        __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
        _mm512_store_si512((__m512i*)&iresult[i], vresult);
        
        for (int j = 0; j < 16 && i+j < N; j++) {
            total_sum += iresult[i+j];
        }
    }
    
    return total_sum;
}

#else /* AVX-512 not available */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend(void) {
    int8_t a[64], b[64], result[64];
    uint64_t sum = 0;
    
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 2);
        b[i] = (int8_t)(i * 3 + 1);
        result[i] = (a[i] > 32) ? b[i] : a[i];
        sum += (uint8_t)result[i];
    }
    return sum;
}

static uint64_t test_v32hi_blend(void) {
    int16_t a[32], b[32], result[32];
    uint64_t sum = 0;
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 100);
        b[i] = (int16_t)(i * 150 + 50);
        result[i] = (a[i] > 1600) ? b[i] : a[i];
        sum += (uint16_t)result[i];
    }
    return sum;
}

static uint64_t test_v16si_blend(void) {
    int32_t a[16], b[16], result[16];
    uint64_t sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 1000;
        b[i] = i * 1500 + 500;
        result[i] = (a[i] > 8000) ? b[i] : a[i];
        sum += (uint32_t)result[i];
    }
    return sum;
}

static uint64_t test_v8di_blend(void) {
    int64_t a[8], b[8], result[8];
    uint64_t sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 10000LL;
        b[i] = i * 15000LL + 5000LL;
        result[i] = (a[i] > 30000) ? b[i] : a[i];
        sum += (uint64_t)result[i];
    }
    return sum;
}

static uint64_t test_v8df_blend(void) {
    double a[8], b[8], result[8];
    uint64_t sum = 0;
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.25 + 0.75;
        result[i] = (a[i] > 6.0) ? b[i] : a[i];
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

static uint64_t test_v16sf_blend(void) {
    float a[16], b[16], result[16];
    uint64_t sum = 0;
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.75f;
        b[i] = i * 1.125f + 0.375f;
        result[i] = (a[i] > 6.0f) ? b[i] : a[i];
        sum += (uint64_t)(result[i] * 1000);
    }
    return sum;
}

static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    float fa[N], fb[N], fresult[N];
    double da[N/2], db[N/2], dresult[N/2];
    int32_t ia[N], ib[N], iresult[N];
    int16_t sa[N*2], sb[N*2], sresult[N*2];
    uint64_t total_sum = 0;
    
    for (int i = 0; i < N; i++) {
        fa[i] = (i % 32) * 0.1f;
        fb[i] = (i % 32) * 0.15f + 0.05f;
        fresult[i] = (fa[i] > 1.5f) ? fb[i] : fa[i];
        total_sum += (uint64_t)(fresult[i] * 100);
        
        ia[i] = (i % 32) * 10;
        ib[i] = (i % 32) * 15 + 5;
        iresult[i] = (ia[i] > 160) ? ib[i] : ia[i];
        total_sum += iresult[i];
    }
    
    for (int i = 0; i < N/2; i++) {
        da[i] = (i % 16) * 0.2;
        db[i] = (i % 16) * 0.3 + 0.1;
        dresult[i] = (da[i] > 1.5) ? db[i] : da[i];
        total_sum += (uint64_t)(dresult[i] * 100);
    }
    
    for (int i = 0; i < N*2; i++) {
        sa[i] = (i % 64) * 2;
        sb[i] = (i % 64) * 3 + 1;
        sresult[i] = (sa[i] > 32) ? sb[i] : sa[i];
        total_sum += sresult[i];
    }
    
    return total_sum;
}

#endif /* __AVX512F__ */

int main(void) {
    uint64_t total_checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
#endif
#ifdef __AVX512FP16__
    printf("AVX-512-FP16 supported\n");
#endif
#ifdef __AVX512BF16__
    printf("AVX-512-BF16 supported\n");
#endif
    
    /* Run all blend tests */
    total_checksum += test_v16sf_blend();
    printf("  V16SF blend test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("  V8DF blend test completed\n");
    
    total_checksum += test_v16si_blend();
    printf("  V16SI blend test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("  V8DI blend test completed\n");
    
#ifdef __AVX512BW__
    total_checksum += test_v64qi_blend();
    printf("  V64QI blend test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("  V32HI blend test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("  V32HF blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("  V32BF blend test completed\n");
#endif
#endif /* __AVX512BW__ */
    
    total_checksum += test_mixed_blends();
    printf("  Mixed blends test completed\n");
    
#else
    printf("AVX-512 not supported, using scalar fallback\n");
    
    total_checksum += test_v16sf_blend();
    total_checksum += test_v8df_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    total_checksum += test_mixed_blends();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    
    /* Return non-zero checksum to ensure code isn't optimized away */
    return (int)(total_checksum & 0x7FFFFFFF);
}
