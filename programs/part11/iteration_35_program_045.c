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
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAA;
        
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
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var;
        store_var = blended;
        
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
        // Create vectors with alternating patterns
        __m512i a = _mm512_set_epi16(
            0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
            32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62
        );
        
        __m512i b = _mm512_set_epi16(
            1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
            33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63
        );
        
        // Multiple mask generation strategies
        __mmask32 mask1 = 0x55555555;  // Alternating pattern
        
        // Data-dependent mask
        __mmask32 mask2 = (__mmask32)((i * 0x01010101) & 0xFFFFFFFF);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi16(30);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(a, cmp_val);
        
        __mmask32 mask;
        if (i % 4 == 0) mask = mask1;
        else if (i % 4 == 1) mask = mask2;
        else mask = mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Force usage
        volatile __m512i store_var;
        store_var = blended;
        
        // Extract result
        uint16_t temp[32];
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
        
        // Mask generation
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 7);
        
        // Comparison mask (greater than 15.0)
        __m512h cmp_val = _mm512_set1_ph(15.0f);
        __mmask32 mask3 = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
        
        __mmask32 mask = (i % 2 == 0) ? mask1 : mask3;
        
        // Blend operation for half-precision
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Force usage
        volatile __m512h store_var;
        store_var = blended;
        
        // Extract and accumulate
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
        __m512bh a = _mm512_set_epi16(
            0x0000, 0x3C00, 0x4000, 0x4200, 0x4400, 0x4500, 0x4600, 0x4700,
            0x4800, 0x4880, 0x4900, 0x4980, 0x4A00, 0x4A80, 0x4B00, 0x4B80,
            0x4C00, 0x4C80, 0x4D00, 0x4D80, 0x4E00, 0x4E80, 0x4F00, 0x4F80,
            0x5000, 0x5080, 0x5100, 0x5180, 0x5200, 0x5280, 0x5300, 0x5380
        );
        
        __m512bh b = _mm512_set_epi16(
            0x5380, 0x5300, 0x5280, 0x5200, 0x5180, 0x5100, 0x5080, 0x5000,
            0x4F80, 0x4F00, 0x4E80, 0x4E00, 0x4D80, 0x4D00, 0x4C80, 0x4C00,
            0x4B80, 0x4B00, 0x4A80, 0x4A00, 0x4980, 0x4900, 0x4880, 0x4800,
            0x4700, 0x4600, 0x4500, 0x4400, 0x4200, 0x4000, 0x3C00, 0x0000
        );
        
        // Mask generation
        __mmask32 mask1 = 0x55555555;
        __mmask32 mask2 = (__mmask32)((i << 8) | i);
        
        // Use different masks
        __mmask32 mask = (i % 3 == 0) ? mask1 : mask2;
        
        // Blend operation for bfloat16
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Force usage
        volatile __m512bh store_var;
        store_var = blended;
        
        // Extract and accumulate
        uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, _mm512_castps_si512(_mm512_castbh_ps(blended)));
        result += temp[i % 32];
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
        // Create vectors
        __m512i a = _mm512_set_epi32(
            0, 4, 8, 12, 16, 20, 24, 28,
            32, 36, 40, 44, 48, 52, 56, 60
        );
        
        __m512i b = _mm512_set_epi32(
            2, 6, 10, 14, 18, 22, 26, 30,
            34, 38, 42, 46, 50, 54, 58, 62
        );
        
        // Multiple mask strategies
        __mmask16 mask1 = 0xAAAA;  // 0b1010101010101010
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi32(30);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        __mmask16 mask;
        if (i % 5 == 0) mask = mask1;
        else if (i % 5 == 1) mask = mask2;
        else mask = mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Force usage
        volatile __m512i store_var;
        store_var = blended;
        
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
        // Create vectors
        __m512i a = _mm512_set_epi64(0, 8, 16, 24, 32, 40, 48, 56);
        __m512i b = _mm512_set_epi64(4, 12, 20, 28, 36, 44, 52, 60);
        
        // Mask generation
        __mmask8 mask1 = 0xAA;  // 0b10101010
        
        // Data-dependent mask
        __mmask8 mask2 = (__mmask8)((i * 0x55) & 0xFF);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi64(30);
        __mmask8 mask3 = _mm512_cmpgt_epi64_mask(a, cmp_val);
        
        __mmask8 mask = (i % 4 == 0) ? mask1 : 
                       (i % 4 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Force usage
        volatile __m512i store_var;
        store_var = blended;
        
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
        // Create double vectors
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Mask generation
        __mmask8 mask1 = 0x55;  // 0b01010101
        
        // Data-dependent mask
        __mmask8 mask2 = (__mmask8)((i * 0x33) & 0xFF);
        
        // Comparison mask
        __m512d cmp_val = _mm512_set1_pd(3.5);
        __mmask8 mask3 = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        
        __mmask8 mask;
        if (i % 6 == 0) mask = mask1;
        else if (i % 6 == 1) mask = mask2;
        else mask = mask3;
        
        // Blend operation for doubles
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Force usage
        volatile __m512d store_var;
        store_var = blended;
        
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
        // Create float vectors
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Multiple mask strategies
        __mmask16 mask1 = 0xAAAA;  // Alternating
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        // Comparison mask
        __m512 cmp_val = _mm512_set1_ps(7.5f);
        __mmask16 mask3 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        
        __mmask16 mask;
        if (i % 7 == 0) mask = mask1;
        else if (i % 7 == 1) mask = mask2;
        else mask = mask3;
        
        // Blend operation for floats
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Force usage
        volatile __m512 store_var;
        store_var = blended;
        
        // Extract and accumulate
        float temp[16];
        _mm512_storeu_ps(temp, blended);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif  // __AVX512F__

// Dummy implementations for non-AVX512 builds
#ifndef __AVX512F__
__attribute__((noinline)) int test_blend_v16si(int iter) { return 0; }
__attribute__((noinline)) int test_blend_v8di(int iter) { return 0; }
__attribute__((noinline)) int test_blend_v8df(int iter) { return 0; }
__attribute__((noinline)) int test_blend_v16sf(int iter) { return 0; }
#endif

#ifndef __AVX512BW__
__attribute__((noinline)) int test_blend_v64qi(int iter) { return 0; }
__attribute__((noinline)) int test_blend_v32hi(int iter) { return 0; }
__attribute__((noinline)) int test_blend_v32hf(int iter) { return 0; }
__attribute__((noinline)) int test_blend_v32bf(int iter) { return 0; }
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
        printf("AVX-512BW not supported on this CPU (some tests will be limited)\n");
    }
    
    // Parse command line argument
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    // Number of iterations for each test
    const int iterations = 50;
    volatile int total_result = 0;
    
    // Run selected tests
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_result += test_blend_v64qi(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_result += test_blend_v32hi(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_result += test_blend_v32hf(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_result += test_blend_v32bf(iterations);
            }
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 20; i++) {
            total_result += test_blend_v16si(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 20; i++) {
            total_result += test_blend_v8di(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 20; i++) {
            total_result += test_blend_v8df(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_result += test_blend_v16sf(iterations);
        }
    }
    
    // Print final result to prevent optimization
    printf("Total result: %d\n", total_result);
    
    return 0;
}
