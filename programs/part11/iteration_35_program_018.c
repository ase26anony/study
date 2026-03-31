#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode: 64 x 8-bit integers */
static int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors with different patterns
        __m512i a = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        
        __m512i b = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Method 1: Constant mask
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAull;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = (__mmask64)(i & 0xFF);
        
        // Method 3: Comparison-based mask
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Use different masks based on iteration
        __mmask64 mask;
        if (i % 3 == 0) mask = mask1;
        else if (i % 3 == 1) mask = mask2;
        else mask = mask3;
        
        // Perform blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var = blended;
        
        // Extract and accumulate a result
        int elem = _mm512_extract_epi8(blended, i % 64);
        result += elem;
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
static int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors with incrementing values
        __m512i a = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        
        __m512i b = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Generate mask in different ways
        __mmask32 mask;
        if (i % 2 == 0) {
            // Constant pattern mask
            mask = 0x55555555;
        } else {
            // Data-dependent mask
            mask = (__mmask32)((i * 0x01010101) & 0xFFFFFFFF);
        }
        
        // Perform blend
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Force usage
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        short elem = _mm512_extract_epi16(blended, i % 32);
        result += elem;
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
static int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create half-precision float vectors
        __m512h a = _mm512_set_ph(
            0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f,
            4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f, 8.0f,
            8.5f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 11.5f, 12.0f,
            12.5f, 13.0f, 13.5f, 14.0f, 14.5f, 15.0f, 15.5f, 16.0f
        );
        
        __m512h b = _mm512_set_ph(
            16.0f, 15.5f, 15.0f, 14.5f, 14.0f, 13.5f, 13.0f, 12.5f,
            12.0f, 11.5f, 11.0f, 10.5f, 10.0f, 9.5f, 9.0f, 8.5f,
            8.0f, 7.5f, 7.0f, 6.5f, 6.0f, 5.5f, 5.0f, 4.5f,
            4.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 1.0f, 0.5f
        );
        
        // Create mask using comparison
        __m512h cmp_val = _mm512_set1_ph(8.0f);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
        
        // Perform blend
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Force usage
        volatile __m512h store_var = blended;
        
        // Extract and accumulate (convert to int for accumulation)
        _Float16 elem;
        _mm512_store_ph((_Float16*)&elem, blended);
        result += (int)(elem * 10);
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
static int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create bfloat16 vectors
        __m512bh a = _mm512_set_epi16(
            0x3F00, 0x3F80, 0x4000, 0x4040, 0x4080, 0x40A0, 0x40C0, 0x40E0,
            0x4100, 0x4110, 0x4120, 0x4130, 0x4140, 0x4150, 0x4160, 0x4170,
            0x4180, 0x4188, 0x4190, 0x4198, 0x41A0, 0x41A8, 0x41B0, 0x41B8,
            0x41C0, 0x41C8, 0x41D0, 0x41D8, 0x41E0, 0x41E8, 0x41F0, 0x41F8
        );
        
        __m512bh b = _mm512_set_epi16(
            0x41F8, 0x41F0, 0x41E8, 0x41E0, 0x41D8, 0x41D0, 0x41C8, 0x41C0,
            0x41B8, 0x41B0, 0x41A8, 0x41A0, 0x4198, 0x4190, 0x4188, 0x4180,
            0x4170, 0x4160, 0x4150, 0x4140, 0x4130, 0x4120, 0x4110, 0x4100,
            0x40E0, 0x40C0, 0x40A0, 0x4080, 0x4040, 0x4000, 0x3F80, 0x3F00
        );
        
        // Create alternating mask pattern
        __mmask32 mask = (i % 2 == 0) ? 0xAAAAAAAA : 0x55555555;
        
        // Perform blend
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Force usage
        volatile __m512bh store_var = blended;
        
        // Extract and accumulate
        unsigned short elem;
        _mm512_store_epi16(&elem, blended);
        result += elem;
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode: 16 x 32-bit integers */
static int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        
        __m512i b = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Generate mask using comparison
        __m512i cmp_val = _mm512_set1_epi32(7);
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        // Alternate with constant mask
        if (i % 3 == 0) {
            mask = 0xAAAA;
        } else if (i % 3 == 1) {
            mask = 0x5555;
        }
        
        // Perform blend
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Force usage
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        int elem = _mm512_extract_epi32(blended, i % 16);
        result += elem;
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
static int test_blend_v8di(int iter) {
    volatile long long result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        
        // Create mask
        __mmask8 mask;
        if (i % 2 == 0) {
            mask = 0xAA;  // 10101010
        } else {
            mask = 0x55;  // 01010101
        }
        
        // Perform blend
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Force usage
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        long long elem = _mm512_extract_epi64(blended, i % 8);
        result += elem;
    }
    
    return (int)result;
}

/* V8DFmode: 8 x double-precision floats */
static int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask using comparison
        __m512d cmp_val = _mm512_set1_pd(3.5);
        __mmask8 mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        
        // Alternate pattern
        if (i % 4 == 0) mask = 0xFF;
        else if (i % 4 == 1) mask = 0x00;
        else if (i % 4 == 2) mask = 0xAA;
        
        // Perform blend
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Force usage
        volatile __m512d store_var = blended;
        
        // Extract and accumulate
        double elem;
        _mm512_store_pd(&elem, blended);
        result += (int)(elem * 10);
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
static int test_blend_v16sf(int iter) {
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
        
        // Create mask in different ways
        __mmask16 mask;
        if (i % 3 == 0) {
            // Constant mask
            mask = 0xAAAA;
        } else if (i % 3 == 1) {
            // Comparison-based mask
            __m512 cmp_val = _mm512_set1_ps(7.5f);
            mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        } else {
            // Data-dependent mask
            mask = (__mmask16)((i * 0x1111) & 0xFFFF);
        }
        
        // Perform blend
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Force usage
        volatile __m512 store_var = blended;
        
        // Extract and accumulate
        float elem;
        _mm512_store_ps(&elem, blended);
        result += (int)(elem * 10);
    }
    
    return result;
}

#endif // __AVX512F__

// Fallback implementations for non-AVX512 builds
#ifndef __AVX512F__
static int test_blend_v64qi(int iter) { return 0; }
static int test_blend_v32hi(int iter) { return 0; }
static int test_blend_v32hf(int iter) { return 0; }
static int test_blend_v32bf(int iter) { return 0; }
static int test_blend_v16si(int iter) { return 0; }
static int test_blend_v8di(int iter) { return 0; }
static int test_blend_v8df(int iter) { return 0; }
static int test_blend_v16sf(int iter) { return 0; }
#endif

int main(int argc, char *argv[]) {
    const int DEFAULT_ITERATIONS = 20;
    int iterations = DEFAULT_ITERATIONS;
    volatile int total_result = 0;
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    // Parse command line arguments
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = DEFAULT_ITERATIONS;
    }
    
    printf("Running AVX-512 blend tests with %d iterations\n", iterations);
    printf("AVX-512F: %s, AVX-512BW: %s\n", 
           has_avx512f ? "yes" : "no",
           has_avx512bw ? "yes" : "no");
    
    // Run selected tests
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            int res = test_blend_v64qi(iterations);
            total_result += res;
            printf("V64QImode test result: %d\n", res);
        } else {
            printf("Skipping V64QImode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            int res = test_blend_v32hi(iterations);
            total_result += res;
            printf("V32HImode test result: %d\n", res);
        } else {
            printf("Skipping V32HImode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            int res = test_blend_v32hf(iterations);
            total_result += res;
            printf("V32HFmode test result: %d\n", res);
        } else {
            printf("Skipping V32HFmode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            int res = test_blend_v32bf(iterations);
            total_result += res;
            printf("V32BFmode test result: %d\n", res);
        } else {
            printf("Skipping V32BFmode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        int res = test_blend_v16si(iterations);
        total_result += res;
        printf("V16SImode test result: %d\n", res);
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        int res = test_blend_v8di(iterations);
        total_result += res;
        printf("V8DImode test result: %d\n", res);
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        int res = test_blend_v8df(iterations);
        total_result += res;
        printf("V8DFmode test result: %d\n", res);
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        int res = test_blend_v16sf(iterations);
        total_result += res;
        printf("V16SFmode test result: %d\n", res);
    }
    
    printf("Total accumulated result: %d\n", total_result);
    
    return 0;
}
