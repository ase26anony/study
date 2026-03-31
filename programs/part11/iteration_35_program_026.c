#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
__attribute__((noinline))
int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Method 1: Constant mask */
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Method 2: Data-dependent mask */
        __mmask64 mask2 = (__mmask64)(i * 0x5555555555555555ULL);
        
        /* Method 3: Comparison mask */
        __m512i vec1 = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec2 = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec1, vec2);
        
        /* Blend with different masks */
        __m512i a = _mm512_set1_epi8(i);
        __m512i b = _mm512_set1_epi8(i * 2);
        
        __m512i blended1 = _mm512_mask_blend_epi8(mask1, a, b);
        __m512i blended2 = _mm512_mask_blend_epi8(mask2, blended1, a);
        __m512i blended3 = _mm512_mask_blend_epi8(mask3, blended2, b);
        
        /* Extract and accumulate result */
        volatile char temp[64];
        _mm512_storeu_epi8((void*)temp, blended3);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode - 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Three different mask generation methods */
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512i vec1 = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec2 = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec1, vec2);
        
        /* Blend operations */
        __m512i a = _mm512_set1_epi16(i);
        __m512i b = _mm512_set1_epi16(i * 3);
        
        __m512i blended1 = _mm512_mask_blend_epi16(mask1, a, b);
        __m512i blended2 = _mm512_mask_blend_epi16(mask2, blended1, a);
        __m512i blended3 = _mm512_mask_blend_epi16(mask3, blended2, b);
        
        /* Reduce and accumulate */
        volatile short temp[32];
        _mm512_storeu_epi16((void*)temp, blended3);
        for (int j = 0; j < 32; j++) {
            result += temp[j];
        }
    }
    
    return result;
}

/* V32HFmode - 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        /* Create comparison mask */
        __m512h vec1 = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512h vec2 = _mm512_set1_ph((_Float16)i);
        __mmask32 mask3 = _mm512_cmp_ph_mask(vec1, vec2, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512h a = _mm512_set1_ph((_Float16)i);
        __m512h b = _mm512_set1_ph((_Float16)(i * 2.5f));
        
        __m512h blended1 = _mm512_mask_blend_ph(mask1, a, b);
        __m512h blended2 = _mm512_mask_blend_ph(mask2, blended1, a);
        __m512h blended3 = _mm512_mask_blend_ph(mask3, blended2, b);
        
        /* Extract and accumulate */
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended3);
        for (int j = 0; j < 32; j++) {
            result += (int)temp[j];
        }
    }
    
    return result;
}

/* V32BFmode - 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        /* Create bfloat16 vectors */
        __m512bh vec1 = _mm512_set1_epi16(0x3C00); /* 1.0 in bfloat16 */
        __m512bh vec2 = _mm512_set1_epi16(i << 8); /* Varying values */
        
        /* Blend operations */
        __m512bh a = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
        __m512bh b = _mm512_set1_epi16(0x4080); /* 3.0 in bfloat16 */
        
        __m512bh blended1 = _mm512_mask_blend_epi16(mask1, a, b);
        __m512bh blended2 = _mm512_mask_blend_epi16(mask2, blended1, a);
        
        /* Extract and accumulate */
        volatile unsigned short temp[32];
        _mm512_storeu_epi16((void*)temp, (__m512i)blended2);
        for (int j = 0; j < 32; j++) {
            result += temp[j];
        }
    }
    
    return result;
}

#endif /* __AVX512BW__ */

/* V16SImode - 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        /* Comparison mask */
        __m512i vec1 = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec2 = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec1, vec2);
        
        /* Blend operations */
        __m512i a = _mm512_set1_epi32(i);
        __m512i b = _mm512_set1_epi32(i * 4);
        
        __m512i blended1 = _mm512_mask_blend_epi32(mask1, a, b);
        __m512i blended2 = _mm512_mask_blend_epi32(mask2, blended1, a);
        __m512i blended3 = _mm512_mask_blend_epi32(mask3, blended2, b);
        
        /* Reduce and accumulate */
        result += _mm512_reduce_add_epi32(blended3);
    }
    
    return result;
}

/* V8DImode - 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        /* Comparison mask */
        __m512i vec1 = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec2 = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec1, vec2);
        
        /* Blend operations */
        __m512i a = _mm512_set1_epi64(i);
        __m512i b = _mm512_set1_epi64(i * 5);
        
        __m512i blended1 = _mm512_mask_blend_epi64(mask1, a, b);
        __m512i blended2 = _mm512_mask_blend_epi64(mask2, blended1, a);
        __m512i blended3 = _mm512_mask_blend_epi64(mask3, blended2, b);
        
        /* Extract and accumulate */
        volatile long long temp[8];
        _mm512_storeu_epi64((void*)temp, blended3);
        for (int j = 0; j < 8; j++) {
            result += (int)(temp[j] & 0xFFFFFFFF);
        }
    }
    
    return result;
}

/* V8DFmode - 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        /* Comparison mask */
        __m512d vec1 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec2 = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec1, vec2, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512d a = _mm512_set1_pd((double)i);
        __m512d b = _mm512_set1_pd((double)(i * 1.5));
        
        __m512d blended1 = _mm512_mask_blend_pd(mask1, a, b);
        __m512d blended2 = _mm512_mask_blend_pd(mask2, blended1, a);
        __m512d blended3 = _mm512_mask_blend_pd(mask3, blended2, b);
        
        /* Extract and accumulate */
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended3);
        for (int j = 0; j < 8; j++) {
            result += (int)temp[j];
        }
    }
    
    return result;
}

/* V16SFmode - 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        /* Comparison mask */
        __m512 vec1 = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512 vec2 = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec1, vec2, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512 a = _mm512_set1_ps((float)i);
        __m512 b = _mm512_set1_ps((float)(i * 2.0f));
        
        __m512 blended1 = _mm512_mask_blend_ps(mask1, a, b);
        __m512 blended2 = _mm512_mask_blend_ps(mask2, blended1, a);
        __m512 blended3 = _mm512_mask_blend_ps(mask3, blended2, b);
        
        /* Reduce and accumulate */
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended3);
        for (int j = 0; j < 16; j++) {
            result += (int)temp[j];
        }
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
    if (!__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
#ifdef __AVX512BW__
    if (!__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512BW not supported on this CPU\n");
        return 0;
    }
#endif
    
    const int iterations = 20;
    volatile int total_sum = 0;
    
    /* Determine which test to run based on command line */
    if (argc > 1) {
        if (strcmp(argv[1], "v64qi") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v64qi(iterations);
            }
        } else if (strcmp(argv[1], "v32hi") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v32hi(iterations);
            }
        } else if (strcmp(argv[1], "v32hf") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v32hf(iterations);
            }
        } else if (strcmp(argv[1], "v32bf") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v32bf(iterations);
            }
        } else if (strcmp(argv[1], "v16si") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v16si(iterations);
            }
        } else if (strcmp(argv[1], "v8di") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v8di(iterations);
            }
        } else if (strcmp(argv[1], "v8df") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v8df(iterations);
            }
        } else if (strcmp(argv[1], "v16sf") == 0) {
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v16sf(iterations);
            }
        } else if (strcmp(argv[1], "all") == 0) {
            /* Run all tests */
            for (int i = 0; i < iterations; i++) {
                total_sum += test_blend_v64qi(iterations);
                total_sum += test_blend_v32hi(iterations);
                total_sum += test_blend_v32hf(iterations);
                total_sum += test_blend_v32bf(iterations);
                total_sum += test_blend_v16si(iterations);
                total_sum += test_blend_v8di(iterations);
                total_sum += test_blend_v8df(iterations);
                total_sum += test_blend_v16sf(iterations);
            }
        } else {
            printf("Usage: %s [v64qi|v32hi|v32hf|v32bf|v16si|v8di|v8df|v16sf|all]\n", argv[0]);
            return 1;
        }
    } else {
        /* Default: run all tests */
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v64qi(iterations);
            total_sum += test_blend_v32hi(iterations);
            total_sum += test_blend_v32hf(iterations);
            total_sum += test_blend_v32bf(iterations);
            total_sum += test_blend_v16si(iterations);
            total_sum += test_blend_v8di(iterations);
            total_sum += test_blend_v8df(iterations);
            total_sum += test_blend_v16sf(iterations);
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
