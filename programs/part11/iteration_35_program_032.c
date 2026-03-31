#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
volatile int test_blend_v64qi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Method 1: Constant mask
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
        
        // Pattern: alternating bits (0xAAAAAAAAAAAAAAAA)
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask
        if (i % 2 == 0) {
            mask = 0x5555555555555555ULL;  // Alternate pattern
        }
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Extract and accumulate
        uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, result);
        sum += temp[i % 64];
    }
    
    return sum;
}

/* V32HImode: 32 x 16-bit integers */
volatile int test_blend_v32hi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Initialize with pattern
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        // Method 3: Mask from comparison
        __m512i cmp_val = _mm512_set1_epi16(16);
        __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, cmp_val);
        
        // Blend with comparison mask
        __m512i result = _mm512_mask_blend_epi16(cmp_mask, a, b);
        
        // Extract and accumulate
        int16_t temp[32];
        _mm512_storeu_si512((void*)temp, result);
        sum += temp[i % 32];
    }
    
    return sum;
}

/* V32HFmode: 32 x half-precision floats */
volatile int test_blend_v32hf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Initialize with pattern (using _Float16 if available)
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
        
        // Pattern mask
        __mmask32 mask = 0xAAAAAAAA;
        
        // Blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Extract and accumulate (store as half floats)
        _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, result);
        sum += (int)temp[i % 32];
    }
    
    return sum;
}

/* V32BFmode: 32 x bfloat16 floats */
volatile int test_blend_v32bf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Initialize bfloat16 vectors
        __m512bh a = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Alternating mask
        __mmask32 mask = (i % 2) ? 0x55555555 : 0xAAAAAAAA;
        
        // Blend
        __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Extract and accumulate
        uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, result);
        sum += temp[i % 32];
    }
    
    return sum;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode: 16 x 32-bit integers */
volatile int test_blend_v16si(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7,
            8, 9, 10, 11, 12, 13, 14, 15
        );
        
        // Method: Mask from comparison
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 cmp_mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        // Blend with dynamic mask
        __m512i result = _mm512_mask_blend_epi32(cmp_mask, a, b);
        
        // Extract and accumulate
        int32_t temp[16];
        _mm512_storeu_si512((void*)temp, result);
        sum += temp[i % 16];
    }
    
    return sum;
}

/* V8DImode: 8 x 64-bit integers */
volatile int test_blend_v8di(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        
        // Pattern mask
        __mmask8 mask = 0xAA;  // 0b10101010
        
        // Blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        int64_t temp[8];
        _mm512_storeu_si512((void*)temp, result);
        sum += (int)temp[i % 8];
    }
    
    return sum;
}

/* V8DFmode: 8 x double-precision floats */
volatile int test_blend_v8df(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        
        // Dynamic mask based on iteration
        __mmask8 mask = (i % 3 == 0) ? 0xFF : 0x55;
        
        // Blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Extract and accumulate
        double temp[8];
        _mm512_storeu_pd(temp, result);
        sum += (int)temp[i % 8];
    }
    
    return sum;
}

/* V16SFmode: 16 x single-precision floats */
volatile int test_blend_v16sf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 b = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        // Mask from comparison
        __m512 cmp_val = _mm512_set1_ps(7.5f);
        __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        
        // Blend with comparison mask
        __m512 result = _mm512_mask_blend_ps(cmp_mask, a, b);
        
        // Extract and accumulate
        float temp[16];
        _mm512_storeu_ps(temp, result);
        sum += (int)temp[i % 16];
    }
    
    return sum;
}

#endif // __AVX512F__

/* Fallback implementations for non-AVX512 builds */
#ifndef __AVX512F__
volatile int test_blend_v64qi(int iterations) { return 0; }
volatile int test_blend_v32hi(int iterations) { return 0; }
volatile int test_blend_v32hf(int iterations) { return 0; }
volatile int test_blend_v32bf(int iterations) { return 0; }
volatile int test_blend_v16si(int iterations) { return 0; }
volatile int test_blend_v8di(int iterations) { return 0; }
volatile int test_blend_v8df(int iterations) { return 0; }
volatile int test_blend_v16sf(int iterations) { return 0; }
#endif

int main(int argc, char *argv[]) {
    const int ITERATIONS = 50;
    volatile int total_sum = 0;
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    // Determine which test to run
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    printf("Running AVX-512 blend tests (mode: %s)\n", test_mode);
    
    // Run selected tests
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v64qi(ITERATIONS);
        } else {
            printf("Skipping V64QImode: AVX-512BW required\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hi(ITERATIONS);
        } else {
            printf("Skipping V32HImode: AVX-512BW required\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hf(ITERATIONS);
        } else {
            printf("Skipping V32HFmode: AVX-512BW required\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32bf(ITERATIONS);
        } else {
            printf("Skipping V32BFmode: AVX-512BW required\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        total_sum += test_blend_v16si(ITERATIONS);
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        total_sum += test_blend_v8di(ITERATIONS);
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        total_sum += test_blend_v8df(ITERATIONS);
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        total_sum += test_blend_v16sf(ITERATIONS);
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
