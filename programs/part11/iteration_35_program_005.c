#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
volatile int test_blend_v64qi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Method 1: Constant mask
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = (__mmask64)(i * 0x5555555555555555ULL);
        
        // Method 3: Comparison mask
        __m512i vec_a = _mm512_set_epi8(
            64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,
            48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,
            32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
            16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
        );
        __m512i vec_b = _mm512_set_epi8(
            128,127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,
            112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,
            96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,
            80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65
        );
        
        // Generate mask by comparing with threshold
        __m512i threshold = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(vec_a, threshold);
        
        // Use different masks based on iteration
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Perform blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, vec_a, vec_b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var;
        store_var = blended;
        
        // Extract and accumulate result
        int sum = 0;
        for (int j = 0; j < 64; j++) {
            sum += ((char*)&store_var)[j];
        }
        result += sum;
    }
    
    return result;
}

/* V32HImode - 32 x 16-bit integers */
volatile int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Multiple mask generation methods
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512i vec_a = _mm512_set_epi16(
            32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,
            16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
        );
        __m512i vec_b = _mm512_set_epi16(
            64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,
            48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33
        );
        
        // Comparison mask
        __m512i threshold = _mm512_set1_epi16(16);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(vec_a, threshold);
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
        
        volatile __m512i store_var;
        store_var = blended;
        
        // Extract and accumulate
        int sum = 0;
        for (int j = 0; j < 32; j++) {
            sum += ((short*)&store_var)[j];
        }
        result += sum;
    }
    
    return result;
}

/* V32HFmode - 32 x half-precision floats */
volatile int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Initialize with half-precision values
        __m512h vec_a = _mm512_set_ph(
            1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
            17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
            25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
        );
        __m512h vec_b = _mm512_set_ph(
            33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f,
            41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f, 48.0f,
            49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f, 56.0f,
            57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f, 64.0f
        );
        
        // Comparison mask (requires AVX-512FP16)
        #ifdef __AVX512FP16__
        __m512h threshold = _mm512_set1_ph(16.0f);
        __mmask32 mask3 = _mm512_cmp_ph_mask(vec_a, threshold, _CMP_GT_OQ);
        #else
        __mmask32 mask3 = 0x55555555;
        #endif
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512h blended = _mm512_mask_blend_ph(mask, vec_a, vec_b);
        
        volatile __m512h store_var;
        store_var = blended;
        
        // Extract and accumulate
        for (int j = 0; j < 32; j++) {
            result += (int)((short*)&store_var)[j];
        }
    }
    
    return result;
}

/* V32BFmode - 32 x bfloat16 floats */
volatile int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Initialize bfloat16 values
        __m512bh vec_a = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh vec_b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Simple alternating pattern for mask3
        __mmask32 mask3 = 0x55555555;
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation for bfloat16
        __m512bh blended = _mm512_mask_blend_epi16(mask, vec_a, vec_b);
        
        volatile __m512bh store_var;
        store_var = blended;
        
        // Extract and accumulate
        for (int j = 0; j < 32; j++) {
            result += ((unsigned short*)&store_var)[j];
        }
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
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512i vec_a = _mm512_set_epi32(
            16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
        );
        __m512i vec_b = _mm512_set_epi32(
            32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17
        );
        
        // Comparison mask
        __m512i threshold = _mm512_set1_epi32(8);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(vec_a, threshold);
        
        __mmask16 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi32(mask, vec_a, vec_b);
        
        volatile __m512i store_var;
        store_var = blended;
        
        // Extract and accumulate using reduce add
        result += _mm512_reduce_add_epi32(blended);
    }
    
    return result;
}

/* V8DImode - 8 x 64-bit integers */
volatile int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512i vec_a = _mm512_set_epi64(8,7,6,5,4,3,2,1);
        __m512i vec_b = _mm512_set_epi64(16,15,14,13,12,11,10,9);
        
        // Comparison mask
        __m512i threshold = _mm512_set1_epi64(4);
        __mmask8 mask3 = _mm512_cmpgt_epi64_mask(vec_a, threshold);
        
        __mmask8 mask = (i % 3 == 0) ? mask1 : 
                       (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi64(mask, vec_a, vec_b);
        
        volatile __m512i store_var;
        store_var = blended;
        
        // Extract and accumulate
        long long sum = 0;
        for (int j = 0; j < 8; j++) {
            sum += ((long long*)&store_var)[j];
        }
        result += (int)sum;
    }
    
    return result;
}

/* V8DFmode - 8 x double-precision floats */
volatile int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512d vec_a = _mm512_set_pd(8.0,7.0,6.0,5.0,4.0,3.0,2.0,1.0);
        __m512d vec_b = _mm512_set_pd(16.0,15.0,14.0,13.0,12.0,11.0,10.0,9.0);
        
        // Comparison mask
        __m512d threshold = _mm512_set1_pd(4.0);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, threshold, _CMP_GT_OQ);
        
        __mmask8 mask = (i % 3 == 0) ? mask1 : 
                       (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512d blended = _mm512_mask_blend_pd(mask, vec_a, vec_b);
        
        volatile __m512d store_var;
        store_var = blended;
        
        // Extract and accumulate
        double sum = 0.0;
        for (int j = 0; j < 8; j++) {
            sum += ((double*)&store_var)[j];
        }
        result += (int)sum;
    }
    
    return result;
}

/* V16SFmode - 16 x single-precision floats */
volatile int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512 vec_a = _mm512_set_ps(
            16.0f,15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,
            8.0f,7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f
        );
        __m512 vec_b = _mm512_set_ps(
            32.0f,31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,
            24.0f,23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f
        );
        
        // Comparison mask
        __m512 threshold = _mm512_set1_ps(8.0f);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, threshold, _CMP_GT_OQ);
        
        __mmask16 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        // Blend operation
        __m512 blended = _mm512_mask_blend_ps(mask, vec_a, vec_b);
        
        volatile __m512 store_var;
        store_var = blended;
        
        // Extract and accumulate using reduce add
        result += (int)_mm512_reduce_add_ps(blended);
    }
    
    return result;
}

#endif // __AVX512F__

/* Dummy implementations for non-AVX512 builds */
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
    const int ITERATIONS = 20;
    volatile int total_sum = 0;
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    printf("AVX-512F supported: %s\n", has_avx512f ? "YES" : "NO");
    printf("AVX-512BW supported: %s\n", has_avx512bw ? "YES" : "NO");
    
    // Determine which test to run
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    // Run selected tests
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v64qi(ITERATIONS);
            printf("V64QImode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hi(ITERATIONS);
            printf("V32HImode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hf(ITERATIONS);
            printf("V32HFmode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32bf(ITERATIONS);
            printf("V32BFmode test completed\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        total_sum += test_blend_v16si(ITERATIONS);
        printf("V16SImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        total_sum += test_blend_v8di(ITERATIONS);
        printf("V8DImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        total_sum += test_blend_v8df(ITERATIONS);
        printf("V8DFmode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        total_sum += test_blend_v16sf(ITERATIONS);
        printf("V16SFmode test completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
