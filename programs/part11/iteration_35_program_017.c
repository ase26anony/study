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
        /* Method 1: Constant mask blend */
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
        
        /* Pattern: alternating bytes (0xAAAAAAAAAAAAAAAA) */
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        /* Method 2: Data-dependent mask */
        __m512i cmp_val = _mm512_set1_epi8(i & 0xFF);
        __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(a, cmp_val);
        __m512i blended2 = _mm512_mask_blend_epi8(cmp_mask, a, b);
        
        /* Extract and accumulate results */
        volatile char temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[0] + temp[63];
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += temp[32];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Initialize vectors with pattern */
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        /* Method 1: Constant mask (alternating words) */
        __mmask32 mask = 0xAAAAAAAA;
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        /* Method 2: Comparison-based mask */
        __m512i threshold = _mm512_set1_epi16(16);
        __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, threshold);
        __m512i blended2 = _mm512_mask_blend_epi16(cmp_mask, a, b);
        
        /* Extract results */
        volatile short temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[0] + temp[31];
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += temp[16];
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
        
        /* Use comparison for mask generation */
        __m512h cmp_val = _mm512_set1_ph(15.5f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        /* Store and accumulate */
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)temp[0] + (int)temp[31];
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Initialize bfloat16 vectors */
        __m512bh a = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
        __m512bh b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
        
        /* Create alternating mask */
        __mmask32 mask = 0x55555555;
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        /* Store and extract */
        volatile unsigned short temp[32];
        _mm512_storeu_si512((void*)temp, (__m512i)blended);
        result += temp[0] + temp[31];
    }
    
    return result;
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7,
            8, 9, 10, 11, 12, 13, 14, 15
        );
        
        /* Method 1: Constant mask */
        __mmask16 mask = 0xAAAA;
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        /* Method 2: Comparison mask */
        __m512i cmp_val = _mm512_set1_epi32(7);
        __mmask16 cmp_mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
        __m512i blended2 = _mm512_mask_blend_epi32(cmp_mask, a, b);
        
        /* Extract results */
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[0] + temp[15];
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += temp[8];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        
        /* Pattern mask */
        __mmask8 mask = 0xAA;
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        /* Data-dependent mask */
        __m512i cmp_val = _mm512_set1_epi64(3);
        __mmask8 cmp_mask = _mm512_cmpgt_epi64_mask(a, cmp_val);
        __m512i blended2 = _mm512_mask_blend_epi64(cmp_mask, a, b);
        
        /* Extract and accumulate */
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)(temp[0] + temp[7]);
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += (int)temp[4];
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        
        /* Constant mask blend */
        __mmask8 mask = 0x55;
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        /* Comparison-based mask */
        __m512d threshold = _mm512_set1_pd(3.5);
        __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, threshold, _CMP_GT_OQ);
        __m512d blended2 = _mm512_mask_blend_pd(cmp_mask, a, b);
        
        /* Store and extract */
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended);
        result += (int)temp[0] + (int)temp[7];
        
        _mm512_storeu_pd((void*)temp, blended2);
        result += (int)temp[3];
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
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        /* Alternating mask pattern */
        __mmask16 mask = 0xAAAA;
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        /* Data-dependent mask */
        __m512 threshold = _mm512_set1_ps(7.5f);
        __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
        __m512 blended2 = _mm512_mask_blend_ps(cmp_mask, a, b);
        
        /* Extract results */
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended);
        result += (int)temp[0] + (int)temp[15];
        
        _mm512_storeu_ps((void*)temp, blended2);
        result += (int)temp[8];
    }
    
    return result;
}

#else /* __AVX512F__ not defined */

/* Dummy implementations for non-AVX512 compilation */
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
        printf("AVX-512BW not supported, some tests will be limited\n");
    }
    
    const int iterations = 20;
    volatile int accumulator = 0;
    
    /* Determine which test to run based on command line */
    char *test_to_run = NULL;
    if (argc > 1) {
        test_to_run = argv[1];
    }
    
    /* Run selected test or all tests */
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0) {
        printf("Running all AVX-512 blend tests...\n");
        
        if (has_avx512bw) {
            accumulator += test_blend_v64qi(iterations);
            accumulator += test_blend_v32hi(iterations);
            accumulator += test_blend_v32hf(iterations);
            accumulator += test_blend_v32bf(iterations);
        }
        
        accumulator += test_blend_v16si(iterations);
        accumulator += test_blend_v8di(iterations);
        accumulator += test_blend_v8df(iterations);
        accumulator += test_blend_v16sf(iterations);
    }
    else if (strcmp(test_to_run, "v64qi") == 0 && has_avx512bw) {
        accumulator += test_blend_v64qi(iterations * 5);
    }
    else if (strcmp(test_to_run, "v32hi") == 0 && has_avx512bw) {
        accumulator += test_blend_v32hi(iterations * 5);
    }
    else if (strcmp(test_to_run, "v32hf") == 0 && has_avx512bw) {
        accumulator += test_blend_v32hf(iterations * 5);
    }
    else if (strcmp(test_to_run, "v32bf") == 0 && has_avx512bw) {
        accumulator += test_blend_v32bf(iterations * 5);
    }
    else if (strcmp(test_to_run, "v16si") == 0) {
        accumulator += test_blend_v16si(iterations * 5);
    }
    else if (strcmp(test_to_run, "v8di") == 0) {
        accumulator += test_blend_v8di(iterations * 5);
    }
    else if (strcmp(test_to_run, "v8df") == 0) {
        accumulator += test_blend_v8df(iterations * 5);
    }
    else if (strcmp(test_to_run, "v16sf") == 0) {
        accumulator += test_blend_v16sf(iterations * 5);
    }
    else {
        printf("Unknown test or feature not supported: %s\n", test_to_run);
        printf("Available tests: v64qi, v32hi, v32hf, v32bf, v16si, v8di, v8df, v16sf, all\n");
        return 1;
    }
    
    printf("Accumulated result: %d\n", accumulator);
    return 0;
}
