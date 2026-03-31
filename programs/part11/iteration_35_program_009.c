#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64-byte integers */
volatile int test_blend_v64qi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create two vectors with different patterns
        __m512i a = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        __m512i b = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Method 1: Constant mask pattern (alternating bits)
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAA;
        
        // Method 2: Data-dependent mask based on iteration
        __mmask64 mask2 = (i % 2) ? 0xFFFFFFFFFFFFFFFF : 0xAAAAAAAAAAAAAAAA;
        
        // Method 3: Mask from comparison
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Combine masks for complex pattern
        __mmask64 final_mask = mask1 ^ mask2 | mask3;
        
        // Perform the blend operation
        __m512i result = _mm512_mask_blend_epi8(final_mask, a, b);
        
        // Extract and accumulate to prevent optimization
        volatile char temp[64];
        _mm512_storeu_epi8((void*)temp, result);
        sum += temp[i % 64];
    }
    
    return sum;
}

/* V32HImode: 32 half-word integers */
volatile int test_blend_v32hi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors with sequential values
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask using different methods
        __mmask32 mask1 = 0xAAAAAAAA;  // Alternating pattern
        
        // Data-dependent mask
        __mmask32 mask2 = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        
        // Comparison-based mask
        __m512i threshold = _mm512_set1_epi16(16);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(a, threshold);
        
        // Complex mask combination
        __mmask32 final_mask = (mask1 & mask2) | mask3;
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
        
        // Extract and accumulate
        volatile short temp[32];
        _mm512_storeu_epi16((void*)temp, result);
        sum += temp[i % 32];
    }
    
    return sum;
}

/* V32HFmode: 32 half-precision floats */
volatile int test_blend_v32hf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create half-precision float vectors
        // Note: Using _mm512_set_ph requires AVX-512FP16, so we'll use integer representation
        __m512h a, b;
        
        // Initialize with simple patterns using integer representation
        // This is a workaround for compilers without full FP16 support
        unsigned short pattern_a[32];
        unsigned short pattern_b[32];
        
        for (int j = 0; j < 32; j++) {
            pattern_a[j] = (j + 1) * 0x3C00;  // 1.0, 2.0, 3.0...
            pattern_b[j] = (32 - j) * 0x3C00; // 32.0, 31.0, 30.0...
        }
        
        // Load patterns (assuming compiler supports __m512h)
        #ifdef __AVX512FP16__
        a = _mm512_loadu_ph((void*)pattern_a);
        b = _mm512_loadu_ph((void*)pattern_b);
        
        // Create mask
        __mmask32 mask = 0x55555555;  // Alternating pattern
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store and accumulate
        volatile unsigned short temp[32];
        _mm512_storeu_ph((void*)temp, result);
        sum += temp[i % 32];
        #else
        // Fallback for compilers without FP16 support
        sum += i;
        #endif
    }
    
    return sum;
}

/* V32BFmode: 32 brain-float (bfloat16) */
volatile int test_blend_v32bf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Similar to HF but for bfloat16
        // Using integer representation as workaround
        unsigned short pattern_a[32];
        unsigned short pattern_b[32];
        
        for (int j = 0; j < 32; j++) {
            pattern_a[j] = (j % 8) << 8;  // Simple pattern
            pattern_b[j] = ((7 - (j % 8)) << 8) | 0x80;
        }
        
        #ifdef __AVX512BF16__
        // Load as __m512bh
        __m512bh a = _mm512_loadu_epi16((void*)pattern_a);
        __m512bh b = _mm512_loadu_epi16((void*)pattern_b);
        
        // Create mask
        __mmask32 mask = (i % 2) ? 0xFFFFFFFF : 0xAAAAAAAA;
        
        // Blend operation for bfloat16
        __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Store and accumulate
        volatile unsigned short temp[32];
        _mm512_storeu_epi16((void*)temp, result);
        sum += temp[i % 32];
        #else
        sum += i * 2;
        #endif
    }
    
    return sum;
}

#endif  /* __AVX512BW__ */

/* V16SImode: 16 single-word integers */
volatile int test_blend_v16si(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Multiple mask generation methods
        __mmask16 mask1 = 0xAAAA;  // Binary 1010101010101010
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x3333) & 0xFFFF);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        // Combine masks
        __mmask16 final_mask = mask1 ^ mask2 | mask3;
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(final_mask, a, b);
        
        // Extract and accumulate
        volatile int temp[16];
        _mm512_storeu_epi32((void*)temp, result);
        sum += temp[i % 16];
    }
    
    return sum;
}

/* V8DImode: 8 double-word integers */
volatile int test_blend_v8di(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask
        __mmask8 mask = 0xAA;  // Binary 10101010
        
        // Modify mask based on iteration
        if (i % 3 == 0) mask = 0x55;  // 01010101
        else if (i % 3 == 1) mask = 0xF0;  // 11110000
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_epi64((void*)temp, result);
        sum += (int)(temp[i % 8] & 0xFFFFFFFF);
    }
    
    return sum;
}

/* V8DFmode: 8 double-precision floats */
volatile int test_blend_v8df(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask using different patterns
        __mmask8 mask1 = 0xAA;
        
        // Data-dependent mask
        __mmask8 mask2 = (i % 4 == 0) ? 0xFF : 0x00;
        
        // Comparison-based mask
        __m512d threshold = _mm512_set1_pd(3.5);
        __mmask8 mask3 = _mm512_cmp_pd_mask(a, threshold, _CMP_GT_OQ);
        
        // Combine masks
        __mmask8 final_mask = mask1 ^ mask2 | mask3;
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(final_mask, a, b);
        
        // Extract and accumulate
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, result);
        sum += (int)(temp[i % 8] * 100);
    }
    
    return sum;
}

/* V16SFmode: 16 single-precision floats */
volatile int test_blend_v16sf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Multiple mask patterns
        __mmask16 mask1 = 0xAAAA;
        
        // Pattern based on iteration
        __mmask16 mask2 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        // Comparison mask
        __m512 threshold = _mm512_set1_ps(7.5f);
        __mmask16 mask3 = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
        
        // Complex mask combination
        __mmask16 final_mask = (mask1 & ~mask2) | mask3;
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(final_mask, a, b);
        
        // Extract and accumulate
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, result);
        sum += (int)(temp[i % 16] * 10);
    }
    
    return sum;
}

#else  /* __AVX512F__ not defined */

// Dummy implementations for non-AVX512 builds
volatile int test_blend_v64qi(int iterations) { return 0; }
volatile int test_blend_v32hi(int iterations) { return 0; }
volatile int test_blend_v32hf(int iterations) { return 0; }
volatile int test_blend_v32bf(int iterations) { return 0; }
volatile int test_blend_v16si(int iterations) { return 0; }
volatile int test_blend_v8di(int iterations) { return 0; }
volatile int test_blend_v8df(int iterations) { return 0; }
volatile int test_blend_v16sf(int iterations) { return 0; }

#endif  /* __AVX512F__ */

int main(int argc, char *argv[]) {
    const int DEFAULT_ITERATIONS = 50;
    const char *test_mode = (argc > 1) ? argv[1] : "all";
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (strstr(test_mode, "64qi") || strstr(test_mode, "bw")) {
        if (!has_avx512bw) {
            printf("AVX-512BW required for V64QImode/V32HImode tests\n");
        }
    }
    
    volatile int total_sum = 0;
    
    // Run tests based on command line argument
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v64qi(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v32hi(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v32hf(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v32bf(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16si(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8di(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8df(DEFAULT_ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16sf(DEFAULT_ITERATIONS);
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
