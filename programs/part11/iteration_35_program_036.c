#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        
        __m512i vec_b = _mm512_set_epi8(
            127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112,
            111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96,
            95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
            79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64
        );
        
        __m512i cmp_result = _mm512_cmpeq_epi8_mask(vec_a, _mm512_set1_epi8(i & 0xFF));
        __mmask64 mask3 = cmp_result;
        
        /* Use different masks in different iterations */
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        /* Perform the blend operation */
        __m512i blended = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
        
        /* Extract and accumulate result */
        volatile char temp[64];
        _mm512_storeu_epi8((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Method 1: Constant mask */
        __mmask32 mask1 = 0xAAAAAAAA;
        
        /* Method 2: Patterned mask */
        __mmask32 mask2 = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        
        /* Create vectors */
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi16(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32
        );
        
        /* Method 3: Comparison mask */
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, _mm512_set1_epi16(i & 0xFFFF));
        
        /* Rotate through masks */
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        /* Perform blend */
        __m512i blended = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
        
        /* Extract result */
        volatile short temp[32];
        _mm512_storeu_epi16((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Create half-precision vectors */
        __m512h vec_a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512h vec_b = _mm512_set_ph(
            63.0f, 62.0f, 61.0f, 60.0f, 59.0f, 58.0f, 57.0f, 56.0f,
            55.0f, 54.0f, 53.0f, 52.0f, 51.0f, 50.0f, 49.0f, 48.0f,
            47.0f, 46.0f, 45.0f, 44.0f, 43.0f, 42.0f, 41.0f, 40.0f,
            39.0f, 38.0f, 37.0f, 36.0f, 35.0f, 34.0f, 33.0f, 32.0f
        );
        
        /* Create mask */
        __mmask32 mask = (__mmask32)((0xAAAAAAAA ^ (i * 0x11111111)) & 0xFFFFFFFF);
        
        /* Perform blend */
        __m512h blended = _mm512_mask_blend_ph(mask, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile _Float16 temp[32];
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
        /* Create bfloat16 vectors */
        __m512bh vec_a = _mm512_set1_epi16(0x3C00); /* 1.0 in bfloat16 */
        __m512bh vec_b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
        
        /* Create varying mask */
        __mmask32 mask = (__mmask32)((0x55555555 * (i + 1)) & 0xFFFFFFFF);
        
        /* Perform blend - using integer blend since bfloat16 blend may not be available */
        __m512bh blended = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile unsigned short temp[32];
        _mm512_storeu_epi16((void*)temp, (__m512i)blended);
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
        /* Method 1: Constant mask */
        __mmask16 mask1 = 0xAAAA;
        
        /* Create vectors */
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi32(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16
        );
        
        /* Method 2: Comparison mask */
        __mmask16 mask2 = _mm512_cmpeq_epi32_mask(vec_a, _mm512_set1_epi32(i));
        
        /* Alternate masks */
        __mmask16 mask = (i % 2 == 0) ? mask1 : mask2;
        
        /* Perform blend */
        __m512i blended = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile int temp[16];
        _mm512_storeu_epi32((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Create mask */
        __mmask8 mask = (__mmask8)((0xAA ^ (i * 0x55)) & 0xFF);
        
        /* Create vectors */
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        
        /* Perform blend */
        __m512i blended = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile long long temp[8];
        _mm512_storeu_epi64((void*)temp, blended);
        result += (int)(temp[i % 8] & 0xFFFFFFFF);
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Create mask with pattern */
        __mmask8 mask = (__mmask8)((0x55 * (i + 1)) & 0xFF);
        
        /* Create vectors */
        __m512d vec_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec_b = _mm512_set_pd(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        
        /* Perform blend */
        __m512d blended = _mm512_mask_blend_pd(mask, vec_a, vec_b);
        
        /* Store and accumulate */
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
        /* Method 1: Constant mask */
        __mmask16 mask1 = 0xAAAA;
        
        /* Create vectors */
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 vec_b = _mm512_set_ps(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
        );
        
        /* Method 2: Comparison mask */
        __m512 cmp_vec = _mm512_set1_ps((float)i);
        __mmask16 mask2 = _mm512_cmp_ps_mask(vec_a, cmp_vec, _CMP_EQ_OQ);
        
        /* Alternate masks */
        __mmask16 mask = (i % 2 == 0) ? mask1 : mask2;
        
        /* Perform blend */
        __m512 blended = _mm512_mask_blend_ps(mask, vec_a, vec_b);
        
        /* Store and accumulate */
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
    
    volatile int total_result = 0;
    const int iterations = 50;
    
    /* Determine which test to run based on command line */
    char *test_to_run = (argc > 1) ? argv[1] : "all";
    
    if (strcmp(test_to_run, "v64qi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v64qi(iterations);
            }
            printf("V64QImode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v32hi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32hi(iterations);
            }
            printf("V32HImode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v32hf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32hf(iterations);
            }
            printf("V32HFmode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v32bf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32bf(iterations);
            }
            printf("V32BFmode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v16si") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16si(iterations);
        }
        printf("V16SImode test completed\n");
    }
    
    if (strcmp(test_to_run, "v8di") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8di(iterations);
        }
        printf("V8DImode test completed\n");
    }
    
    if (strcmp(test_to_run, "v8df") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8df(iterations);
        }
        printf("V8DFmode test completed\n");
    }
    
    if (strcmp(test_to_run, "v16sf") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16sf(iterations);
        }
        printf("V16SFmode test completed\n");
    }
    
    printf("Total accumulated result: %d\n", total_result);
    
    return 0;
}
