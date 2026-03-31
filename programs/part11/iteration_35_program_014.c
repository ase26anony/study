#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
volatile int test_blend_v64qi(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors with different patterns
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
        
        // Method 1: Constant mask pattern
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = (__mmask64)(i & 0xFF);
        
        // Method 3: Comparison-based mask
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Use different masks in different iterations
        __mmask64 mask;
        if (i % 3 == 0) mask = mask1;
        else if (i % 3 == 1) mask = mask2;
        else mask = mask3;
        
        // Perform the blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Extract and accumulate result
        volatile char temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode - 32 x 16-bit integers */
volatile int test_blend_v32hi(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors with arithmetic progression
        __m512i a = _mm512_set_epi16(
            0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
            160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310
        );
        
        __m512i b = _mm512_set_epi16(
            310, 300, 290, 280, 270, 260, 250, 240, 230, 220, 210, 200, 190, 180, 170, 160,
            150, 140, 130, 120, 110, 100, 90, 80, 70, 60, 50, 40, 30, 20, 10, 0
        );
        
        // Pattern-based mask
        __mmask32 mask1 = 0x55555555;
        
        // Data-dependent mask
        __mmask32 mask2 = (__mmask32)((i * 7) & 0xFFFFFFFF);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi16(150);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(a, cmp_val);
        
        __mmask32 mask;
        if (i % 4 == 0) mask = mask1;
        else if (i % 4 == 1) mask = mask2;
        else mask = mask3;
        
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Extract and accumulate
        volatile short temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode - 32 x half-precision floats */
volatile int test_blend_v32hf(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create half-precision float vectors
        __m512h a = _mm512_set_ph(
            0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f,
            4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f,
            8.0f, 8.5f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 11.5f,
            12.0f, 12.5f, 13.0f, 13.5f, 14.0f, 14.5f, 15.0f, 15.5f
        );
        
        __m512h b = _mm512_set_ph(
            15.5f, 15.0f, 14.5f, 14.0f, 13.5f, 13.0f, 12.5f, 12.0f,
            11.5f, 11.0f, 10.5f, 10.0f, 9.5f, 9.0f, 8.5f, 8.0f,
            7.5f, 7.0f, 6.5f, 6.0f, 5.5f, 5.0f, 4.5f, 4.0f,
            3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f, 0.0f
        );
        
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Extract and accumulate
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)(temp[i % 32] * 10);
    }
    
    return result;
}

/* V32BFmode - 32 x bfloat16 floats */
volatile int test_blend_v32bf(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create bfloat16 vectors
        __m512bh a = _mm512_set_ph(
            0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f,
            2.0f, 2.25f, 2.5f, 2.75f, 3.0f, 3.25f, 3.5f, 3.75f,
            4.0f, 4.25f, 4.5f, 4.75f, 5.0f, 5.25f, 5.5f, 5.75f,
            6.0f, 6.25f, 6.5f, 6.75f, 7.0f, 7.25f, 7.5f, 7.75f
        );
        
        __m512bh b = _mm512_set_ph(
            7.75f, 7.5f, 7.25f, 7.0f, 6.75f, 6.5f, 6.25f, 6.0f,
            5.75f, 5.5f, 5.25f, 5.0f, 4.75f, 4.5f, 4.25f, 4.0f,
            3.75f, 3.5f, 3.25f, 3.0f, 2.75f, 2.5f, 2.25f, 2.0f,
            1.75f, 1.5f, 1.25f, 1.0f, 0.75f, 0.5f, 0.25f, 0.0f
        );
        
        __mmask32 mask = 0x55555555; // Alternating pattern (opposite of v32hf)
        
        __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Extract and accumulate
        volatile __bf16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)(*(float*)&temp[i % 32] * 10);
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode - 16 x 32-bit integers */
volatile int test_blend_v16si(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi32(
            0, 100, 200, 300, 400, 500, 600, 700,
            800, 900, 1000, 1100, 1200, 1300, 1400, 1500
        );
        
        __m512i b = _mm512_set_epi32(
            1500, 1400, 1300, 1200, 1100, 1000, 900, 800,
            700, 600, 500, 400, 300, 200, 100, 0
        );
        
        // Multiple mask generation strategies
        __mmask16 mask1 = 0xAAAA; // 1010101010101010
        
        __mmask16 mask2 = (__mmask16)((i * 13) & 0xFFFF);
        
        __m512i cmp_val = _mm512_set1_epi32(750);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        __mmask16 mask;
        switch (i % 5) {
            case 0: mask = mask1; break;
            case 1: mask = mask2; break;
            case 2: mask = mask3; break;
            case 3: mask = mask1 | mask2; break;
            default: mask = mask1 & mask3; break;
        }
        
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Extract and accumulate
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode - 8 x 64-bit integers */
volatile int test_blend_v8di(int iterations) {
    volatile long long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_set_epi64(
            0, 1000, 2000, 3000, 4000, 5000, 6000, 7000
        );
        
        __m512i b = _mm512_set_epi64(
            7000, 6000, 5000, 4000, 3000, 2000, 1000, 0
        );
        
        __mmask8 mask1 = 0xAA; // 10101010
        
        __mmask8 mask2 = (__mmask8)((i * 17) & 0xFF);
        
        __m512i cmp_val = _mm512_set1_epi64(3500);
        __mmask8 mask3 = _mm512_cmpgt_epi64_mask(a, cmp_val);
        
        __mmask8 mask;
        if (i % 2 == 0) mask = mask1;
        else mask = mask3;
        
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 8];
    }
    
    return (int)result;
}

/* V8DFmode - 8 x double-precision floats */
volatile int test_blend_v8df(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_set_pd(
            0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7
        );
        
        __m512d b = _mm512_set_pd(
            7.7, 6.6, 5.5, 4.4, 3.3, 2.2, 1.1, 0.0
        );
        
        __mmask8 mask1 = 0x55; // 01010101
        
        __mmask8 mask2 = (__mmask8)((i * 11) & 0xFF);
        
        __m512d cmp_val = _mm512_set1_pd(3.85);
        __mmask8 mask3 = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        
        __mmask8 mask;
        if (i % 3 == 0) mask = mask1;
        else if (i % 3 == 1) mask = mask2;
        else mask = mask3;
        
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Extract and accumulate
        volatile double temp[8];
        _mm512_storeu_pd(temp, blended);
        result += (int)(temp[i % 8] * 10);
    }
    
    return result;
}

/* V16SFmode - 16 x single-precision floats */
volatile int test_blend_v16sf(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_set_ps(
            0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f,
            4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f
        );
        
        __m512 b = _mm512_set_ps(
            7.5f, 7.0f, 6.5f, 6.0f, 5.5f, 5.0f, 4.5f, 4.0f,
            3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f, 0.0f
        );
        
        __mmask16 mask1 = 0xAAAA;
        
        __mmask16 mask2 = (__mmask16)((i * 19) & 0xFFFF);
        
        __m512 cmp_val = _mm512_set1_ps(3.75f);
        __mmask16 mask3 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        
        __mmask16 mask;
        switch (i % 4) {
            case 0: mask = mask1; break;
            case 1: mask = mask2; break;
            case 2: mask = mask3; break;
            default: mask = ~mask1; break;
        }
        
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Extract and accumulate
        volatile float temp[16];
        _mm512_storeu_ps(temp, blended);
        result += (int)(temp[i % 16] * 10);
    }
    
    return result;
}

#endif // __AVX512F__

// Dummy implementations for non-AVX512 builds
#ifndef __AVX512F__
volatile int test_blend_v16si(int iterations) { return 0; }
volatile int test_blend_v8di(int iterations) { return 0; }
volatile int test_blend_v8df(int iterations) { return 0; }
volatile int test_blend_v16sf(int iterations) { return 0; }
#endif

#ifndef __AVX512BW__
volatile int test_blend_v64qi(int iterations) { return 0; }
volatile int test_blend_v32hi(int iterations) { return 0; }
volatile int test_blend_v32hf(int iterations) { return 0; }
volatile int test_blend_v32bf(int iterations) { return 0; }
#endif

int main(int argc, char *argv[]) {
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    const int iterations = 50;
    volatile int total_sum = 0;
    
    // Determine which test to run
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    printf("Running AVX-512 blend tests (iterations: %d)\n", iterations);
    
    // Run selected tests in a loop
    for (int loop = 0; loop < 10; loop++) {
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
            if (has_avx512bw) {
                total_sum += test_blend_v64qi(iterations);
            }
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
            if (has_avx512bw) {
                total_sum += test_blend_v32hi(iterations);
            }
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
            if (has_avx512bw) {
                total_sum += test_blend_v32hf(iterations);
            }
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
            if (has_avx512bw) {
                total_sum += test_blend_v32bf(iterations);
            }
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
            total_sum += test_blend_v16si(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
            total_sum += test_blend_v8di(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
            total_sum += test_blend_v8df(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
            total_sum += test_blend_v16sf(iterations);
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
