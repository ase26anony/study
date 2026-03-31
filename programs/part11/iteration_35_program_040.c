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
        __m512i a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi8(
            127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112,
            111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96,
            95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
            79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64
        );
        
        /* Alternating pattern mask: 0xAAAAAAAAAAAAAAAA */
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Blend operation */
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
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
        /* Method 2: Data-dependent mask generation */
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32
        );
        
        /* Generate mask based on iteration count */
        __mmask32 mask = (i % 2) ? 0xFFFFFFFF : 0xAAAAAAAA;
        
        /* Blend operation */
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        /* Extract and accumulate */
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
        /* Initialize with simple pattern */
        __m512h a = _mm512_set_ph(
            1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
            17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
            25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
        );
        
        __m512h b = _mm512_set_ph(
            33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f,
            41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f,
            49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f,
            57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f, 64.0f
        );
        
        /* Simple alternating mask */
        __mmask32 mask = 0x55555555;
        
        /* Blend operation */
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
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Initialize bfloat16 vectors */
        __m512bh a = _mm512_set_epi16(
            0x3C00, 0x4000, 0x4200, 0x4400, 0x4500, 0x4600, 0x4700, 0x4800,
            0x4880, 0x4900, 0x4980, 0x4A00, 0x4A80, 0x4B00, 0x4B80, 0x4C00,
            0x3C00, 0x4000, 0x4200, 0x4400, 0x4500, 0x4600, 0x4700, 0x4800,
            0x4880, 0x4900, 0x4980, 0x4A00, 0x4A80, 0x4B00, 0x4B80, 0x4C00
        );
        
        __m512bh b = _mm512_set_epi16(
            0x4C80, 0x4D00, 0x4D80, 0x4E00, 0x4E80, 0x4F00, 0x4F80, 0x5000,
            0x5040, 0x5080, 0x50C0, 0x5100, 0x5140, 0x5180, 0x51C0, 0x5200,
            0x4C80, 0x4D00, 0x4D80, 0x4E00, 0x4E80, 0x4F00, 0x4F80, 0x5000,
            0x5040, 0x5080, 0x50C0, 0x5100, 0x5140, 0x5180, 0x51C0, 0x5200
        );
        
        /* Pattern mask */
        __mmask32 mask = 0x33333333;
        
        /* Blend operation */
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        /* Extract and accumulate */
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
        /* Method 3: Mask from vector comparison */
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16
        );
        
        /* Create comparison mask */
        __m512i cmp_val = _mm512_set1_epi32(i);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        /* Blend operation */
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
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
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        
        /* Dynamic mask based on iteration */
        __mmask8 mask = (1 << (i % 8)) - 1;
        
        /* Blend operation */
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
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
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        
        /* Pattern mask */
        __mmask8 mask = 0xAA; /* 0b10101010 */
        
        /* Blend operation */
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
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
        __m512 a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 b = _mm512_set_ps(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
        );
        
        /* Complex pattern mask */
        __mmask16 mask = (0xAAAA >> (i % 4)) | 0x5555;
        
        /* Blend operation */
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
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
    
    const int iterations = 50;
    volatile int total_sum = 0;
    
    /* Determine which test to run based on command line */
    const char *test_to_run = (argc > 1) ? argv[1] : "all";
    
    if (strcmp(test_to_run, "v64qi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v64qi(iterations);
            }
            printf("V64QImode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v32hi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hi(iterations);
            }
            printf("V32HImode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v32hf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hf(iterations);
            }
            printf("V32HFmode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v32bf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32bf(iterations);
            }
            printf("V32BFmode test completed\n");
        }
    }
    
    if (strcmp(test_to_run, "v16si") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16si(iterations);
        }
        printf("V16SImode test completed\n");
    }
    
    if (strcmp(test_to_run, "v8di") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8di(iterations);
        }
        printf("V8DImode test completed\n");
    }
    
    if (strcmp(test_to_run, "v8df") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8df(iterations);
        }
        printf("V8DFmode test completed\n");
    }
    
    if (strcmp(test_to_run, "v16sf") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16sf(iterations);
        }
        printf("V16SFmode test completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
