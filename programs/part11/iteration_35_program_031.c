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
        // Method 1: Constant mask
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = (__mmask64)((i & 1) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL);
        
        // Method 3: Comparison mask
        __m512i vec_a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi8(
            127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112,
            111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96,
            95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
            79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64
        );
        
        __m512i cmp = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, cmp);
        
        // Perform blend with different masks
        __m512i blended1 = _mm512_mask_blend_epi8(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi8(mask2, blended1, vec_a);
        __m512i blended3 = _mm512_mask_blend_epi8(mask3, blended2, vec_b);
        
        // Extract and accumulate result
        volatile uint8_t temp[64];
        _mm512_storeu_si512((void*)temp, blended3);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Multiple mask generation methods
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)((i * 7) & 0xFFFFFFFF);
        
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi16(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32
        );
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, cmp_val);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi16(mask2, blended1, vec_a);
        __m512i blended3 = _mm512_mask_blend_epi16(mask3, blended2, vec_b);
        
        // Extract result
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, blended3);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0x55555555;
        __mmask32 mask2 = (__mmask32)(i * 13);
        
        // Create half-precision vectors (using _Float16)
        _Float16 a_data[32], b_data[32];
        for (int j = 0; j < 32; j++) {
            a_data[j] = (_Float16)(j * 1.5f);
            b_data[j] = (_Float16)(j * 2.5f);
        }
        
        __m512h vec_a = _mm512_set_ph(
            a_data[31], a_data[30], a_data[29], a_data[28], a_data[27], a_data[26], a_data[25], a_data[24],
            a_data[23], a_data[22], a_data[21], a_data[20], a_data[19], a_data[18], a_data[17], a_data[16],
            a_data[15], a_data[14], a_data[13], a_data[12], a_data[11], a_data[10], a_data[9], a_data[8],
            a_data[7], a_data[6], a_data[5], a_data[4], a_data[3], a_data[2], a_data[1], a_data[0]
        );
        
        __m512h vec_b = _mm512_set_ph(
            b_data[31], b_data[30], b_data[29], b_data[28], b_data[27], b_data[26], b_data[25], b_data[24],
            b_data[23], b_data[22], b_data[21], b_data[20], b_data[19], b_data[18], b_data[17], b_data[16],
            b_data[15], b_data[14], b_data[13], b_data[12], b_data[11], b_data[10], b_data[9], b_data[8],
            b_data[7], b_data[6], b_data[5], b_data[4], b_data[3], b_data[2], b_data[1], b_data[0]
        );
        
        // Blend operations
        __m512h blended1 = _mm512_mask_blend_ph(mask1, vec_a, vec_b);
        __m512h blended2 = _mm512_mask_blend_ph(mask2, blended1, vec_a);
        
        // Store and accumulate
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended2);
        result += (int)temp[i % 32];
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0x33333333;
        __mmask32 mask2 = (__mmask32)(i * 17);
        
        // Create bfloat16 vectors
        __m512bh vec_a = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh vec_b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Blend operations
        __m512bh blended1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512bh blended2 = _mm512_mask_blend_epi16(mask2, blended1, vec_a);
        
        // Store and accumulate
        volatile uint16_t temp[32];
        _mm512_storeu_si512((void*)temp, _mm512_castps_si512(_mm512_castbh_ps(blended2)));
        result += temp[i % 32];
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
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 11);
        
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i vec_b = _mm512_set_epi32(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16
        );
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, cmp_val);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi32(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi32(mask2, blended1, vec_a);
        __m512i blended3 = _mm512_mask_blend_epi32(mask3, blended2, vec_b);
        
        // Extract and accumulate
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, blended3);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 3);
        
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec_a, cmp_val);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi64(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi64(mask2, blended1, vec_a);
        __m512i blended3 = _mm512_mask_blend_epi64(mask3, blended2, vec_b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended3);
        result += (int)(temp[i % 8] & 0xFFFFFFFF);
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0x55;
        __mmask8 mask2 = (__mmask8)(i * 5);
        
        __m512d vec_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d vec_b = _mm512_set_pd(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        
        // Comparison mask
        __m512d cmp_val = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, cmp_val, _CMP_EQ_OQ);
        
        // Blend operations
        __m512d blended1 = _mm512_mask_blend_pd(mask1, vec_a, vec_b);
        __m512d blended2 = _mm512_mask_blend_pd(mask2, blended1, vec_a);
        __m512d blended3 = _mm512_mask_blend_pd(mask3, blended2, vec_b);
        
        // Store and accumulate
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended3);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0x5555;
        __mmask16 mask2 = (__mmask16)(i * 7);
        
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 vec_b = _mm512_set_ps(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
        );
        
        // Comparison mask
        __m512 cmp_val = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, cmp_val, _CMP_EQ_OQ);
        
        // Blend operations
        __m512 blended1 = _mm512_mask_blend_ps(mask1, vec_a, vec_b);
        __m512 blended2 = _mm512_mask_blend_ps(mask2, blended1, vec_a);
        __m512 blended3 = _mm512_mask_blend_ps(mask3, blended2, vec_b);
        
        // Store and accumulate
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended3);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif // __AVX512F__

// Dummy implementations for non-AVX512 builds
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

#ifndef __AVX512BW__
int test_blend_v64qi(int iter) { return 0; }
int test_blend_v32hi(int iter) { return 0; }
int test_blend_v32hf(int iter) { return 0; }
int test_blend_v32bf(int iter) { return 0; }
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
    
    const int iterations = 50;
    volatile int total_sum = 0;
    
    // Parse command line argument
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    // Run tests based on mode
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v64qi(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v32hi(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v32hf(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v32bf(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16si(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8di(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8df(iterations);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16sf(iterations);
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
