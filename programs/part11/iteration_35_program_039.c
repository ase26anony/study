#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
__attribute__((noinline))
int test_blend_v64qi(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create vectors using different patterns */
        __m512i a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        /* Method 1: Constant mask pattern */
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Method 2: Data-dependent mask */
        __mmask64 mask2 = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a, _mm512_set1_epi8(1)),
            _mm512_set1_epi8(0)
        );
        
        /* Method 3: Alternating pattern based on iteration */
        __mmask64 mask3 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        /* Use different masks to ensure all code paths are exercised */
        __m512i blended;
        if (i % 3 == 0) {
            blended = _mm512_mask_blend_epi8(mask1, a, b);
        } else if (i % 3 == 1) {
            blended = _mm512_mask_blend_epi8(mask2, a, b);
        } else {
            blended = _mm512_mask_blend_epi8(mask3, a, b);
        }
        
        /* Extract and accumulate to prevent optimization */
        volatile char temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        /* Different mask generation strategies */
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = _mm512_cmpeq_epi16_mask(
            _mm512_and_si512(a, _mm512_set1_epi16(1)),
            _mm512_set1_epi16(0)
        );
        __mmask32 mask3 = (__mmask32)((i << 1) | (i >> 1));
        
        __m512i blended;
        if (i % 4 == 0) {
            blended = _mm512_mask_blend_epi16(mask1, a, b);
        } else if (i % 4 == 1) {
            blended = _mm512_mask_blend_epi16(mask2, a, b);
        } else if (i % 4 == 2) {
            blended = _mm512_mask_blend_epi16(mask3, a, b);
        } else {
            /* All-ones mask */
            blended = _mm512_mask_blend_epi16(0xFFFFFFFF, a, b);
        }
        
        volatile short temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create half-precision float vectors */
        __m512h a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512h b = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __mmask32 mask = 0x55555555 ^ (i * 0x11111111);
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        /* Extract and accumulate */
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)temp[i % 32];
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create bfloat16 vectors */
        __m512bh a = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
        __m512bh b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
        
        /* Create pattern mask */
        __mmask32 mask = 0xAAAAAAAA ^ (i * 0x11111111);
        
        /* Blend bfloat16 vectors */
        __m512bh blended = _mm512_mask_blend_epi16(mask, 
            (__m512i)a, (__m512i)b);
        
        /* Store and accumulate */
        volatile unsigned short temp[32];
        _mm512_storeu_si512((void*)temp, (__m512i)blended);
        result += temp[i % 32];
    }
    
    return result;
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        /* Multiple mask generation approaches */
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = _mm512_cmpeq_epi32_mask(
            _mm512_and_si512(a, _mm512_set1_epi32(1)),
            _mm512_set1_epi32(0)
        );
        __mmask16 mask3 = (__mmask16)((i << 2) | (i >> 2));
        
        __m512i blended;
        if (i % 3 == 0) {
            blended = _mm512_mask_blend_epi32(mask1, a, b);
        } else if (i % 3 == 1) {
            blended = _mm512_mask_blend_epi32(mask2, a, b);
        } else {
            blended = _mm512_mask_blend_epi32(mask3, a, b);
        }
        
        /* Reduce and accumulate */
        result += _mm512_reduce_add_epi32(blended);
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        
        __mmask8 mask = 0xAA ^ (i & 0xFF);
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        /* Extract and accumulate */
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        
        /* Create mask using comparison */
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        /* Horizontal add and accumulate */
        __m512d sum = _mm512_add_pd(blended, _mm512_set1_pd(i));
        volatile double temp[8];
        _mm512_storeu_pd(temp, sum);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 b = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        /* Multiple mask strategies */
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        __mmask16 mask3 = (__mmask16)(0x5555 ^ (i * 0x1111));
        
        __m512 blended;
        if (i % 4 == 0) {
            blended = _mm512_mask_blend_ps(mask1, a, b);
        } else if (i % 4 == 1) {
            blended = _mm512_mask_blend_ps(mask2, a, b);
        } else if (i % 4 == 2) {
            blended = _mm512_mask_blend_ps(mask3, a, b);
        } else {
            blended = _mm512_mask_blend_ps(0xFFFF, a, b);
        }
        
        /* Reduce and accumulate */
        __m512 sum = _mm512_add_ps(blended, _mm512_set1_ps(i));
        volatile float temp[16];
        _mm512_storeu_ps(temp, sum);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif /* __AVX512F__ */

/* Fallback implementations for non-AVX512 builds */
#ifndef __AVX512F__
int test_blend_v64qi(int iterations) { return 0; }
int test_blend_v32hi(int iterations) { return 0; }
int test_blend_v32hf(int iterations) { return 0; }
int test_blend_v32bf(int iterations) { return 0; }
int test_blend_v16si(int iterations) { return 0; }
int test_blend_v8di(int iterations) { return 0; }
int test_blend_v8df(int iterations) { return 0; }
int test_blend_v16sf(int iterations) { return 0; }
#endif

int main(int argc, char *argv[]) {
    /* Check CPU support at runtime */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported, some tests will be limited\n");
    }
    
    const int iterations = 100;
    volatile int total_result = 0;
    
    /* Command-line selection of specific test */
    if (argc > 1) {
        if (strcmp(argv[1], "v64qi") == 0) {
            total_result += test_blend_v64qi(iterations);
        } else if (strcmp(argv[1], "v32hi") == 0 && has_avx512bw) {
            total_result += test_blend_v32hi(iterations);
        } else if (strcmp(argv[1], "v32hf") == 0 && has_avx512bw) {
            total_result += test_blend_v32hf(iterations);
        } else if (strcmp(argv[1], "v32bf") == 0 && has_avx512bw) {
            total_result += test_blend_v32bf(iterations);
        } else if (strcmp(argv[1], "v16si") == 0) {
            total_result += test_blend_v16si(iterations);
        } else if (strcmp(argv[1], "v8di") == 0) {
            total_result += test_blend_v8di(iterations);
        } else if (strcmp(argv[1], "v8df") == 0) {
            total_result += test_blend_v8df(iterations);
        } else if (strcmp(argv[1], "v16sf") == 0) {
            total_result += test_blend_v16sf(iterations);
        } else if (strcmp(argv[1], "all") == 0) {
            /* Run all tests */
            total_result += test_blend_v64qi(iterations);
            if (has_avx512bw) {
                total_result += test_blend_v32hi(iterations);
                total_result += test_blend_v32hf(iterations);
                total_result += test_blend_v32bf(iterations);
            }
            total_result += test_blend_v16si(iterations);
            total_result += test_blend_v8di(iterations);
            total_result += test_blend_v8df(iterations);
            total_result += test_blend_v16sf(iterations);
        } else {
            printf("Unknown test: %s\n", argv[1]);
            printf("Available tests: v64qi, v32hi, v32hf, v32bf, v16si, v8di, v8df, v16sf, all\n");
            return 1;
        }
    } else {
        /* Default: run all tests */
        total_result += test_blend_v64qi(iterations);
        if (has_avx512bw) {
            total_result += test_blend_v32hi(iterations);
            total_result += test_blend_v32hf(iterations);
            total_result += test_blend_v32bf(iterations);
        }
        total_result += test_blend_v16si(iterations);
        total_result += test_blend_v8di(iterations);
        total_result += test_blend_v8df(iterations);
        total_result += test_blend_v16sf(iterations);
    }
    
    printf("Final checksum: %d\n", total_result);
    return 0;
}
