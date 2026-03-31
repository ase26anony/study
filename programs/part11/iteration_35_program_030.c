#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
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
        
        // Method 1: Constant mask pattern (alternating bits)
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAA;
        
        // Method 2: Data-dependent mask based on iteration
        __mmask64 mask2 = (i % 2 == 0) ? 0xFFFFFFFFFFFFFFFF : 0xAAAAAAAAAAAAAAAA;
        
        // Method 3: Mask from vector comparison
        __m512i cmp_vec = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(a, cmp_vec);
        
        // Use different masks in different iterations
        __mmask64 mask = (i % 3 == 0) ? mask1 : ((i % 3 == 1) ? mask2 : mask3);
        
        // Perform the blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var = blended;
        
        // Extract and accumulate a result
        uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode - 32 x 16-bit integers */
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
        
        // Create mask using different methods
        __mmask32 mask;
        if (i % 4 == 0) {
            // Constant pattern
            mask = 0xAAAAAAAA;
        } else if (i % 4 == 1) {
            // Pattern based on iteration
            mask = (__mmask32)((1U << (i % 32)) - 1);
        } else if (i % 4 == 2) {
            // All ones
            mask = 0xFFFFFFFF;
        } else {
            // Comparison mask
            __m512i threshold = _mm512_set1_epi16(16);
            mask = _mm512_cmpgt_epi16_mask(a, threshold);
        }
        
        // Perform blend
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Store to volatile
        volatile __m512i store_var = blended;
        
        // Extract result
        uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode - 32 x half-precision floats */
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
        
        // Create mask
        __mmask32 mask = (i % 2 == 0) ? 0xAAAAAAAA : 0x55555555;
        
        // Perform blend
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        volatile __m512h store_var = blended;
        
        // Extract and accumulate
        _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)temp[i % 32];
    }
    
    return result;
}

/* V32BFmode - 32 x bfloat16 floats */
volatile int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create bfloat16 vectors
        __m512bh a = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __m512bh b = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask
        __mmask32 mask = (i % 3 == 0) ? 0xFFFFFFFF : 
                        (i % 3 == 1) ? 0xAAAAAAAA : 0x55555555;
        
        // Perform blend
        __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        volatile __m512bh store_var = blended;
        
        // Extract and accumulate
        __m512bh temp_store = blended;
        result += i; // Use iteration count since direct extraction is complex
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode - 16 x 32-bit integers */
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
        
        // Create mask using different strategies
        __mmask16 mask;
        if (i % 5 == 0) {
            mask = 0xAAAA;  // Alternating pattern
        } else if (i % 5 == 1) {
            mask = 0xFFFF;  // All ones
        } else if (i % 5 == 2) {
            mask = 0x5555;  // Opposite alternating
        } else if (i % 5 == 3) {
            // Mask based on comparison
            __m512i cmp_val = _mm512_set1_epi32(8);
            mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
        } else {
            // Dynamic mask based on iteration
            mask = (__mmask16)((1 << (i % 16)) - 1);
        }
        
        // Perform blend
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Store to volatile
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        uint32_t temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode - 8 x 64-bit integers */
volatile int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask
        __mmask8 mask;
        switch (i % 4) {
            case 0: mask = 0xAA; break;  // 10101010
            case 1: mask = 0x55; break;  // 01010101
            case 2: mask = 0xFF; break;  // All ones
            case 3: 
                // Comparison-based mask
                __m512i threshold = _mm512_set1_epi64(4);
                mask = _mm512_cmpgt_epi64_mask(a, threshold);
                break;
        }
        
        // Perform blend
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Store to volatile
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        uint64_t temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)(temp[i % 8] & 0xFFFFFFFF);
    }
    
    return result;
}

/* V8DFmode - 8 x double-precision floats */
volatile int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create double vectors
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask
        __mmask8 mask = (i % 2 == 0) ? 0xAA : 0x55;
        
        // Perform blend
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Store to volatile
        volatile __m512d store_var = blended;
        
        // Extract and accumulate
        double temp[8];
        _mm512_storeu_pd(temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode - 16 x single-precision floats */
volatile int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create float vectors
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask using various methods
        __mmask16 mask;
        if (i % 6 == 0) {
            mask = 0xAAAA;
        } else if (i % 6 == 1) {
            mask = 0x5555;
        } else if (i % 6 == 2) {
            mask = 0xFFFF;
        } else if (i % 6 == 3) {
            mask = 0x0000;
        } else if (i % 6 == 4) {
            // Comparison mask
            __m512 threshold = _mm512_set1_ps(7.5f);
            mask = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
        } else {
            // Pattern based on iteration
            mask = (__mmask16)(1 << (i % 16));
        }
        
        // Perform blend
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Store to volatile
        volatile __m512 store_var = blended;
        
        // Extract and accumulate
        float temp[16];
        _mm512_storeu_ps(temp, blended);
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
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported on this CPU (needed for some tests)\n");
        // Continue anyway for tests that only need AVX512F
    }
    
    // Parse command line argument
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    volatile int total_result = 0;
    const int iterations = 50;  // Enough iterations to trigger expansion
    
    // Run tests based on command line argument
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v64qi(iterations);
            }
            printf("V64QImode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32hi(iterations);
            }
            printf("V32HImode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32hf(iterations);
            }
            printf("V32HFmode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 10; i++) {
                total_result += test_blend_v32bf(iterations);
            }
            printf("V32BFmode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16si(iterations);
        }
        printf("V16SImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8di(iterations);
        }
        printf("V8DImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8df(iterations);
        }
        printf("V8DFmode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16sf(iterations);
        }
        printf("V16SFmode test completed\n");
    }
    
    printf("Total accumulated result: %d\n", total_result);
    
    return 0;
}
