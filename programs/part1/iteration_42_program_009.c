#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    __m512i a = _mm512_set1_epi8(1);
    __m512i b = _mm512_set1_epi8(2);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Compute checksum to prevent elimination */
    uint64_t sum = 0;
    uint8_t temp[64];
    _mm512_storeu_si512((__m512i*)temp, result);
    for (int i = 0; i < 64; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    __m512i a = _mm512_set1_epi16(10);
    __m512i b = _mm512_set1_epi16(20);
    
    /* Generate dynamic mask */
    __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    uint16_t temp[32];
    _mm512_storeu_si512((__m512i*)temp, result);
    for (int i = 0; i < 32; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V32HFmode: 32-half-precision floats */
#ifdef __AVX512FP16__
static uint64_t test_v32hf_blend(void) {
    _Float16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = (_Float16)(i * 1.5f);
        b_vals[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h a = _mm512_loadu_ph(a_vals);
    __m512h b = _mm512_loadu_ph(b_vals);
    
    /* Generate mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    _Float16 temp[32];
    _mm512_storeu_ph(temp, result);
    for (int i = 0; i < 32; i++) {
        sum += (uint64_t)(temp[i] * 100);
    }
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 floats */
#ifdef __AVX512BF16__
static uint64_t test_v32bf_blend(void) {
    /* Use epi16 blend for bfloat16 emulation */
    __m512i a = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
    __m512i b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
    
    /* Generate dynamic mask */
    __mmask32 mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation using epi16 intrinsic */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    uint16_t temp[32];
    _mm512_storeu_si512((__m512i*)temp, result);
    for (int i = 0; i < 32; i++) {
        sum += temp[i];
    }
    return sum;
}
#endif

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    __m512i a = _mm512_set1_epi32(100);
    __m512i b = _mm512_set1_epi32(200);
    
    /* Generate dynamic mask */
    __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    uint32_t temp[16];
    _mm512_storeu_si512((__m512i*)temp, result);
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    __m512i a = _mm512_set1_epi64(1000);
    __m512i b = _mm512_set1_epi64(2000);
    
    /* Generate dynamic mask */
    __mmask8 mask = _mm512_cmp_epi64_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    uint64_t temp[8];
    _mm512_storeu_si512((__m512i*)temp, result);
    for (int i = 0; i < 8; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static uint64_t test_v8df_blend(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    /* Generate dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    double temp[8];
    _mm512_storeu_pd(temp, result);
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(temp[i] * 1000);
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static uint64_t test_v16sf_blend(void) {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    /* Generate dynamic mask */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _MM_CMPINT_LT);
    
    /* Perform blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Compute checksum */
    uint64_t sum = 0;
    float temp[16];
    _mm512_storeu_ps(temp, result);
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)(temp[i] * 1000);
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(void) {
    const int N = 1024;
    uint64_t total_sum = 0;
    
    /* Process arrays with different data types */
    for (int i = 0; i < N; i += 16) {
        /* V16SFmode blend in loop */
        float fa[16], fb[16];
        for (int j = 0; j < 16; j++) {
            fa[j] = (float)(i + j);
            fb[j] = (float)(i + j + 100);
        }
        
        __m512 a = _mm512_loadu_ps(fa);
        __m512 b = _mm512_loadu_ps(fb);
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _MM_CMPINT_LT);
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        float fres[16];
        _mm512_storeu_ps(fres, result);
        for (int j = 0; j < 16; j++) {
            total_sum += (uint64_t)fres[j];
        }
    }
    
    for (int i = 0; i < N; i += 8) {
        /* V8DFmode blend in loop */
        double da[8], db[8];
        for (int j = 0; j < 8; j++) {
            da[j] = (double)(i + j);
            db[j] = (double)(i + j + 100);
        }
        
        __m512d a = _mm512_loadu_pd(da);
        __m512d b = _mm512_loadu_pd(db);
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _MM_CMPINT_LT);
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        double dres[8];
        _mm512_storeu_pd(dres, result);
        for (int j = 0; j < 8; j++) {
            total_sum += (uint64_t)dres[j];
        }
    }
    
    return total_sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

/* Scalar fallback implementations */
static uint64_t scalar_test_v64qi_blend(void) {
    uint8_t a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = 1;
        b[i] = 2;
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        uint8_t result = (a[i] < b[i]) ? b[i] : a[i];
        sum += result;
    }
    return sum;
}

static uint64_t scalar_test_v16sf_blend(void) {
    float a[16], b[16];
    for (int i = 0; i < 16; i++) {
        a[i] = 1.0f;
        b[i] = 2.0f;
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        float result = (a[i] < b[i]) ? b[i] : a[i];
        sum += (uint64_t)(result * 1000);
    }
    return sum;
}

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    /* Test all vector modes */
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
#ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend();
    printf("V32BFmode test completed\n");
#endif
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    total_checksum += test_mixed_blends();
    printf("Mixed blends test completed\n");
    
#else
    printf("AVX-512BW not available. Using scalar fallbacks...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v16sf_blend();
#endif
#else
    printf("AVX-512 not available. Using scalar fallbacks...\n");
    total_checksum += scalar_test_v64qi_blend();
    total_checksum += scalar_test_v16sf_blend();
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum % 256);
}
