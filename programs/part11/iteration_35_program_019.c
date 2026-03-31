#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
volatile int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
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
        
        // Method 1: Constant mask pattern
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask based on iteration
        __mmask64 mask2 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        // Method 3: Mask from vector comparison
        __m512i cmp_vec = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(a, cmp_vec);
        
        // Use different masks in different iterations
        __mmask64 mask;
        if (i % 3 == 0) mask = mask1;
        else if (i % 3 == 1) mask = mask2;
        else mask = mask3;
        
        // Perform the blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Extract and accumulate result
        volatile uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
volatile int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors with incrementing values
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Generate mask in multiple ways
        __mmask32 mask;
        if (i % 2 == 0) {
            // Constant alternating pattern
            mask = 0xAAAAAAAA;
        } else {
            // Data-dependent mask
            mask = (__mmask32)(i * 0x55555555);
        }
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Extract and accumulate
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
volatile int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create half-precision float vectors
        __m512h a = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __m512h b = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask from comparison
        __m512h cmp_val = _mm512_set1_ph((float)(i % 16));
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_EQ_OQ);
        
        // Blend operation
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Extract and accumulate
        volatile uint16_t temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32BFmode: 32 x brain float16 */
volatile int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create bfloat16 vectors (using same layout as half-precision)
        __m512bh a = _mm512_set1_epi16(0x3C00); // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create alternating mask
        __mmask32 mask = (i % 2) ? 0xFFFFFFFF : 0xAAAAAAAA;
        
        // Blend operation
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Extract and accumulate
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, (__m512i)blended);
        result += temp[i % 32];
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode: 16 x 32-bit integers */
volatile int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Generate mask using different methods
        __mmask16 mask;
        if (i % 3 == 0) {
            mask = 0xAAAA;  // Alternating pattern
        } else if (i % 3 == 1) {
            mask = (__mmask16)(i * 0x5555);  // Data-dependent
        } else {
            // Mask from comparison
            __m512i cmp = _mm512_set1_epi32(i % 8);
            mask = _mm512_cmpeq_epi32_mask(a, cmp);
        }
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Extract and accumulate
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
volatile int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Generate mask
        __mmask8 mask;
        if (i % 2 == 0) {
            mask = 0xAA;  // Alternating
        } else {
            mask = (__mmask8)((i * 0x55) & 0xFF);
        }
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)(temp[i % 8] & 0xFFFFFFFF);
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
volatile int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask from comparison
        __m512d cmp = _mm512_set1_pd((double)(i % 4));
        __mmask8 mask = _mm512_cmp_pd_mask(a, cmp, _CMP_LT_OQ);
        
        // Blend operation
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Extract and accumulate
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
volatile int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Generate mask
        __mmask16 mask;
        if (i % 4 == 0) {
            mask = 0xAAAA;
        } else if (i % 4 == 1) {
            mask = 0x5555;
        } else if (i % 4 == 2) {
            mask = (__mmask16)(i * 0x3333);
        } else {
            __m512 cmp = _mm512_set1_ps((float)(i % 8));
            mask = _mm512_cmp_ps_mask(a, cmp, _CMP_GT_OQ);
        }
        
        // Blend operation
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Extract and accumulate
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif // __AVX512F__

// Dummy implementations for non-AVX512 builds
#ifndef __AVX512F__
volatile int test_blend_v64qi(int iter) { return 0; }
volatile int test_blend_v32hi(int iter) { return 0; }
volatile int test_blend_v32hf(int iter) { return 0; }
volatile int test_blend_v32bf(int iter) { return 0; }
volatile int test_blend_v16si(int iter) { return 0; }
volatile int test_blend_v8di(int iter) { return 0; }
volatile int test_blend_v8df(int iter) { return 0; }
volatile int test_blend_v16sf(int iter) { return 0; }
#endif

int main(int argc, char *argv[]) {
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    // Determine which test to run
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    volatile int total_result = 0;
    const int iterations = 50;  // Enough to trigger expansion
    
    // Run tests based on mode
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v64qi(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32hi(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32hf(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32bf(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16si(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8di(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8df(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16sf(iterations);
        }
    }
    
    printf("Total result: %d\n", total_result);
    return 0;
}
