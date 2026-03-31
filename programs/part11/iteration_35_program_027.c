#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
__attribute__((noinline))
int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
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
        
        // Method 1: Constant mask pattern (alternating bits)
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask based on iteration
        __mmask64 mask2 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        // Method 3: Mask from comparison
        __m512i cmp_val = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(a, cmp_val);
        
        // Use different masks in different iterations to ensure all paths are taken
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
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

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Various mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA;  // Alternating pattern
        
        // Mask based on iteration count
        __mmask32 mask2 = (__mmask32)((1ULL << (i % 32)) - 1);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(a, cmp_val);
        
        __mmask32 mask = (i % 4 == 0) ? mask1 : 
                        (i % 4 == 1) ? mask2 : mask3;
        
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        int16_t temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
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
        
        __mmask32 mask = 0x55555555;  // Alternating pattern
        
        // Blend operation for half-precision
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __m512h store_var = blended;
        
        // Extract and accumulate (convert to int for accumulation)
        _Float16 temp[32];
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
        
        __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern (different from v32hf)
        
        // Blend operation for bfloat16
        __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __m512bh store_var = blended;
        
        // Extract and accumulate
        __bf16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)temp[i % 32];
    }
    
    return result;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

#ifdef __AVX512F__

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Multiple mask generation approaches
        __mmask16 mask1 = 0xAAAA;  // Alternating
        
        // Progressive mask based on iteration
        __mmask16 mask2 = (__mmask16)((1 << ((i % 16) + 1)) - 1);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        __mmask16 mask = (i % 5 == 0) ? mask1 : 
                        (i % 5 == 1) ? mask2 : mask3;
        
        // Blend using generic intrinsic (compiler should select appropriate pattern)
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        int32_t temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Various mask patterns
        __mmask8 mask1 = 0xAA;  // Alternating
        
        // Mask with single bit set based on iteration
        __mmask8 mask2 = 1 << (i % 8);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(a, cmp_val);
        
        __mmask8 mask = (i % 6 == 0) ? mask1 : 
                       (i % 6 == 1) ? mask2 : mask3;
        
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        int64_t temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Different mask generation strategies
        __mmask8 mask1 = 0x55;  // Alternating (different pattern from integers)
        
        // Progressive mask
        __mmask8 mask2 = (i < 4) ? 0x0F : 0xF0;
        
        // Comparison mask
        __m512d cmp_val = _mm512_set1_pd(i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(a, cmp_val, _CMP_EQ_OQ);
        
        __mmask8 mask = (i % 7 == 0) ? mask1 : 
                       (i % 7 == 1) ? mask2 : mask3;
        
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        volatile __m512d store_var = blended;
        
        // Extract and accumulate
        double temp[8];
        _mm512_storeu_pd(temp, blended);
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
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Multiple mask patterns
        __mmask16 mask1 = 0x5555;  // Alternating
        
        // Mask with varying number of bits set
        __mmask16 mask2 = (__mmask16)((1 << ((i % 16) + 1)) - 1);
        
        // Comparison mask
        __m512 cmp_val = _mm512_set1_ps(i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_EQ_OQ);
        
        __mmask16 mask = (i % 8 == 0) ? mask1 : 
                        (i % 8 == 1) ? mask2 : mask3;
        
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        volatile __m512 store_var = blended;
        
        // Extract and accumulate
        float temp[16];
        _mm512_storeu_ps(temp, blended);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif  // __AVX512F__

// Fallback implementations for non-AVX512 builds
#ifndef __AVX512F__
int test_blend_v64qi(int iter) { return 0; }
int test_blend_v32hi(int iter) { return 0; }
int test_blend_v32hf(int iter) { return 0; }
int test_blend_v32bf(int iter) { return 0; }
int test_blend_v16si(int iter) { return 0; }
int test_blend_v8di(int iter) { return 0; }
int test_blend_v8df(int iter) { return 0; }
int test_blend_v16sf(int iter) { return 0; }
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
    
    const int iterations = 20;
    volatile int total_sum = 0;
    
    // Determine which test to run based on command line argument
    char *test_to_run = NULL;
    if (argc > 1) {
        test_to_run = argv[1];
    }
    
    // Run selected test or all tests
    if (test_to_run == NULL || strcmp(test_to_run, "v64qi") == 0) {
        printf("Testing V64QImode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v64qi(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v32hi") == 0) {
        printf("Testing V32HImode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v32hi(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v32hf") == 0) {
        printf("Testing V32HFmode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v32hf(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v32bf") == 0) {
        printf("Testing V32BFmode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v32bf(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v16si") == 0) {
        printf("Testing V16SImode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v16si(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v8di") == 0) {
        printf("Testing V8DImode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v8di(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v8df") == 0) {
        printf("Testing V8DFmode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v8df(iterations);
        }
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "v16sf") == 0) {
        printf("Testing V16SFmode blend...\n");
        for (int i = 0; i < iterations; i++) {
            total_sum += test_blend_v16sf(iterations);
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
