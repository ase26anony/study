#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
static int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Method 1: Constant mask */
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Method 2: Data-dependent mask */
        __mmask64 mask2 = (i & 1) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        /* Method 3: Comparison mask */
        __m512i vec_a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, vec_b);
        
        /* Blend with different masks */
        __m512i blend1 = _mm512_mask_blend_epi8(mask1, vec_a, vec_b);
        __m512i blend2 = _mm512_mask_blend_epi8(mask2, blend1, vec_a);
        __m512i blend3 = _mm512_mask_blend_epi8(mask3, blend2, vec_b);
        
        /* Extract and accumulate result */
        volatile char temp[64];
        _mm512_storeu_epi8((void*)temp, blend3);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
static int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask32 mask1 = 0xAAAAAAAA;
        
        /* Data-dependent mask */
        __mmask32 mask2 = (i & 1) ? 0xFFFFFFFF : 0x55555555;
        
        /* Comparison mask */
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, vec_b);
        
        /* Blend operations */
        __m512i blend1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512i blend2 = _mm512_mask_blend_epi16(mask2, blend1, vec_a);
        __m512i blend3 = _mm512_mask_blend_epi16(mask3, blend2, vec_b);
        
        /* Extract result */
        volatile short temp[32];
        _mm512_storeu_epi16((void*)temp, blend3);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
static int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask32 mask1 = 0xAAAAAAAA;
        
        /* Data-dependent mask */
        __mmask32 mask2 = (i & 1) ? 0xFFFFFFFF : 0x55555555;
        
        /* Create vectors */
        __m512h vec_a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512h vec_b = _mm512_set1_ph((float)i);
        
        /* Blend operations */
        __m512h blend1 = _mm512_mask_blend_ph(mask1, vec_a, vec_b);
        __m512h blend2 = _mm512_mask_blend_ph(mask2, blend1, vec_a);
        
        /* Extract and accumulate */
        volatile __fp16 temp[32];
        _mm512_storeu_ph((void*)temp, blend2);
        result += (int)temp[i % 32];
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
static int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask32 mask1 = 0xAAAAAAAA;
        
        /* Data-dependent mask */
        __mmask32 mask2 = (i & 1) ? 0xFFFFFFFF : 0x55555555;
        
        /* Create vectors (using integer representation for bfloat16) */
        __m512bh vec_a = _mm512_set_epi16(
            0x3F80, 0x4000, 0x4040, 0x4080, 0x40A0, 0x40C0, 0x40E0, 0x4100,
            0x4110, 0x4120, 0x4130, 0x4140, 0x4150, 0x4160, 0x4170, 0x4180,
            0x4188, 0x4190, 0x4198, 0x41A0, 0x41A8, 0x41B0, 0x41B8, 0x41C0,
            0x41C8, 0x41D0, 0x41D8, 0x41E0, 0x41E8, 0x41F0, 0x41F8, 0x4200
        );
        __m512bh vec_b = _mm512_set1_epi16(0x3F80 + i);
        
        /* Blend operations */
        __m512bh blend1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512bh blend2 = _mm512_mask_blend_epi16(mask2, blend1, vec_a);
        
        /* Extract and accumulate */
        volatile unsigned short temp[32];
        _mm512_storeu_epi16((void*)temp, (__m512i)blend2);
        result += temp[i % 32];
    }
    
    return result;
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
static int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask16 mask1 = 0xAAAA;
        
        /* Data-dependent mask */
        __mmask16 mask2 = (i & 1) ? 0xFFFF : 0x5555;
        
        /* Comparison mask */
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, vec_b);
        
        /* Blend operations */
        __m512i blend1 = _mm512_mask_blend_epi32(mask1, vec_a, vec_b);
        __m512i blend2 = _mm512_mask_blend_epi32(mask2, blend1, vec_a);
        __m512i blend3 = _mm512_mask_blend_epi32(mask3, blend2, vec_b);
        
        /* Extract and accumulate */
        volatile int temp[16];
        _mm512_storeu_epi32((void*)temp, blend3);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
static int test_blend_v8di(int iter) {
    volatile long long result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask8 mask1 = 0xAA;
        
        /* Data-dependent mask */
        __mmask8 mask2 = (i & 1) ? 0xFF : 0x55;
        
        /* Create vectors */
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set1_epi64(i);
        
        /* Blend operations */
        __m512i blend1 = _mm512_mask_blend_epi64(mask1, vec_a, vec_b);
        __m512i blend2 = _mm512_mask_blend_epi64(mask2, blend1, vec_a);
        
        /* Extract and accumulate */
        volatile long long temp[8];
        _mm512_storeu_epi64((void*)temp, blend2);
        result += temp[i % 8];
    }
    
    return (int)result;
}

/* V8DFmode: 8 x double-precision floats */
static int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask8 mask1 = 0xAA;
        
        /* Data-dependent mask */
        __mmask8 mask2 = (i & 1) ? 0xFF : 0x55;
        
        /* Comparison mask */
        __m512d vec_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec_b = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512d blend1 = _mm512_mask_blend_pd(mask1, vec_a, vec_b);
        __m512d blend2 = _mm512_mask_blend_pd(mask2, blend1, vec_a);
        __m512d blend3 = _mm512_mask_blend_pd(mask3, blend2, vec_b);
        
        /* Extract and accumulate */
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blend3);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
static int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Constant mask */
        __mmask16 mask1 = 0xAAAA;
        
        /* Data-dependent mask */
        __mmask16 mask2 = (i & 1) ? 0xFFFF : 0x5555;
        
        /* Comparison mask */
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512 vec_b = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512 blend1 = _mm512_mask_blend_ps(mask1, vec_a, vec_b);
        __m512 blend2 = _mm512_mask_blend_ps(mask2, blend1, vec_a);
        __m512 blend3 = _mm512_mask_blend_ps(mask3, blend2, vec_b);
        
        /* Extract and accumulate */
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blend3);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif /* __AVX512F__ */

/* Fallback implementations for non-AVX512 systems */
#ifndef __AVX512F__
static int test_blend_v64qi(int iter) { return 0; }
static int test_blend_v32hi(int iter) { return 0; }
static int test_blend_v32hf(int iter) { return 0; }
static int test_blend_v32bf(int iter) { return 0; }
static int test_blend_v16si(int iter) { return 0; }
static int test_blend_v8di(int iter) { return 0; }
static int test_blend_v8df(int iter) { return 0; }
static int test_blend_v16sf(int iter) { return 0; }
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
        printf("AVX-512BW not supported on this CPU (needed for some tests)\n");
    }
    
    const int iterations = 20;
    volatile int accumulator = 0;
    
    /* Determine which test to run based on command line argument */
    const char *test_mode = (argc > 1) ? argv[1] : "all";
    
    if (strcmp(test_mode, "v64qi") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                accumulator += test_blend_v64qi(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "v32hi") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                accumulator += test_blend_v32hi(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "v32hf") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                accumulator += test_blend_v32hf(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "v32bf") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < iterations; i++) {
                accumulator += test_blend_v32bf(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "v16si") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < iterations; i++) {
            accumulator += test_blend_v16si(iterations);
        }
    }
    
    if (strcmp(test_mode, "v8di") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < iterations; i++) {
            accumulator += test_blend_v8di(iterations);
        }
    }
    
    if (strcmp(test_mode, "v8df") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < iterations; i++) {
            accumulator += test_blend_v8df(iterations);
        }
    }
    
    if (strcmp(test_mode, "v16sf") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < iterations; i++) {
            accumulator += test_blend_v16sf(iterations);
        }
    }
    
    printf("Accumulated result: %d\n", accumulator);
    
    return 0;
}
