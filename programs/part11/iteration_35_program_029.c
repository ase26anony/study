#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
__attribute__((noinline))
int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Method 1: Constant mask */
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Method 2: Data-dependent mask */
        __mmask64 mask2 = (__mmask64)(i * 0x5555555555555555ULL);
        
        /* Method 3: Comparison mask */
        __m512i vec_a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, vec_b);
        
        /* Use different masks based on iteration */
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i src1 = _mm512_set1_epi8(i);
        __m512i src2 = _mm512_set1_epi8(i * 2);
        
        __m512i blended = _mm512_mask_blend_epi8(mask, src1, src2);
        
        /* Extract and accumulate result */
        volatile int8_t temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Multiple mask generation methods */
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, vec_b);
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i src1 = _mm512_set1_epi16(i);
        __m512i src2 = _mm512_set1_epi16(i * 3);
        
        __m512i blended = _mm512_mask_blend_epi16(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile int16_t temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        /* Create comparison mask */
        __m512h vec_a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512h vec_b = _mm512_set1_ph((float)i);
        __mmask32 mask3 = _mm512_cmp_ph_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512h src1 = _mm512_set1_ph((float)i);
        __m512h src2 = _mm512_set1_ph((float)(i * 2));
        
        __m512h blended = _mm512_mask_blend_ph(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile __fp16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)temp[i % 32];
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        /* Create comparison mask for bfloat16 */
        __m512bh vec_a = _mm512_set_epi16(
            0x3F80, 0x3F00, 0x3E80, 0x3E00, 0x3D80, 0x3D00, 0x3C80, 0x3C00,
            0x3B80, 0x3B00, 0x3A80, 0x3A00, 0x3980, 0x3900, 0x3880, 0x3800,
            0x3780, 0x3700, 0x3680, 0x3600, 0x3580, 0x3500, 0x3480, 0x3400,
            0x3380, 0x3300, 0x3280, 0x3200, 0x3180, 0x3100, 0x3080, 0x3000
        );
        __m512bh vec_b = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
        __mmask32 mask3 = _mm512_cmp_epi16_mask(vec_a, vec_b, _MM_CMPINT_EQ);
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512bh src1 = _mm512_set1_epi16(0x3F80); /* 1.0 */
        __m512bh src2 = _mm512_set1_epi16(0x4000); /* 2.0 */
        
        __m512bh blended = _mm512_mask_blend_epi16(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, (__m512i)blended);
        result += temp[i % 32];
    }
    
    return result;
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, vec_b);
        
        __mmask16 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i src1 = _mm512_set1_epi32(i);
        __m512i src2 = _mm512_set1_epi32(i * 4);
        
        __m512i blended = _mm512_mask_blend_epi32(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile int32_t temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec_a, vec_b);
        
        __mmask8 mask = (i % 3 == 0) ? mask1 : 
                       (i % 3 == 1) ? mask2 : mask3;
        
        __m512i src1 = _mm512_set1_epi64(i);
        __m512i src2 = _mm512_set1_epi64(i * 5);
        
        __m512i blended = _mm512_mask_blend_epi64(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile int64_t temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512d vec_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec_b = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        __mmask8 mask = (i % 3 == 0) ? mask1 : 
                       (i % 3 == 1) ? mask2 : mask3;
        
        __m512d src1 = _mm512_set1_pd((double)i);
        __m512d src2 = _mm512_set1_pd((double)(i * 6));
        
        __m512d blended = _mm512_mask_blend_pd(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512 vec_b = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        __mmask16 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512 src1 = _mm512_set1_ps((float)i);
        __m512 src2 = _mm512_set1_ps((float)(i * 7));
        
        __m512 blended = _mm512_mask_blend_ps(mask, src1, src2);
        
        /* Extract and accumulate */
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#else /* __AVX512F__ not defined */

/* Dummy implementations for non-AVX512 builds */
int test_blend_v64qi(int iter) { return 0; }
int test_blend_v32hi(int iter) { return 0; }
int test_blend_v32hf(int iter) { return 0; }
int test_blend_v32bf(int iter) { return 0; }
int test_blend_v16si(int iter) { return 0; }
int test_blend_v8di(int iter) { return 0; }
int test_blend_v8df(int iter) { return 0; }
int test_blend_v16sf(int iter) { return 0; }

#endif /* __AVX512F__ */

int main(int argc, char *argv[]) {
    /* Check CPU support at runtime */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported on this CPU (needed for some tests)\n");
        /* Continue anyway for tests that don't need BW */
    }
    
    const int iterations = 20;
    volatile int total_sum = 0;
    
    /* Parse command line argument to select specific test */
    char *test_to_run = NULL;
    if (argc > 1) {
        test_to_run = argv[1];
    }
    
    /* Run selected test or all tests */
    if (test_to_run == NULL || strcmp(test_to_run, "v64qi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v64qi(iterations);
            }
            printf("V64QImode test completed\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v32hi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v32hi(iterations);
            }
            printf("V32HImode test completed\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v32hf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v32hf(iterations);
            }
            printf("V32HFmode test completed\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v32bf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v32bf(iterations);
            }
            printf("V32BFmode test completed\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v16si") == 0) {
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v16si(iterations);
        }
        printf("V16SImode test completed\n");
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v8di") == 0) {
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v8di(iterations);
        }
        printf("V8DImode test completed\n");
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v8df") == 0) {
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v8df(iterations);
        }
        printf("V8DFmode test completed\n");
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v16sf") == 0) {
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v16sf(iterations);
        }
        printf("V16SFmode test completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
