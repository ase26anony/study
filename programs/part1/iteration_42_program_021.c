#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* ==================== AVX-512 Implementation ==================== */

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    /* Patterned data for predictable masks */
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Use result to prevent elimination */
    char res[64] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)res[i];
    }
    return checksum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    short res[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)res[i];
    }
    return checksum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 res[32] __attribute__((aligned(64)));
    _mm512_store_ph(res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)*(uint16_t*)&res[i];
    }
    return checksum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    __bfloat16 a[32] __attribute__((aligned(64)));
    __bfloat16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        float temp = i * 1.7f;
        a[i] = _mm_cvtness_sbh(temp);
        b[i] = _mm_cvtness_sbh(temp * 1.3f);
    }
    
    __m512bh va = _mm512_load_si512((__m512i*)a);
    __m512bh vb = _mm512_load_si512((__m512i*)b);
    
    /* For bfloat16, we need to use integer comparison since there's no direct bfloat16 compare */
    __m512i va_int = _mm512_load_si512((__m512i*)a);
    __m512i vb_int = _mm512_load_si512((__m512i*)b);
    __mmask32 mask = _mm512_cmp_epi16_mask(va_int, vb_int, _MM_CMPINT_NE);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_epi16(mask, va, vb);
    
    __bfloat16 res[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, (__m512i)result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)*(uint16_t*)&res[i];
    }
    return checksum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    int res[16] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)res[i];
    }
    return checksum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_load_si512((__m512i*)a);
    __m512i vb = _mm512_load_si512((__m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long res[8] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)res[i];
    }
    return checksum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    double a[8] __attribute__((aligned(64)));
    double b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    double res[8] __attribute__((aligned(64)));
    _mm512_store_pd(res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)*(uint64_t*)&res[i];
    }
    return checksum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = i * 1.3f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LE_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    float res[16] __attribute__((aligned(64)));
    _mm512_store_ps(res, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)*(uint32_t*)&res[i];
    }
    return checksum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blend_loop(void) {
    const int N = 1024;
    uint64_t total_checksum = 0;
    
    /* Process different data types in separate loops */
    {
        /* V16SFmode loop */
        float fa[N] __attribute__((aligned(64)));
        float fb[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            fa[i] = (float)(i % 32);
            fb[i] = (float)((i * 3) % 32);
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512 va = _mm512_load_ps(&fa[i]);
            __m512 vb = _mm512_load_ps(&fb[i]);
            __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
            __m512 result = _mm512_mask_blend_ps(mask, va, vb);
            
            float res[16] __attribute__((aligned(64)));
            _mm512_store_ps(res, result);
            
            for (int j = 0; j < 16; j++) {
                total_checksum += (uint32_t)*(uint32_t*)&res[j];
            }
        }
    }
    
    {
        /* V8DFmode loop */
        double da[N] __attribute__((aligned(64)));
        double db[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            da[i] = (double)(i % 16);
            db[i] = (double)((i * 5) % 16);
        }
        
        for (int i = 0; i < N; i += 8) {
            __m512d va = _mm512_load_pd(&da[i]);
            __m512d vb = _mm512_load_pd(&db[i]);
            __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_LT_OQ);
            __m512d result = _mm512_mask_blend_pd(mask, va, vb);
            
            double res[8] __attribute__((aligned(64)));
            _mm512_store_pd(res, result);
            
            for (int j = 0; j < 8; j++) {
                total_checksum += (uint64_t)*(uint64_t*)&res[j];
            }
        }
    }
    
    {
        /* V16SImode loop */
        int ia[N] __attribute__((aligned(64)));
        int ib[N] __attribute__((aligned(64)));
        
        for (int i = 0; i < N; i++) {
            ia[i] = i % 64;
            ib[i] = (i * 7) % 64;
        }
        
        for (int i = 0; i < N; i += 16) {
            __m512i va = _mm512_load_si512((__m512i*)&ia[i]);
            __m512i vb = _mm512_load_si512((__m512i*)&ib[i]);
            __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_EQ);
            __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
            
            int res[16] __attribute__((aligned(64)));
            _mm512_store_si512((__m512i*)res, result);
            
            for (int j = 0; j < 16; j++) {
                total_checksum += (uint32_t)res[j];
            }
        }
    }
    
    return total_checksum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* ==================== Scalar Fallback Implementation ==================== */

static uint64_t scalar_test_v64qi_blend(void) {
    char a[64];
    char b[64];
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    uint64_t mask = 0;
    for (int i = 0; i < 64; i++) {
        if (a[i] > b[i]) {
            mask |= (1ULL << i);
        }
    }
    
    char res[64];
    for (int i = 0; i < 64; i++) {
        res[i] = (mask & (1ULL << i)) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += (uint8_t)res[i];
    }
    return checksum;
}

static uint64_t scalar_test_v32hi_blend(void) {
    short a[32];
    short b[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    uint32_t mask = 0;
    for (int i = 0; i < 32; i++) {
        if (a[i] < b[i]) {
            mask |= (1U << i);
        }
    }
    
    short res[32];
    for (int i = 0; i < 32; i++) {
        res[i] = (mask & (1U << i)) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)res[i];
    }
    return checksum;
}

static uint64_t scalar_test_v16si_blend(void) {
    int a[16];
    int b[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    uint16_t mask = 0;
    for (int i = 0; i < 16; i++) {
        if (a[i] == b[i]) {
            mask |= (1U << i);
        }
    }
    
    int res[16];
    for (int i = 0; i < 16; i++) {
        res[i] = (mask & (1U << i)) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)res[i];
    }
    return checksum;
}

static uint64_t scalar_test_v8di_blend(void) {
    long long a[8];
    long long b[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    uint8_t mask = 0;
    for (int i = 0; i < 8; i++) {
        if (a[i] >= b[i]) {
            mask |= (1U << i);
        }
    }
    
    long long res[8];
    for (int i = 0; i < 8; i++) {
        res[i] = (mask & (1U << i)) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)res[i];
    }
    return checksum;
}

static uint64_t scalar_test_v8df_blend(void) {
    double a[8];
    double b[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 1.1;
        b[i] = i * 2.2;
    }
    
    uint8_t mask = 0;
    for (int i = 0; i < 8; i++) {
        if (a[i] > b[i]) {
            mask |= (1U << i);
        }
    }
    
    double res[8];
    for (int i = 0; i < 8; i++) {
        res[i] = (mask & (1U << i)) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)*(uint64_t*)&res[i];
    }
    return checksum;
}

static uint64_t scalar_test_v16sf_blend(void) {
    float a[16];
    float b[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.7f;
        b[i] = i * 1.3f;
    }
    
    uint16_t mask = 0;
    for (int i = 0; i < 16; i++) {
        if (a[i] <= b[i]) {
            mask |= (1U << i);
        }
    }
    
    float res[16];
    for (int i = 0; i < 16; i++) {
        res[i] = (mask & (1U << i)) ? a[i] : b[i];
    }
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)*(uint32_t*)&res[i];
    }
    return checksum;
}

static uint64_t scalar_test_mixed_blend_loop(void) {
    const int N = 1024;
    uint64_t total_checksum = 0;
    
    /* Process different data types in separate loops */
    {
        float fa[N];
        float fb[N];
        
        for (int i = 0; i < N; i++) {
            fa[i] = (float)(i % 32);
            fb[i] = (float)((i * 3) % 32);
        }
        
        for (int i = 0; i < N; i++) {
            float result = (fa[i] > fb[i]) ? fa[i] : fb[i];
            total_checksum += (uint32_t)*(uint32_t*)&result;
        }
    }
    
    {
        double da[N];
        double db[N];
        
        for (int i = 0; i < N; i++) {
            da[i] = (double)(i % 16);
            db[i] = (double)((i * 5) % 16);
        }
        
        for (int i = 0; i < N; i++) {
            double result = (da[i] < db[i]) ? da[i] : db[i];
            total_checksum += (uint64_t)*(uint64_t*)&result;
        }
    }
    
    {
        int ia[N];
        int ib[N];
        
        for (int i = 0; i < N; i++) {
            ia[i] = i % 64;
            ib[i] = (i * 7) % 64;
        }
        
        for (int i = 0; i < N; i++) {
            int result = (ia[i] == ib[i]) ? ia[i] : ib[i];
            total_checksum += (uint32_t)result;
        }
    }
    
    return total_checksum;
}

/* ==================== Main Driver ==================== */

int main(void) {
    uint64_t total_checksum = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    printf("===========================\n");
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512F and AVX-512BW detected. Using vectorized implementation.\n");
    
    total_checksum += test_v64qi_blend();
    printf("  V64QImode blend test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("  V32HImode blend test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("  V32HFmode blend test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("  V32BFmode blend test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("  V16SImode blend test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("  V8DImode blend test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("  V8DFmode blend test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("  V16SFmode blend test completed\n");
    
    total_checksum += test_mixed_blend_loop();
    printf("  Mixed data type loop test completed\n");
    
#else
    printf("AVX-512BW not detected. Using scalar fallback for some tests.\n");
#endif
#else
    printf("AVX-512F not detected. Using scalar fallback implementation.\n");
#endif

#ifndef __AVX512F__
    /* Use scalar fallbacks */
    total_checksum += scalar_test_v64qi_blend();
    printf("  V64QImode scalar test completed\n");
    
    total_checksum += scalar_test_v32hi_blend();
    printf("  V32HImode scalar test completed\n");
    
    total_checksum += scalar_test_v16si_blend();
    printf("  V16SImode scalar test completed\n");
    
    total_checksum += scalar_test_v8di_blend();
    printf("  V8DImode scalar test completed\n");
    
    total_checksum += scalar_test_v8df_blend();
    printf("  V8DFmode scalar test completed\n");
    
    total_checksum += scalar_test_v16sf_blend();
    printf("  V16SFmode scalar test completed\n");
    
    total_checksum += scalar_test_mixed_blend_loop();
    printf("  Mixed data type scalar loop test completed\n");
#endif
    
    printf("\nTotal checksum: %lu\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return (int)(total_checksum % 256);
}
