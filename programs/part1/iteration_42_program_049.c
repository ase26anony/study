#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    __m512i a = _mm512_set1_epi8(1);
    __m512i b = _mm512_set1_epi8(2);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Compute checksum to prevent elimination */
    alignas(64) uint8_t data[64];
    _mm512_store_si512(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += data[i];
    }
    return sum;
}

/* V32HImode - 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    __m512i a = _mm512_set1_epi16(10);
    __m512i b = _mm512_set1_epi16(20);
    
    /* Generate mask from runtime comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    /* Blend operation */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Compute checksum */
    alignas(64) uint16_t data[32];
    _mm512_store_si512(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += data[i];
    }
    return sum;
}

/* V32HFmode - 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 pattern_a[32], pattern_b[32];
    for (int i = 0; i < 32; i++) {
        pattern_a[i] = (_Float16)(i % 10);
        pattern_b[i] = (_Float16)(i % 5 + 10);
    }
    
    __m512h a = _mm512_load_ph(pattern_a);
    __m512h b = _mm512_load_ph(pattern_b);
    
    /* Generate mask */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    
    /* Blend operation */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Compute checksum */
    _Float16 data[32];
    _mm512_store_ph(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint64_t)(data[i] * 100);
    }
    return sum;
}
#endif

/* V32BFmode - 32-bfloat16 (emulated with epi16) */
static uint64_t test_v32bf_blend(void) {
    __m512i a = _mm512_set1_epi16(0x3C00); /* 1.0 in bfloat16 */
    __m512i b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
    
    /* Generate mask using integer comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    /* Use epi16 blend for bfloat16 data */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Compute checksum */
    alignas(64) uint16_t data[32];
    _mm512_store_si512(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += data[i];
    }
    return sum;
}

/* V16SImode - 16-dword integers */
static uint64_t test_v16si_blend(void) {
    __m512i a = _mm512_set1_epi32(100);
    __m512i b = _mm512_set1_epi32(200);
    
    /* Generate mask */
    __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
    
    /* Blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Compute checksum */
    alignas(64) uint32_t data[16];
    _mm512_store_si512(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

/* V8DImode - 8-qword integers */
static uint64_t test_v8di_blend(void) {
    __m512i a = _mm512_set1_epi64(1000);
    __m512i b = _mm512_set1_epi64(2000);
    
    /* Generate mask */
    __mmask8 mask = _mm512_cmp_epi64_mask(a, b, _MM_CMPINT_LT);
    
    /* Blend operation */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Compute checksum */
    alignas(64) uint64_t data[8];
    _mm512_store_si512(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += data[i];
    }
    return sum;
}

/* V8DFmode - 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    /* Generate mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* Blend operation */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Compute checksum */
    alignas(64) double data[8];
    _mm512_store_pd(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(data[i] * 1000);
    }
    return sum;
}

/* V16SFmode - 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    __m512 a = _mm512_set1_ps(1.5f);
    __m512 b = _mm512_set1_ps(2.5f);
    
    /* Generate mask */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    /* Blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Compute checksum */
    alignas(64) float data[16];
    _mm512_store_ps(data, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)(data[i] * 100);
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    alignas(64) float fdata_a[N], fdata_b[N];
    alignas(64) double ddata_a[N], ddata_b[N];
    alignas(64) int32_t idata_a[N], idata_b[N];
    alignas(64) int16_t sdata_a[N], sdata_b[N];
    
    /* Initialize with patterns */
    for (int i = 0; i < N; i++) {
        fdata_a[i] = (float)(i % 32);
        fdata_b[i] = (float)(i % 16 + 32);
        ddata_a[i] = (double)(i % 16);
        ddata_b[i] = (double)(i % 8 + 16);
        idata_a[i] = i % 64;
        idata_b[i] = i % 32 + 64;
        sdata_a[i] = (int16_t)(i % 128);
        sdata_b[i] = (int16_t)(i % 64 + 128);
    }
    
    uint64_t total_sum = 0;
    
    /* Process float arrays - V16SFmode */
    for (int i = 0; i < N; i += 16) {
        __m512 a = _mm512_load_ps(&fdata_a[i]);
        __m512 b = _mm512_load_ps(&fdata_b[i]);
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        alignas(64) float temp[16];
        _mm512_store_ps(temp, result);
        for (int j = 0; j < 16; j++) {
            total_sum += (uint64_t)(temp[j] * 10);
        }
    }
    
    /* Process double arrays - V8DFmode */
    for (int i = 0; i < N; i += 8) {
        __m512d a = _mm512_load_pd(&ddata_a[i]);
        __m512d b = _mm512_load_pd(&ddata_b[i]);
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        alignas(64) double temp[8];
        _mm512_store_pd(temp, result);
        for (int j = 0; j < 8; j++) {
            total_sum += (uint64_t)(temp[j] * 10);
        }
    }
    
    /* Process int32 arrays - V16SImode */
    for (int i = 0; i < N; i += 16) {
        __m512i a = _mm512_load_si512(&idata_a[i]);
        __m512i b = _mm512_load_si512(&idata_b[i]);
        __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        alignas(64) int32_t temp[16];
        _mm512_store_si512(temp, result);
        for (int j = 0; j < 16; j++) {
            total_sum += temp[j];
        }
    }
    
    /* Process int16 arrays - V32HImode */
    for (int i = 0; i < N; i += 32) {
        __m512i a = _mm512_load_si512(&sdata_a[i]);
        __m512i b = _mm512_load_si512(&sdata_b[i]);
        __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        alignas(64) int16_t temp[32];
        _mm512_store_si512(temp, result);
        for (int j = 0; j < 32; j++) {
            total_sum += temp[j];
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t scalar_test_v64qi_blend(void) {
    uint8_t a = 1, b = 2;
    uint8_t result = (a < b) ? b : a;
    return result * 64;
}

static uint64_t scalar_test_v32hi_blend(void) {
    uint16_t a = 10, b = 20;
    uint16_t result = (a < b) ? b : a;
    return result * 32;
}

static uint64_t scalar_test_v32bf_blend(void) {
    uint16_t a = 0x3C00, b = 0x4000;
    uint16_t result = (a < b) ? b : a;
    return result * 32;
}

static uint64_t scalar_test_v16si_blend(void) {
    uint32_t a = 100, b = 200;
    uint32_t result = (a < b) ? b : a;
    return result * 16;
}

static uint64_t scalar_test_v8di_blend(void) {
    uint64_t a = 1000, b = 2000;
    uint64_t result = (a < b) ? b : a;
    return result * 8;
}

static uint64_t scalar_test_v8df_blend(void) {
    double a = 1.0, b = 2.0;
    double result = (a < b) ? b : a;
    return (uint64_t)(result * 1000 * 8);
}

static uint64_t scalar_test_v16sf_blend(void) {
    float a = 1.5f, b = 2.5f;
    float result = (a < b) ? b : a;
    return (uint64_t)(result * 100 * 16);
}

static uint64_t scalar_test_mixed_blends(void) {
    uint64_t sum = 0;
    for (int i = 0; i < 1024; i++) {
        float fa = (float)(i % 32);
        float fb = (float)(i % 16 + 32);
        sum += (uint64_t)((fa < fb ? fb : fa) * 10);
        
        double da = (double)(i % 16);
        double db = (double)(i % 8 + 16);
        sum += (uint64_t)((da < db ? db : da) * 10);
        
        int32_t ia = i % 64;
        int32_t ib = i % 32 + 64;
        sum += (ia < ib ? ib : ia);
        
        int16_t sa = (int16_t)(i % 128);
        int16_t sb = (int16_t)(i % 64 + 128);
        sum += (sa < sb ? sb : sa);
    }
    return sum;
}

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Running AVX-512 optimized blend tests...\n");
    
    total_checksum += test_v64qi_blend();
    total_checksum += test_v32hi_blend();
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
#endif
    
    total_checksum += test_v32bf_blend();
    total_checksum += test_v16si_blend();
    total_checksum += test_v8di_blend();
    total_checksum += test_v8df_blend();
    total_checksum += test_v16sf_blend();
    total_checksum += test_mixed_blends();
    
#else
    printf("Running scalar fallback tests...\n");
    
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += scalar_test_v32bf_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
    total_checksum += scalar_test_v8df_blend();
    total_checksum += scalar_test_v16sf_blend();
    total_checksum += scalar_test_mixed_blends();
#endif
#else
    printf("Running scalar fallback tests...\n");
    
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v32hi_blend();
    total_checksum += scalar_test_v32bf_blend();
    total_checksum += scalar_test_v16si_blend();
    total_checksum += scalar_test_v8di_blend();
    total_checksum += scalar_test_v8df_blend();
    total_checksum += scalar_test_v16sf_blend();
    total_checksum += scalar_test_mixed_blends();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
