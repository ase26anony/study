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
        // Method 1: Constant mask
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = (__mmask64)(i * 0x5555555555555555ULL);
        
        // Method 3: Comparison mask
        __m512i vec_a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, vec_b);
        
        // Blend with different masks
        __m512i blended1 = _mm512_mask_blend_epi8(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi8(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi8(mask3, vec_a, vec_b);
        
        // Extract and accumulate results
        result += _mm512_extract_epi32(blended1, 0);
        result += _mm512_extract_epi32(blended2, 4);
        result += _mm512_extract_epi32(blended3, 8);
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Different mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, vec_b);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi16(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi16(mask3, vec_a, vec_b);
        
        // Accumulate results
        result += _mm512_extract_epi32(blended1, 0);
        result += _mm512_extract_epi32(blended2, 4);
        result += _mm512_extract_epi32(blended3, 8);
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Create half-precision vectors
        __m512h vec_a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512h vec_b = _mm512_set1_ph((float)i);
        
        // Comparison for mask generation
        __mmask32 mask3 = _mm512_cmp_ph_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        // Blend operations
        __m512h blended1 = _mm512_mask_blend_ph(mask1, vec_a, vec_b);
        __m512h blended2 = _mm512_mask_blend_ph(mask2, vec_a, vec_b);
        __m512h blended3 = _mm512_mask_blend_ph(mask3, vec_a, vec_b);
        
        // Store to volatile memory and accumulate
        volatile __m512h store_blend = blended1;
        result += i + (int)((float)store_blend[0]);
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Create bfloat16 vectors
        __m512bh vec_a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512bh vec_b = _mm512_set1_ph((float)i);
        
        // Blend operations
        __m512bh blended1 = _mm512_mask_blend_ph(mask1, vec_a, vec_b);
        __m512bh blended2 = _mm512_mask_blend_ph(mask2, vec_a, vec_b);
        
        // Store to volatile array
        volatile float store_array[32];
        _mm512_store_ph((void*)store_array, blended1);
        result += (int)store_array[0] + i;
    }
    
    return result;
}

#endif /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, vec_b);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi32(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi32(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi32(mask3, vec_a, vec_b);
        
        // Extract and accumulate
        result += _mm512_extract_epi32(blended1, 0);
        result += _mm512_extract_epi32(blended2, 4);
        result += _mm512_extract_epi32(blended3, 8);
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec_a, vec_b);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi64(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi64(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi64(mask3, vec_a, vec_b);
        
        // Extract and accumulate
        result += (int)_mm512_extract_epi64(blended1, 0);
        result += (int)_mm512_extract_epi64(blended2, 2);
        result += (int)_mm512_extract_epi64(blended3, 4);
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512d vec_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec_b = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        // Blend operations
        __m512d blended1 = _mm512_mask_blend_pd(mask1, vec_a, vec_b);
        __m512d blended2 = _mm512_mask_blend_pd(mask2, vec_a, vec_b);
        __m512d blended3 = _mm512_mask_blend_pd(mask3, vec_a, vec_b);
        
        // Store to volatile array
        volatile double store_array[8];
        _mm512_store_pd((void*)store_array, blended1);
        result += (int)store_array[0] + i;
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512 vec_b = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        // Blend operations
        __m512 blended1 = _mm512_mask_blend_ps(mask1, vec_a, vec_b);
        __m512 blended2 = _mm512_mask_blend_ps(mask2, vec_a, vec_b);
        __m512 blended3 = _mm512_mask_blend_ps(mask3, vec_a, vec_b);
        
        // Store to volatile array and accumulate
        volatile float store_array[16];
        _mm512_store_ps((void*)store_array, blended1);
        result += (int)store_array[0] + i;
    }
    
    return result;
}

#else /* __AVX512F__ not defined */

/* Dummy implementations for non-AVX512 builds */
int test_blend_v64qi(int iter) { return 0; }
int test_blend_v32hi(int iter) { return 0; }
int test_blend_v32hf(int iter) { return 0; }
int test_blend_v32bf(int iter) { return 0; }
int test_blend_v16si(int iter) { return 0; }
int test_blend_v8di(int iter) { return 0; }
int test_blend_v8df(int iter) { return 0; }
int test_blend_v16sf(int iter) { return 0; }

#endif /* __AVX512F__ */

int main(int argc, char *argv[]) {
    const int ITERATIONS = 50;
    volatile int total_result = 0;
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported, some tests will be limited\n");
    }
    
    // Determine which test to run based on command line
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    // Run selected tests in a loop
    for (int loop = 0; loop < 10; loop++) {
        if (strcmp(test_mode, "v64qi") == 0 || strcmp(test_mode, "all") == 0) {
            if (has_avx512bw) {
                total_result += test_blend_v64qi(ITERATIONS);
            }
        }
        
        if (strcmp(test_mode, "v32hi") == 0 || strcmp(test_mode, "all") == 0) {
            if (has_avx512bw) {
                total_result += test_blend_v32hi(ITERATIONS);
            }
        }
        
        if (strcmp(test_mode, "v32hf") == 0 || strcmp(test_mode, "all") == 0) {
            if (has_avx512bw) {
                total_result += test_blend_v32hf(ITERATIONS);
            }
        }
        
        if (strcmp(test_mode, "v32bf") == 0 || strcmp(test_mode, "all") == 0) {
            if (has_avx512bw) {
                total_result += test_blend_v32bf(ITERATIONS);
            }
        }
        
        if (strcmp(test_mode, "v16si") == 0 || strcmp(test_mode, "all") == 0) {
            total_result += test_blend_v16si(ITERATIONS);
        }
        
        if (strcmp(test_mode, "v8di") == 0 || strcmp(test_mode, "all") == 0) {
            total_result += test_blend_v8di(ITERATIONS);
        }
        
        if (strcmp(test_mode, "v8df") == 0 || strcmp(test_mode, "all") == 0) {
            total_result += test_blend_v8df(ITERATIONS);
        }
        
        if (strcmp(test_mode, "v16sf") == 0 || strcmp(test_mode, "all") == 0) {
            total_result += test_blend_v16sf(ITERATIONS);
        }
    }
    
    printf("Total result: %d\n", total_result);
    return 0;
}
