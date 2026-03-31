#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
static volatile int test_blend_v64qi(int iterations) {
    volatile int sum = 0;
    
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
        __mmask64 mask2 = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a, _mm512_set1_epi8(1)),
            _mm512_setzero_si512()
        );
        
        // Method 3: Alternating mask based on iteration
        __mmask64 mask3 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        // Use different masks in different iterations
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Perform the blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate a value
        sum += _mm512_extract_epi32(result, 0) & 0xFF;
        sum += _mm512_extract_epi32(result, 4) & 0xFF;
    }
    
    return sum;
}

/* V32HImode - 32 x 16-bit integers */
static volatile int test_blend_v32hi(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors with incrementing values
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask using comparison
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        // Alternate mask pattern
        if (i % 2 == 0) {
            mask = 0xAAAAAAAA;  // Alternating pattern
        }
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Store to volatile
        volatile __m512i volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate
        sum += _mm512_extract_epi16(result, 0);
        sum += _mm512_extract_epi16(result, 16);
    }
    
    return sum;
}

/* V32HFmode - 32 x half-precision floats */
static volatile int test_blend_v32hf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create half-precision vectors (using _Float16)
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
        
        // Create mask - alternate patterns
        __mmask32 mask;
        if (i % 3 == 0) {
            mask = 0x55555555;  // Every other element
        } else if (i % 3 == 1) {
            mask = 0xAAAAAAAA;  // Opposite pattern
        } else {
            // Compare-based mask
            mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
        }
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to volatile
        volatile __m512h volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate (convert to int for summing)
        sum += (int)_mm512_extract_ph(result, 0);
        sum += (int)_mm512_extract_ph(result, 16);
    }
    
    return sum;
}

/* V32BFmode - 32 x bfloat16 floats */
static volatile int test_blend_v32bf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create bfloat16 vectors
        __m512bh a = _mm512_set1_epi16(0x3F80);  // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000);  // 2.0 in bfloat16
        
        // Vary the vectors slightly
        if (i % 2 == 0) {
            a = _mm512_add_epi16(a, _mm512_set1_epi16(i & 0xFF));
        }
        
        // Create different mask patterns
        __mmask32 mask;
        if (i % 4 == 0) {
            mask = 0xFFFFFFFF;  // All from b
        } else if (i % 4 == 1) {
            mask = 0x00000000;  // All from a
        } else if (i % 4 == 2) {
            mask = 0x55555555;  // Checkerboard
        } else {
            mask = 0xAAAAAAAA;  // Opposite checkerboard
        }
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Store to volatile
        volatile __m512bh volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate
        sum += _mm512_extract_epi16(result, 0);
        sum += _mm512_extract_epi16(result, 16);
    }
    
    return sum;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

#ifdef __AVX512F__

/* V16SImode - 16 x 32-bit integers */
static volatile int test_blend_v16si(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask using comparison
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(7));
        
        // Alternate with constant mask
        if (i % 2 == 0) {
            mask = 0xAAAA;  // 0b1010101010101010
        }
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Store to volatile
        volatile __m512i volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate
        sum += _mm512_extract_epi32(result, 0);
        sum += _mm512_extract_epi32(result, 8);
    }
    
    return sum;
}

/* V8DImode - 8 x 64-bit integers */
static volatile int test_blend_v8di(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask - alternate patterns
        __mmask8 mask;
        if (i % 3 == 0) {
            mask = 0xAA;  // 0b10101010
        } else if (i % 3 == 1) {
            mask = 0x55;  // 0b01010101
        } else {
            // Compare-based mask
            mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(3));
        }
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Store to volatile
        volatile __m512i volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate (using 32-bit parts)
        sum += (int)_mm512_extract_epi64(result, 0);
        sum += (int)_mm512_extract_epi64(result, 4);
    }
    
    return sum;
}

/* V8DFmode - 8 x double-precision floats */
static volatile int test_blend_v8df(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask
        __mmask8 mask;
        if (i % 2 == 0) {
            mask = 0xF0;  // First 4 from a, last 4 from b
        } else {
            mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
        }
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Store to volatile
        volatile __m512d volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate (convert to int)
        sum += (int)_mm512_extract_pd(result, 0);
        sum += (int)_mm512_extract_pd(result, 4);
    }
    
    return sum;
}

/* V16SFmode - 16 x single-precision floats */
static volatile int test_blend_v16sf(int iterations) {
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Create vectors
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask
        __mmask16 mask;
        if (i % 3 == 0) {
            mask = 0xAAAA;  // Alternating
        } else if (i % 3 == 1) {
            mask = 0x5555;  // Opposite alternating
        } else {
            mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
        }
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Store to volatile
        volatile __m512 volatile_result = result;
        (void)volatile_result;
        
        // Extract and accumulate
        sum += (int)_mm512_extract_ps(result, 0);
        sum += (int)_mm512_extract_ps(result, 8);
    }
    
    return sum;
}

#endif  // __AVX512F__

// Dummy implementations for when AVX-512 is not available
#ifndef __AVX512F__
static volatile int test_blend_v64qi(int iterations) { return 0; }
static volatile int test_blend_v32hi(int iterations) { return 0; }
static volatile int test_blend_v32hf(int iterations) { return 0; }
static volatile int test_blend_v32bf(int iterations) { return 0; }
static volatile int test_blend_v16si(int iterations) { return 0; }
static volatile int test_blend_v8di(int iterations) { return 0; }
static volatile int test_blend_v8df(int iterations) { return 0; }
static volatile int test_blend_v16sf(int iterations) { return 0; }
#endif

int main(int argc, char *argv[]) {
    const int ITERATIONS = 50;
    volatile int total_sum = 0;
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    int has_avx512bf16 = __builtin_cpu_supports("avx512bf16");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    printf("AVX-512F supported: %s\n", has_avx512f ? "yes" : "no");
    printf("AVX-512BW supported: %s\n", has_avx512bw ? "yes" : "no");
    printf("AVX-512BF16 supported: %s\n", has_avx512bf16 ? "yes" : "no");
    
    // Determine which test to run
    char *test_to_run = NULL;
    if (argc > 1) {
        test_to_run = argv[1];
    }
    
    // Run selected tests
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v64qi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v64qi(ITERATIONS);
            printf("Ran V64QImode test\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v32hi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hi(ITERATIONS);
            printf("Ran V32HImode test\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v32hf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hf(ITERATIONS);
            printf("Ran V32HFmode test\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v32bf") == 0) {
        if (has_avx512bf16) {
            total_sum += test_blend_v32bf(ITERATIONS);
            printf("Ran V32BFmode test\n");
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v16si") == 0) {
        total_sum += test_blend_v16si(ITERATIONS);
        printf("Ran V16SImode test\n");
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v8di") == 0) {
        total_sum += test_blend_v8di(ITERATIONS);
        printf("Ran V8DImode test\n");
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v8df") == 0) {
        total_sum += test_blend_v8df(ITERATIONS);
        printf("Ran V8DFmode test\n");
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || strcmp(test_to_run, "v16sf") == 0) {
        total_sum += test_blend_v16sf(ITERATIONS);
        printf("Ran V16SFmode test\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
