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
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        // Generate mask in multiple ways
        __mmask64 mask;
        if (i % 3 == 0) {
            // Constant pattern mask
            mask = 0xAAAAAAAAAAAAAAAA;
        } else if (i % 3 == 1) {
            // Data-dependent mask
            mask = _mm512_cmpeq_epi8_mask(a, b);
        } else {
            // Pattern based on iteration
            mask = (i & 1) ? 0x5555555555555555 : 0xFFFFFFFFFFFFFFFF;
        }
        
        // Perform blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var = blended;
        
        // Extract and accumulate result
        result += _mm512_extract_epi8(blended, i % 64);
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create vectors with arithmetic sequences
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500,
            1600, 1700, 1800, 1900, 2000, 2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000, 3100
        );
        
        // Variable mask generation
        __mmask32 mask;
        if (i % 4 == 0) {
            mask = 0xAAAAAAAA;
        } else if (i % 4 == 1) {
            mask = _mm512_cmpeq_epi16_mask(a, _mm512_set1_epi16(15));
        } else if (i % 4 == 2) {
            mask = _mm512_cmpgt_epi16_mask(a, b);
        } else {
            mask = (1U << (i % 32)) - 1;
        }
        
        // Blend operation
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        result += _mm512_extract_epi16(blended, i % 32);
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
            31.0f, 30.5f, 29.0f, 28.5f, 27.0f, 26.5f, 25.0f, 24.5f,
            23.0f, 22.5f, 21.0f, 20.5f, 19.0f, 18.5f, 17.0f, 16.5f,
            15.0f, 14.5f, 13.0f, 12.5f, 11.0f, 10.5f, 9.0f, 8.5f,
            7.0f, 6.5f, 5.0f, 4.5f, 3.0f, 2.5f, 1.0f, 0.5f
        );
        
        __m512h b = _mm512_set_ph(
            0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f,
            4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f, 8.0f,
            8.5f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 11.5f, 12.0f,
            12.5f, 13.0f, 13.5f, 14.0f, 14.5f, 15.0f, 15.5f, 16.0f
        );
        
        // Generate mask
        __mmask32 mask;
        if (i % 2 == 0) {
            mask = 0x55555555;
        } else {
            // Compare for equality with a specific value
            mask = _mm512_cmpeq_ph_mask(a, _mm512_set1_ph(15.0f));
        }
        
        // Blend half-precision floats
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __m512h store_var = blended;
        
        // Extract and convert to integer for accumulation
        short extracted = _mm512_extract_ph(blended, i % 32);
        result += (int)extracted;
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
            0x3F80, 0x4000, 0x4040, 0x4080, 0x40A0, 0x40C0, 0x40E0, 0x4100,
            0x4110, 0x4120, 0x4130, 0x4140, 0x4150, 0x4160, 0x4170, 0x4180,
            0x4188, 0x4190, 0x4198, 0x41A0, 0x41A8, 0x41B0, 0x41B8, 0x41C0,
            0x41C8, 0x41D0, 0x41D8, 0x41E0, 0x41E8, 0x41F0, 0x41F8, 0x4200
        );
        
        __m512bh b = _mm512_set_epi16(
            0x4200, 0x41F8, 0x41F0, 0x41E8, 0x41E0, 0x41D8, 0x41D0, 0x41C8,
            0x41C0, 0x41B8, 0x41B0, 0x41A8, 0x41A0, 0x4198, 0x4190, 0x4188,
            0x4180, 0x4170, 0x4160, 0x4150, 0x4140, 0x4130, 0x4120, 0x4110,
            0x4100, 0x40E0, 0x40C0, 0x40A0, 0x4080, 0x4040, 0x4000, 0x3F80
        );
        
        // Generate mask
        __mmask32 mask = (i % 2 == 0) ? 0xAAAAAAAA : 0x55555555;
        
        // Blend bfloat16 values
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512bh store_var = blended;
        
        // Extract and accumulate
        unsigned short extracted = _mm512_extract_epi16(blended, i % 32);
        result += (int)extracted;
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
        // Create 32-bit integer vectors
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            0, 100, 200, 300, 400, 500, 600, 700,
            800, 900, 1000, 1100, 1200, 1300, 1400, 1500
        );
        
        // Generate mask using different methods
        __mmask16 mask;
        if (i % 3 == 0) {
            mask = 0xAAAA;
        } else if (i % 3 == 1) {
            mask = _mm512_cmpeq_epi32_mask(a, _mm512_set1_epi32(7));
        } else {
            mask = _mm512_cmpgt_epi32_mask(a, b);
        }
        
        // Blend 32-bit integers
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        result += _mm512_extract_epi32(blended, i % 16);
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile long long result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create 64-bit integer vectors
        __m512i a = _mm512_set_epi64(
            7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi64(
            0, 1000, 2000, 3000, 4000, 5000, 6000, 7000
        );
        
        // Generate mask
        __mmask8 mask;
        if (i % 2 == 0) {
            mask = 0xAA;
        } else {
            mask = _mm512_cmpgt_epi64_mask(a, b);
        }
        
        // Blend 64-bit integers
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        volatile __m512i store_var = blended;
        
        // Extract and accumulate
        result += _mm512_extract_epi64(blended, i % 8);
    }
    
    return (int)result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create double-precision float vectors
        __m512d a = _mm512_set_pd(
            7.0, 6.5, 6.0, 5.5, 5.0, 4.5, 4.0, 3.5
        );
        
        __m512d b = _mm512_set_pd(
            3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0
        );
        
        // Generate mask using comparison
        __mmask8 mask;
        if (i % 3 == 0) {
            mask = 0x55;
        } else if (i % 3 == 1) {
            mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
        } else {
            mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(5.0), _CMP_EQ_OQ);
        }
        
        // Blend double-precision floats
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        volatile __m512d store_var = blended;
        
        // Extract and convert to integer
        double extracted = _mm512_cvtsd_f64(_mm512_castpd512_pd128(blended));
        result += (int)extracted;
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Create single-precision float vectors
        __m512 a = _mm512_set_ps(
            15.0f, 14.5f, 14.0f, 13.5f, 13.0f, 12.5f, 12.0f, 11.5f,
            11.0f, 10.5f, 10.0f, 9.5f, 9.0f, 8.5f, 8.0f, 7.5f
        );
        
        __m512 b = _mm512_set_ps(
            7.5f, 8.0f, 8.5f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f,
            11.5f, 12.0f, 12.5f, 13.0f, 13.5f, 14.0f, 14.5f, 15.0f
        );
        
        // Generate mask using various methods
        __mmask16 mask;
        if (i % 4 == 0) {
            mask = 0xAAAA;
        } else if (i % 4 == 1) {
            mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        } else if (i % 4 == 2) {
            mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_EQ_OQ);
        } else {
            mask = (1U << (i % 16)) - 1;
        }
        
        // Blend single-precision floats
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        volatile __m512 store_var = blended;
        
        // Extract and convert to integer
        float extracted = _mm512_cvtss_f32(_mm512_castps512_ps128(blended));
        result += (int)extracted;
    }
    
    return result;
}

#endif // __AVX512F__

// Dummy implementations for non-AVX512 builds
#ifndef __AVX512F__
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
    
    printf("AVX-512F supported: %s\n", has_avx512f ? "YES" : "NO");
    printf("AVX-512BW supported: %s\n", has_avx512bw ? "YES" : "NO");
    
    // Determine which test to run
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    volatile int total_result = 0;
    const int iterations = 50;  // Enough to trigger expansion
    
    // Run tests based on mode
    if (strcmp(test_mode, "v64qi") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            printf("Testing V64QImode blend...\n");
            total_result += test_blend_v64qi(iterations);
        }
    }
    
    if (strcmp(test_mode, "v32hi") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            printf("Testing V32HImode blend...\n");
            total_result += test_blend_v32hi(iterations);
        }
    }
    
    if (strcmp(test_mode, "v32hf") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            printf("Testing V32HFmode blend...\n");
            total_result += test_blend_v32hf(iterations);
        }
    }
    
    if (strcmp(test_mode, "v32bf") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            printf("Testing V32BFmode blend...\n");
            total_result += test_blend_v32bf(iterations);
        }
    }
    
    if (strcmp(test_mode, "v16si") == 0 || strcmp(test_mode, "all") == 0) {
        printf("Testing V16SImode blend...\n");
        total_result += test_blend_v16si(iterations);
    }
    
    if (strcmp(test_mode, "v8di") == 0 || strcmp(test_mode, "all") == 0) {
        printf("Testing V8DImode blend...\n");
        total_result += test_blend_v8di(iterations);
    }
    
    if (strcmp(test_mode, "v8df") == 0 || strcmp(test_mode, "all") == 0) {
        printf("Testing V8DFmode blend...\n");
        total_result += test_blend_v8df(iterations);
    }
    
    if (strcmp(test_mode, "v16sf") == 0 || strcmp(test_mode, "all") == 0) {
        printf("Testing V16SFmode blend...\n");
        total_result += test_blend_v16sf(iterations);
    }
    
    printf("Final accumulated result: %d\n", total_result);
    
    return 0;
}
