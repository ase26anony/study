#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
volatile int test_blend_v64qi(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Method 1: Constant mask */
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        /* Method 2: Data-dependent mask */
        __mmask64 mask2 = (__mmask64)(i * 0x5555555555555555ULL);
        
        /* Method 3: Comparison-based mask */
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
        
        __m512i vec_cmp = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, vec_cmp);
        
        /* Blend with different masks */
        __m512i result1 = _mm512_mask_blend_epi8(mask1, vec_a, vec_b);
        __m512i result2 = _mm512_mask_blend_epi8(mask2, vec_b, vec_a);
        __m512i result3 = _mm512_mask_blend_epi8(mask3, vec_a, vec_b);
        
        /* Extract and accumulate results */
        volatile uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[i % 64];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[(i + 1) % 64];
        
        _mm512_storeu_si512((void*)temp, result3);
        sum += temp[(i + 2) % 64];
    }
    
    return sum;
}

/* V32HImode - 32 x 16-bit integers */
volatile int test_blend_v32hi(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Multiple mask generation methods */
        __mmask32 mask1 = 0xAAAAAAAAU;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555U);
        
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi16(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32
        );
        
        __m512i vec_cmp = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, vec_cmp);
        
        /* Perform blends */
        __m512i result1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512i result2 = _mm512_mask_blend_epi16(mask2, vec_b, vec_a);
        __m512i result3 = _mm512_mask_blend_epi16(mask3, vec_a, vec_b);
        
        /* Extract results */
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[i % 32];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[(i + 1) % 32];
        
        _mm512_storeu_si512((void*)temp, result3);
        sum += temp[(i + 2) % 32];
    }
    
    return sum;
}

/* V32HFmode - 32 x half-precision floats */
#ifdef __AVX512FP16__
volatile int test_blend_v32hf(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAAU;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555U);
        
        /* Create half-precision vectors */
        _Float16 a_vals[32], b_vals[32];
        for (int j = 0; j < 32; j++) {
            a_vals[j] = (_Float16)(j);
            b_vals[j] = (_Float16)(j + 32);
        }
        
        __m512h vec_a = _mm512_set_ph(
            a_vals[31], a_vals[30], a_vals[29], a_vals[28], a_vals[27], a_vals[26], a_vals[25], a_vals[24],
            a_vals[23], a_vals[22], a_vals[21], a_vals[20], a_vals[19], a_vals[18], a_vals[17], a_vals[16],
            a_vals[15], a_vals[14], a_vals[13], a_vals[12], a_vals[11], a_vals[10], a_vals[9], a_vals[8],
            a_vals[7], a_vals[6], a_vals[5], a_vals[4], a_vals[3], a_vals[2], a_vals[1], a_vals[0]
        );
        
        __m512h vec_b = _mm512_set_ph(
            b_vals[31], b_vals[30], b_vals[29], b_vals[28], b_vals[27], b_vals[26], b_vals[25], b_vals[24],
            b_vals[23], b_vals[22], b_vals[21], b_vals[20], b_vals[19], b_vals[18], b_vals[17], b_vals[16],
            b_vals[15], b_vals[14], b_vals[13], b_vals[12], b_vals[11], b_vals[10], b_vals[9], b_vals[8],
            b_vals[7], b_vals[6], b_vals[5], b_vals[4], b_vals[3], b_vals[2], b_vals[1], b_vals[0]
        );
        
        /* Blend operations */
        __m512h result1 = _mm512_mask_blend_ph(mask1, vec_a, vec_b);
        __m512h result2 = _mm512_mask_blend_ph(mask2, vec_b, vec_a);
        
        /* Store and accumulate */
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, result1);
        sum += (int)temp[i % 32];
        
        _mm512_storeu_ph((void*)temp, result2);
        sum += (int)temp[(i + 1) % 32];
    }
    
    return sum;
}
#else
volatile int test_blend_v32hf(int iter) {
    return iter * 2;  /* Dummy implementation */
}
#endif

/* V32BFmode - 32 x bfloat16 floats */
#ifdef __AVX512BF16__
volatile int test_blend_v32bf(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAAU;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555U);
        
        /* Create bfloat16 vectors */
        __m512bh vec_a = _mm512_set1_epi16(0x3F80);  /* 1.0 in bfloat16 */
        __m512bh vec_b = _mm512_set1_epi16(0x4000);  /* 2.0 in bfloat16 */
        
        /* Blend operations */
        __m512bh result1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512bh result2 = _mm512_mask_blend_epi16(mask2, vec_b, vec_a);
        
        /* Store and accumulate */
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[i % 32];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[(i + 1) % 32];
    }
    
    return sum;
}
#else
volatile int test_blend_v32bf(int iter) {
    return iter * 3;  /* Dummy implementation */
}
#endif

#endif  /* __AVX512BW__ */
#endif  /* __AVX512F__ */

/* V16SImode - 16 x 32-bit integers */
#ifdef __AVX512F__
volatile int test_blend_v16si(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi32(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16
        );
        
        __m512i vec_cmp = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, vec_cmp);
        
        /* Blend operations */
        __m512i result1 = _mm512_mask_blend_epi32(mask1, vec_a, vec_b);
        __m512i result2 = _mm512_mask_blend_epi32(mask2, vec_b, vec_a);
        __m512i result3 = _mm512_mask_blend_epi32(mask3, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, result1);
        sum += temp[i % 16];
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += temp[(i + 1) % 16];
        
        _mm512_storeu_si512((void*)temp, result3);
        sum += temp[(i + 2) % 16];
    }
    
    return sum;
}

/* V8DImode - 8 x 64-bit integers */
volatile int test_blend_v8di(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        
        __m512i vec_cmp = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec_a, vec_cmp);
        
        /* Blend operations */
        __m512i result1 = _mm512_mask_blend_epi64(mask1, vec_a, vec_b);
        __m512i result2 = _mm512_mask_blend_epi64(mask2, vec_b, vec_a);
        __m512i result3 = _mm512_mask_blend_epi64(mask3, vec_a, vec_b);
        
        /* Extract and accumulate */
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, result1);
        sum += (int)(temp[i % 8] & 0xFFFFFFFF);
        
        _mm512_storeu_si512((void*)temp, result2);
        sum += (int)(temp[(i + 1) % 8] & 0xFFFFFFFF);
        
        _mm512_storeu_si512((void*)temp, result3);
        sum += (int)(temp[(i + 2) % 8] & 0xFFFFFFFF);
    }
    
    return sum;
}

/* V8DFmode - 8 x double-precision floats */
volatile int test_blend_v8df(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512d vec_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec_b = _mm512_set_pd(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        
        __m512d vec_cmp = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, vec_cmp, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512d result1 = _mm512_mask_blend_pd(mask1, vec_a, vec_b);
        __m512d result2 = _mm512_mask_blend_pd(mask2, vec_b, vec_a);
        __m512d result3 = _mm512_mask_blend_pd(mask3, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, result1);
        sum += (int)temp[i % 8];
        
        _mm512_storeu_pd((void*)temp, result2);
        sum += (int)temp[(i + 1) % 8];
        
        _mm512_storeu_pd((void*)temp, result3);
        sum += (int)temp[(i + 2) % 8];
    }
    
    return sum;
}

/* V16SFmode - 16 x single-precision floats */
volatile int test_blend_v16sf(int iter) {
    volatile int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 vec_b = _mm512_set_ps(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
        );
        
        __m512 vec_cmp = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, vec_cmp, _CMP_EQ_OQ);
        
        /* Blend operations */
        __m512 result1 = _mm512_mask_blend_ps(mask1, vec_a, vec_b);
        __m512 result2 = _mm512_mask_blend_ps(mask2, vec_b, vec_a);
        __m512 result3 = _mm512_mask_blend_ps(mask3, vec_a, vec_b);
        
        /* Store and accumulate */
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, result1);
        sum += (int)temp[i % 16];
        
        _mm512_storeu_ps((void*)temp, result2);
        sum += (int)temp[(i + 1) % 16];
        
        _mm512_storeu_ps((void*)temp, result3);
        sum += (int)temp[(i + 2) % 16];
    }
    
    return sum;
}
#endif  /* __AVX512F__ */

/* Dummy implementations for non-AVX512 builds */
#ifndef __AVX512F__
volatile int test_blend_v64qi(int iter) { return iter * 1; }
volatile int test_blend_v32hi(int iter) { return iter * 2; }
volatile int test_blend_v32hf(int iter) { return iter * 3; }
volatile int test_blend_v32bf(int iter) { return iter * 4; }
volatile int test_blend_v16si(int iter) { return iter * 5; }
volatile int test_blend_v8di(int iter) { return iter * 6; }
volatile int test_blend_v8df(int iter) { return iter * 7; }
volatile int test_blend_v16sf(int iter) { return iter * 8; }
#endif

/* Main driver function */
int main(int argc, char *argv[]) {
    const int ITERATIONS = 50;
    volatile int total_sum = 0;
    
    /* Check CPU support at runtime */
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    printf("AVX-512F supported: %s\n", has_avx512f ? "YES" : "NO");
    printf("AVX-512BW supported: %s\n", has_avx512bw ? "YES" : "NO");
    
    /* Determine which test to run based on command line */
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    /* Run selected tests in loops to trigger expansion */
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v64qi(ITERATIONS);
        }
        printf("V64QImode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v32hi(ITERATIONS);
        }
        printf("V32HImode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v32hf(ITERATIONS);
        }
        printf("V32HFmode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v32bf(ITERATIONS);
        }
        printf("V32BFmode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v16si(ITERATIONS);
        }
        printf("V16SImode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v8di(ITERATIONS);
        }
        printf("V8DImode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v8df(ITERATIONS);
        }
        printf("V8DFmode tests completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_sum += test_blend_v16sf(ITERATIONS);
        }
        printf("V16SFmode tests completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
