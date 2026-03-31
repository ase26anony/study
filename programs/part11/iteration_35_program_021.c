#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
volatile int test_blend_v64qi(int iter) {
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
        __mmask64 mask2 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        // Method 3: Mask from comparison
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Combine masks for complexity
        __mmask64 final_mask = mask1 & mask2 | mask3;
        
        // Perform the blend operation
        __m512i blended = _mm512_mask_blend_epi8(final_mask, a, b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var = blended;
        
        // Extract and accumulate a result
        int sum = 0;
        unsigned char temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        for (int j = 0; j < 64; j += 8) {
            sum += temp[j];
        }
        result += sum;
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
volatile int test_blend_v32hi(int iter) {
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
        
        // Different mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (i < iter/2) ? 0xFFFFFFFF : 0x55555555;
        
        __m512i cmp_val = _mm512_set1_epi16(16);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(a, cmp_val);
        
        __mmask32 final_mask = (mask1 ^ mask2) | mask3;
        
        __m512i blended = _mm512_mask_blend_epi16(final_mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Reduce and accumulate
        short temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        for (int j = 0; j < 32; j += 4) {
            result += temp[j];
        }
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
volatile int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Initialize with simple pattern
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
        
        __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
        
        // For half-precision, we might need to use _mm512_mask_blend_ph
        // Note: Some compilers may require casting or different intrinsic
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __m512h store_var = blended;
        
        // Extract some values
        _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        for (int j = 0; j < 32; j += 8) {
            result += (int)temp[j];
        }
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
volatile int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // BFloat16 requires special handling - use integer representation
        unsigned short bf_data_a[32];
        unsigned short bf_data_b[32];
        
        for (int j = 0; j < 32; j++) {
            // Simple pattern: j as bfloat16
            bf_data_a[j] = j << 8;  // Rough bfloat16 representation
            bf_data_b[j] = (31 - j) << 8;
        }
        
        __m512i a = _mm512_loadu_si512((void*)bf_data_a);
        __m512i b = _mm512_loadu_si512((void*)bf_data_b);
        
        __mmask32 mask = (i % 3 == 0) ? 0xFFFFFFFF : 0x55555555;
        
        // Blend as 16-bit integers (same size as bfloat16)
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract
        unsigned short temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        for (int j = 0; j < 32; j += 4) {
            result += temp[j] >> 8;  // Get approximate value
        }
    }
    
    return result;
}

#endif  // __AVX512BW__
#endif  // __AVX512F__

#ifdef __AVX512F__

/* V16SImode: 16 x 32-bit integers */
volatile int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Multiple mask generation methods
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (i & 1) ? 0xFFFF : 0x5555;
        
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        __mmask16 final_mask = mask1 ^ mask2 | mask3;
        
        __m512i blended = _mm512_mask_blend_epi32(final_mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Use reduction intrinsic if available, otherwise manual
        result += _mm512_reduce_add_epi32(blended);
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
volatile int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        __mmask8 mask = 0xAA;  // 10101010
        
        // Vary mask based on iteration
        if (i % 4 == 0) mask = 0xFF;
        else if (i % 4 == 1) mask = 0x55;
        else if (i % 4 == 2) mask = 0xCC;
        
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and sum
        long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        for (int j = 0; j < 8; j++) {
            result += (int)(temp[j] & 0xFFFFFFFF);
        }
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
volatile int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        __mmask8 mask = 0xAA;
        
        // Create data-dependent mask
        __m512d cmp_val = _mm512_set1_pd(3.5);
        __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        __mmask8 final_mask = mask ^ cmp_mask;
        
        __m512d blended = _mm512_mask_blend_pd(final_mask, a, b);
        
        volatile __m512d store_var = blended;
        
        // Horizontal add
        __m256d low = _mm512_castpd512_pd256(blended);
        __m256d high = _mm512_extractf64x4_pd(blended, 1);
        __m256d sum256 = _mm256_add_pd(low, high);
        __m128d sum128 = _mm_add_pd(_mm256_castpd256_pd128(sum256), 
                                   _mm256_extractf128_pd(sum256, 1));
        double sum = _mm_cvtsd_f64(_mm_add_pd(sum128, 
                                             _mm_unpackhi_pd(sum128, sum128)));
        
        result += (int)sum;
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
volatile int test_blend_v16sf(int iter) {
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
        
        __mmask16 mask = 0xAAAA;
        
        // Complex mask generation
        __m512 cmp_val = _mm512_set1_ps(7.5f);
        __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        
        __mmask16 dynamic_mask = (i % 5 == 0) ? 0xFFFF : 0x5555;
        __mmask16 final_mask = (mask & dynamic_mask) | cmp_mask;
        
        __m512 blended = _mm512_mask_blend_ps(final_mask, a, b);
        
        volatile __m512 store_var = blended;
        
        // Reduce add
        __m256 low = _mm512_castps512_ps256(blended);
        __m256 high = _mm512_extractf32x8_ps(blended, 1);
        __m256 sum8 = _mm256_add_ps(low, high);
        
        __m128 sum4 = _mm_add_ps(_mm256_castps256_ps128(sum8),
                                _mm256_extractf128_ps(sum8, 1));
        
        sum4 = _mm_hadd_ps(sum4, sum4);
        sum4 = _mm_hadd_ps(sum4, sum4);
        
        result += (int)_mm_cvtss_f32(sum4);
    }
    
    return result;
}

#endif  // __AVX512F__

// Fallback implementations for non-AVX512 builds
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
        printf("AVX-512BW not supported (needed for some tests)\n");
        // Continue anyway for F-only tests
    }
    
    const int iterations = 20;
    volatile int total_result = 0;
    
    // Determine which test to run based on command line
    char *test_to_run = NULL;
    if (argc > 1) {
        test_to_run = argv[1];
    }
    
    // Run selected test or all tests
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v64qi") == 0) {
        total_result += test_blend_v64qi(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v32hi") == 0) {
        total_result += test_blend_v32hi(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v32hf") == 0) {
        total_result += test_blend_v32hf(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v32bf") == 0) {
        total_result += test_blend_v32bf(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v16si") == 0) {
        total_result += test_blend_v16si(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v8di") == 0) {
        total_result += test_blend_v8di(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v8df") == 0) {
        total_result += test_blend_v8df(iterations);
    }
    
    if (test_to_run == NULL || strcmp(test_to_run, "all") == 0 || 
        strcmp(test_to_run, "v16sf") == 0) {
        total_result += test_blend_v16sf(iterations);
    }
    
    printf("Total result: %d\n", total_result);
    
    return 0;
}
