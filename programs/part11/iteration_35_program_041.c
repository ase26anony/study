#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
__attribute__((noinline))
int test_blend_v64qi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Method 1: Constant mask pattern */
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Method 2: Data-dependent mask */
        __mmask64 mask2 = (__mmask64)(i * 0x5555555555555555ULL);
        
        /* Method 3: Comparison-based mask */
        __m512i vec_a = _mm512_set_epi8(
            64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,
            48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,
            32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
            16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
        );
        __m512i vec_b = _mm512_set_epi8(
            128,127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,
            112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,
            96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,
            80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65
        );
        __m512i cmp_vec = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, cmp_vec);
        
        /* Blend with mask1 */
        __m512i result1 = _mm512_mask_blend_epi8(mask1, vec_a, vec_b);
        
        /* Blend with mask2 */
        __m512i result2 = _mm512_mask_blend_epi8(mask2, vec_a, vec_b);
        
        /* Blend with mask3 */
        __m512i result3 = _mm512_mask_blend_epi8(mask3, vec_a, vec_b);
        
        /* Extract and accumulate results */
        volatile uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[0] + temp[63];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[31] + temp[32];
        
        _mm512_storeu_si512((void*)temp, result3);
        sum += temp[16] + temp[47];
    }
    
    return sum;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Method 1: Constant mask */
        __mmask32 mask1 = 0xAAAAAAAA;
        
        /* Method 2: Patterned mask */
        __mmask32 mask2 = (__mmask32)((i & 1) ? 0xFFFFFFFF : 0x55555555);
        
        __m512i vec_a = _mm512_set_epi16(
            32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
            16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
        );
        __m512i vec_b = _mm512_set_epi16(
            64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,
            48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33
        );
        
        /* Blend operations */
        __m512i result1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512i result2 = _mm512_mask_blend_epi16(mask2, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile int16_t temp[32];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[0] + temp[31];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[15] + temp[16];
    }
    
    return sum;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __mmask32 mask = (__mmask32)(0x55555555 ^ (i * 0x11111111));
        
        /* Create vectors with different patterns */
        __m512h vec_a = _mm512_set_ph(
            1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
            17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
            25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
        );
        __m512h vec_b = _mm512_set_ph(
            100.0f, 99.0f, 98.0f, 97.0f, 96.0f, 95.0f, 94.0f, 93.0f,
            92.0f, 91.0f, 90.0f, 89.0f, 88.0f, 87.0f, 86.0f, 85.0f,
            84.0f, 83.0f, 82.0f, 81.0f, 80.0f, 79.0f, 78.0f, 77.0f,
            76.0f, 75.0f, 74.0f, 73.0f, 72.0f, 71.0f, 70.0f, 69.0f
        );
        
        /* Blend operation */
        __m512h result = _mm512_mask_blend_ph(mask, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, result);
        sum += (int)temp[0] + (int)temp[31];
    }
    
    return sum;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __mmask32 mask = (__mmask32)(0x33333333 | (i & 0xCCCCCCCC));
        
        /* Create bfloat16 vectors */
        __m512bh vec_a = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
        __m512bh vec_b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
        
        /* Blend operation */
        __m512bh result = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, result);
        sum += temp[0] + temp[31];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple mask generation methods */
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x1111);
        
        /* Comparison-based mask */
        __m512i vec_a = _mm512_set_epi32(
            16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
        );
        __m512i vec_b = _mm512_set_epi32(
            32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17
        );
        __m512i cmp_val = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, cmp_val);
        
        /* Blend operations */
        __m512i result1 = _mm512_mask_blend_epi32(mask1, vec_a, vec_b);
        __m512i result2 = _mm512_mask_blend_epi32(mask2, vec_a, vec_b);
        __m512i result3 = _mm512_mask_blend_epi32(mask3, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile int32_t temp[16];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[0] + temp[15];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[7] + temp[8];
        
        _mm512_storeu_si512((void*)temp, result3);
        sum += temp[3] + temp[12];
    }
    
    return sum;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __mmask8 mask = (__mmask8)(0xAA ^ (i & 0xFF));
        
        __m512i vec_a = _mm512_set_epi64(8,7,6,5,4,3,2,1);
        __m512i vec_b = _mm512_set_epi64(16,15,14,13,12,11,10,9);
        
        /* Blend operation */
        __m512i result = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile int64_t temp[8];
        _mm512_storeu_si512((void*)temp, result);
        sum += (int)(temp[0] + temp[7]);
    }
    
    return sum;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __mmask8 mask = (__mmask8)(0x55 | ((i % 3) << 1));
        
        __m512d vec_a = _mm512_set_pd(8.0,7.0,6.0,5.0,4.0,3.0,2.0,1.0);
        __m512d vec_b = _mm512_set_pd(16.0,15.0,14.0,13.0,12.0,11.0,10.0,9.0);
        
        /* Blend operation */
        __m512d result = _mm512_mask_blend_pd(mask, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, result);
        sum += (int)(temp[0] + temp[7]);
    }
    
    return sum;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple mask patterns */
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        __m512 vec_a = _mm512_set_ps(
            16.0f,15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,
            8.0f,7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f
        );
        __m512 vec_b = _mm512_set_ps(
            32.0f,31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,
            24.0f,23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f
        );
        
        /* Comparison-based mask */
        __m512 cmp_val = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, cmp_val, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512 result1 = _mm512_mask_blend_ps(mask1, vec_a, vec_b);
        __m512 result2 = _mm512_mask_blend_ps(mask2, vec_a, vec_b);
        __m512 result3 = _mm512_mask_blend_ps(mask3, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, result1);
        sum += (int)(temp[0] + temp[15]);
        
        _mm512_storeu_ps((void*)temp, result2);
        sum += (int)(temp[7] + temp[8]);
        
        _mm512_storeu_ps((void*)temp, result3);
        sum += (int)(temp[3] + temp[12]);
    }
    
    return sum;
}

#else
/* Dummy implementations for non-AVX512 builds */
int test_blend_v64qi(int iterations) { return 0; }
int test_blend_v32hi(int iterations) { return 0; }
int test_blend_v32hf(int iterations) { return 0; }
int test_blend_v32bf(int iterations) { return 0; }
int test_blend_v16si(int iterations) { return 0; }
int test_blend_v8di(int iterations) { return 0; }
int test_blend_v8df(int iterations) { return 0; }
int test_blend_v16sf(int iterations) { return 0; }
#endif /* __AVX512F__ */

int main(int argc, char *argv[]) {
    /* Check AVX-512 support at runtime */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported (needed for some operations)\n");
        /* Continue anyway for F-only operations */
    }
    
    const int iterations = 20;
    volatile int total_sum = 0;
    
    /* Determine which test to run based on command line */
    char *test_to_run = (argc > 1) ? argv[1] : "all";
    
    if (strcmp(test_to_run, "v64qi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v64qi(iterations);
        }
    }
    
    if (strcmp(test_to_run, "v32hi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hi(iterations);
        }
    }
    
    if (strcmp(test_to_run, "v32hf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hf(iterations);
        }
    }
    
    if (strcmp(test_to_run, "v32bf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32bf(iterations);
        }
    }
    
    if (strcmp(test_to_run, "v16si") == 0 || strcmp(test_to_run, "all") == 0) {
        total_sum += test_blend_v16si(iterations);
    }
    
    if (strcmp(test_to_run, "v8di") == 0 || strcmp(test_to_run, "all") == 0) {
        total_sum += test_blend_v8di(iterations);
    }
    
    if (strcmp(test_to_run, "v8df") == 0 || strcmp(test_to_run, "all") == 0) {
        total_sum += test_blend_v8df(iterations);
    }
    
    if (strcmp(test_to_run, "v16sf") == 0 || strcmp(test_to_run, "all") == 0) {
        total_sum += test_blend_v16sf(iterations);
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
