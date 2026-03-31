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
        
        // Method 2: Data-dependent mask based on iteration
        __mmask64 mask2 = (i % 2 == 0) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 3: Mask from comparison
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Combine masks for complex pattern
        __mmask64 final_mask = mask1 & mask2 | mask3;
        
        // Perform the blend operation
        __m512i blended = _mm512_mask_blend_epi8(final_mask, a, b);
        
        // Extract and accumulate result
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
        // Create vectors with arithmetic progression
        __m512i a = _mm512_set_epi16(
            0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
            32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62
        );
        
        __m512i b = _mm512_set_epi16(
            62, 60, 58, 56, 54, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32,
            30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0
        );
        
        // Pattern-based mask
        __mmask32 mask1 = 0x55555555;
        
        // Data-dependent mask
        __mmask32 mask2 = (__mmask32)((i * 0x01010101) & 0xFFFFFFFF);
        
        // Comparison mask
        __m512i threshold = _mm512_set1_epi16(30);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(a, threshold);
        
        __mmask32 final_mask = mask1 ^ mask2 | mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi16(final_mask, a, b);
        
        // Extract and accumulate
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
        
        // Create mask
        __mmask32 mask = (i % 2 == 0) ? 0xAAAAAAAA : 0x55555555;
        
        // Blend operation for half-precision
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Store and accumulate
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)(temp[i % 32] * 100);
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
            0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f,
            4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f,
            8.0f, 8.5f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 11.5f,
            12.0f, 12.5f, 13.0f, 13.5f, 14.0f, 14.5f, 15.0f, 15.5f
        );
        
        __m512bh b = _mm512_set_ph(
            15.5f, 15.0f, 14.5f, 14.0f, 13.5f, 13.0f, 12.5f, 12.0f,
            11.5f, 11.0f, 10.5f, 10.0f, 9.5f, 9.0f, 8.5f, 8.0f,
            7.5f, 7.0f, 6.5f, 6.0f, 5.5f, 5.0f, 4.5f, 4.0f,
            3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f, 0.0f
        );
        
        // Create alternating mask
        __mmask32 mask = 0xAAAAAAAA;
        
        // Blend operation for bfloat16
        __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Store and accumulate
        volatile __bf16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)(((float)temp[i % 32]) * 100);
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask with pattern
        __mmask16 mask1 = 0xAAAA;
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi32(7);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        __mmask16 final_mask = mask1 ^ mask2 | mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi32(final_mask, a, b);
        
        // Extract and accumulate
        volatile int temp[16];
        _mm512_storeu_epi32((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile long long result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask
        __mmask8 mask = (i % 2 == 0) ? 0xAA : 0x55;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_epi64((void*)temp, blended);
        result += temp[i % 8];
    }
    
    return (int)result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create double vectors
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask with pattern
        __mmask8 mask1 = 0xAA;
        
        // Data-dependent mask
        __mmask8 mask2 = (__mmask8)((i * 0x55) & 0xFF);
        
        // Comparison mask
        __m512d threshold = _mm512_set1_pd(3.5);
        __mmask8 mask3 = _mm512_cmp_pd_mask(a, threshold, _CMP_GT_OQ);
        
        __mmask8 final_mask = mask1 ^ mask2 | mask3;
        
        // Blend operation
        __m512d blended = _mm512_mask_blend_pd(final_mask, a, b);
        
        // Store and accumulate
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended);
        result += (int)(temp[i % 8] * 100);
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create float vectors
        __m512 a = _mm512_set_ps(
            0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f,
            4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f
        );
        
        __m512 b = _mm512_set_ps(
            7.5f, 7.0f, 6.5f, 6.0f, 5.5f, 5.0f, 4.5f, 4.0f,
            3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f, 0.0f
        );
        
        // Create mask
        __mmask16 mask1 = 0xAAAA;
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        // Comparison mask
        __m512 threshold = _mm512_set1_ps(3.5f);
        __mmask16 mask3 = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
        
        __mmask16 final_mask = mask1 ^ mask2 | mask3;
        
        // Blend operation
        __m512 blended = _mm512_mask_blend_ps(final_mask, a, b);
        
        // Store and accumulate
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended);
        result += (int)(temp[i % 16] * 100);
    }
    
    return result;
}

#endif // __AVX512F__

// Fallback implementations for non-AVX512 systems
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
    const int DEFAULT_ITERATIONS = 50;
    const char *test_mode = (argc > 1) ? argv[1] : "all";
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (strstr(test_mode, "v64qi") || strstr(test_mode, "v32hi") || 
        strstr(test_mode, "v32hf") || strstr(test_mode, "v32bf")) {
        if (!has_avx512bw) {
            printf("AVX-512BW required for this test\n");
            return 0;
        }
    }
    
    volatile int total_result = 0;
    
    // Run tests based on mode selection
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v64qi(DEFAULT_ITERATIONS);
        }
        printf("V64QImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v32hi(DEFAULT_ITERATIONS);
        }
        printf("V32HImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v32hf(DEFAULT_ITERATIONS);
        }
        printf("V32HFmode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v32bf(DEFAULT_ITERATIONS);
        }
        printf("V32BFmode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16si(DEFAULT_ITERATIONS);
        }
        printf("V16SImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8di(DEFAULT_ITERATIONS);
        }
        printf("V8DImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v8df(DEFAULT_ITERATIONS);
        }
        printf("V8DFmode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 10; i++) {
            total_result += test_blend_v16sf(DEFAULT_ITERATIONS);
        }
        printf("V16SFmode test completed\n");
    }
    
    printf("Final accumulated result: %d\n", total_result);
    
    return 0;
}
