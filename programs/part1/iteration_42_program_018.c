#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
static uint64_t test_v64qi_blend(void) {
    char a[64] __attribute__((aligned(64)));
    char b[64] __attribute__((aligned(64)));
    
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Generate dynamic mask using comparison */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Use result in computation to prevent elimination */
    char res[64] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)res[i];
    }
    return sum;
}

/* V32HImode: 32-halfword integers */
static uint64_t test_v32hi_blend(void) {
    short a[32] __attribute__((aligned(64)));
    short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (short)(i * 7);
        b[i] = (short)(i * 11);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Generate dynamic mask */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_LT);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    short res[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)res[i];
    }
    return sum;
}

#ifdef __AVX512FP16__
/* V32HFmode: 32-half-precision floats */
static float test_v32hf_blend(void) {
    _Float16 a[32] __attribute__((aligned(64)));
    _Float16 b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 1.5f);
        b[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Generate mask by comparing */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _Float16 res[32] __attribute__((aligned(64)));
    _mm512_store_ph(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}
#endif

/* V32BFmode: 32-bfloat16 floats (emulated with epi16) */
static float test_v32bf_blend(void) {
    unsigned short a[32] __attribute__((aligned(64)));
    unsigned short b[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        a[i] = (unsigned short)(i * 0x4000);  /* bfloat16 pattern */
        b[i] = (unsigned short)(i * 0x4040);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Generate mask by comparing as 16-bit integers */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_EQ);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    unsigned short res[32] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        /* Convert bfloat16 pattern to float for checksum */
        unsigned int temp = res[i] << 16;
        float f;
        memcpy(&f, &temp, sizeof(float));
        sum += f;
    }
    return sum;
}

/* V16SImode: 16-dword integers */
static uint64_t test_v16si_blend(void) {
    int a[16] __attribute__((aligned(64)));
    int b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 13;
        b[i] = i * 17;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_NE);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    int res[16] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint32_t)res[i];
    }
    return sum;
}

/* V8DImode: 8-qword integers */
static uint64_t test_v8di_blend(void) {
    long long a[8] __attribute__((aligned(64)));
    long long b[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        a[i] = i * 23LL;
        b[i] = i * 29LL;
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GE);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    long long res[8] __attribute__((aligned(64)));
    _mm512_store_si512((__m512i*)res, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)res[i];
    }
    return sum;
}

/* V8DFmode: 8-double-precision floats */
static double test_v8df_blend(void) {
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
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* V16SFmode: 16-single-precision floats */
static float test_v16sf_blend(void) {
    float a[16] __attribute__((aligned(64)));
    float b[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.75f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_LE_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    float res[16] __attribute__((aligned(64)));
    _mm512_store_ps(res, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* Mixed data types in loop structure */
static uint64_t test_mixed_blends(int iterations) {
    uint64_t total_sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Test different blend operations in sequence */
        if (iter & 1) {
            /* V64QImode blend */
            char data1[64], data2[64];
            for (int i = 0; i < 64; i++) {
                data1[i] = (char)((i + iter) & 0xFF);
                data2[i] = (char)((i * iter) & 0xFF);
            }
            __m512i v1 = _mm512_loadu_si512(data1);
            __m512i v2 = _mm512_loadu_si512(data2);
            __mmask64 mask = _mm512_cmp_epi8_mask(v1, v2, _MM_CMPINT_GT);
            __m512i res = _mm512_mask_blend_epi8(mask, v1, v2);
            
            char out[64];
            _mm512_storeu_si512(out, res);
            for (int i = 0; i < 64; i++) total_sum += out[i];
        }
        
        if (iter & 2) {
            /* V32HImode blend */
            short data1[32], data2[32];
            for (int i = 0; i < 32; i++) {
                data1[i] = (short)((i + iter) * 3);
                data2[i] = (short)((i * iter) * 5);
            }
            __m512i v1 = _mm512_loadu_si512(data1);
            __m512i v2 = _mm512_loadu_si512(data2);
            __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_LT);
            __m512i res = _mm512_mask_blend_epi16(mask, v1, v2);
            
            short out[32];
            _mm512_storeu_si512(out, res);
            for (int i = 0; i < 32; i++) total_sum += out[i];
        }
        
        if (iter & 4) {
            /* V16SFmode blend */
            float data1[16], data2[16];
            for (int i = 0; i < 16; i++) {
                data1[i] = (i + iter) * 0.25f;
                data2[i] = (i * iter) * 0.125f;
            }
            __m512 v1 = _mm512_loadu_ps(data1);
            __m512 v2 = _mm512_loadu_ps(data2);
            __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_EQ_OQ);
            __m512 res = _mm512_mask_blend_ps(mask, v1, v2);
            
            float out[16];
            _mm512_storeu_ps(out, res);
            for (int i = 0; i < 16; i++) total_sum += (uint32_t)(out[i] * 1000);
        }
    }
    
    return total_sum;
}

#else  /* !__AVX512BW__ */

/* Scalar fallback implementations */
static uint64_t test_v64qi_blend(void) {
    char a[64], b[64];
    for (int i = 0; i < 64; i++) {
        a[i] = (char)(i * 3);
        b[i] = (char)(i * 5);
    }
    
    uint64_t mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((uint64_t)(a[i] > b[i]) << i);
    }
    
    char res[64];
    for (int i = 0; i < 64; i++) {
        res[i] = (mask & (1ULL << i)) ? a[i] : b[i];
    }
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) sum += (uint8_t)res[i];
    return sum;
}

/* Similar scalar fallbacks for other modes... */

#endif  /* __AVX512BW__ */
#endif  /* __AVX512F__ */

int main(void) {
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 support detected. Running vectorized tests...\n");
    
    /* Run all specific blend tests */
    total_checksum += test_v64qi_blend();
    printf("V64QImode test completed\n");
    
    total_checksum += test_v32hi_blend();
    printf("V32HImode test completed\n");
    
#ifdef __AVX512FP16__
    total_checksum += (uint64_t)test_v32hf_blend();
    printf("V32HFmode test completed\n");
#endif
    
    total_checksum += (uint64_t)test_v32bf_blend();
    printf("V32BFmode test completed\n");
    
    total_checksum += test_v16si_blend();
    printf("V16SImode test completed\n");
    
    total_checksum += test_v8di_blend();
    printf("V8DImode test completed\n");
    
    total_checksum += (uint64_t)test_v8df_blend();
    printf("V8DFmode test completed\n");
    
    total_checksum += (uint64_t)test_v16sf_blend();
    printf("V16SFmode test completed\n");
    
    /* Run mixed test */
    total_checksum += test_mixed_blends(10);
    printf("Mixed blends test completed\n");
    
#else
    printf("AVX-512BW not available, using scalar fallbacks\n");
    total_checksum += test_v64qi_blend();
#endif
#else
    printf("AVX-512 not available\n");
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    return (int)(total_checksum & 0x7FFFFFFF);
}
