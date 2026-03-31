#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64x 8-bit integers */
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
            mask = (__mmask64)(i * 0x5555555555555555);
        } else {
            // Comparison-based mask
            __m512i cmp_val = _mm512_set1_epi8(32);
            mask = _mm512_cmpgt_epi8_mask(a, cmp_val);
        }
        
        // Perform blend operation
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory and extract result
        volatile __m512i store_var = blended;
        volatile char arr[64];
        _mm512_storeu_si512((void*)arr, store_var);
        
        // Accumulate some elements
        result += arr[0] + arr[63] + arr[i % 64];
    }
    
    return result;
}

/* V32HImode - 32x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __mmask32 mask;
        if (i % 3 == 0) {
            mask = 0xAAAAAAAA;
        } else if (i % 3 == 1) {
            mask = (__mmask32)(i * 0x55555555);
        } else {
            __m512i cmp_val = _mm512_set1_epi16(16);
            mask = _mm512_cmpgt_epi16_mask(a, cmp_val);
        }
        
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512i store_var = blended;
        volatile short arr[32];
        _mm512_storeu_si512((void*)arr, store_var);
        
        result += arr[0] + arr[31] + arr[i % 32];
    }
    
    return result;
}

/* V32HFmode - 32x half precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512h a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512h b = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __mmask32 mask;
        if (i % 3 == 0) {
            mask = 0xAAAAAAAA;
        } else if (i % 3 == 1) {
            mask = (__mmask32)(i * 0x55555555);
        } else {
            __m512h cmp_val = _mm512_set1_ph(16.0f);
            mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
        }
        
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __m512h store_var = blended;
        volatile _Float16 arr[32];
        _mm512_storeu_ph((void*)arr, store_var);
        
        result += (int)arr[0] + (int)arr[31] + (int)arr[i % 32];
    }
    
    return result;
}

/* V32BFmode - 32x bfloat16 */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512bh a = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        __mmask32 mask;
        if (i % 3 == 0) {
            mask = 0xAAAAAAAA;
        } else if (i % 3 == 1) {
            mask = (__mmask32)(i * 0x55555555);
        } else {
            mask = (__mmask32)((i & 1) ? 0xFFFFFFFF : 0x00000000);
        }
        
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512bh store_var = blended;
        volatile unsigned short arr[32];
        _mm512_storeu_si512((void*)arr, (__m512i)store_var);
        
        result += arr[0] + arr[31] + arr[i % 32];
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode - 16x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7,
            8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __mmask16 mask;
        if (i % 3 == 0) {
            mask = 0xAAAA;
        } else if (i % 3 == 1) {
            mask = (__mmask16)(i * 0x5555);
        } else {
            __m512i cmp_val = _mm512_set1_epi32(8);
            mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
        }
        
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        volatile __m512i store_var = blended;
        volatile int arr[16];
        _mm512_storeu_si512((void*)arr, store_var);
        
        result += arr[0] + arr[15] + arr[i % 16];
    }
    
    return result;
}

/* V8DImode - 8x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        
        __mmask8 mask;
        if (i % 3 == 0) {
            mask = 0xAA;
        } else if (i % 3 == 1) {
            mask = (__mmask8)(i * 0x55);
        } else {
            __m512i cmp_val = _mm512_set1_epi64(4);
            mask = _mm512_cmpgt_epi64_mask(a, cmp_val);
        }
        
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        volatile __m512i store_var = blended;
        volatile long long arr[8];
        _mm512_storeu_si512((void*)arr, store_var);
        
        result += (int)(arr[0] + arr[7] + arr[i % 8]);
    }
    
    return result;
}

/* V8DFmode - 8x double precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        
        __mmask8 mask;
        if (i % 3 == 0) {
            mask = 0xAA;
        } else if (i % 3 == 1) {
            mask = (__mmask8)(i * 0x55);
        } else {
            __m512d cmp_val = _mm512_set1_pd(4.0);
            mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        }
        
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        volatile __m512d store_var = blended;
        volatile double arr[8];
        _mm512_storeu_pd((void*)arr, store_var);
        
        result += (int)(arr[0] + arr[7] + arr[i % 8]);
    }
    
    return result;
}

/* V16SFmode - 16x single precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512 a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 b = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __mmask16 mask;
        if (i % 3 == 0) {
            mask = 0xAAAA;
        } else if (i % 3 == 1) {
            mask = (__mmask16)(i * 0x5555);
        } else {
            __m512 cmp_val = _mm512_set1_ps(8.0f);
            mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        }
        
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        volatile __m512 store_var = blended;
        volatile float arr[16];
        _mm512_storeu_ps((void*)arr, store_var);
        
        result += (int)(arr[0] + arr[15] + arr[i % 16]);
    }
    
    return result;
}

#endif // __AVX512F__

/* Dummy implementations for non-AVX512 builds */
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
    const int ITERATIONS = 50;
    volatile int total_sum = 0;
    
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    // Determine which test to run
    char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    printf("Running AVX-512 blend tests...\n");
    printf("AVX-512F: %s, AVX-512BW: %s\n", 
           has_avx512f ? "yes" : "no",
           has_avx512bw ? "yes" : "no");
    
    // Run tests based on mode
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v64qi(ITERATIONS);
            printf("V64QImode test completed\n");
        } else {
            printf("Skipping V64QImode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hi(ITERATIONS);
            printf("V32HImode test completed\n");
        } else {
            printf("Skipping V32HImode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hf(ITERATIONS);
            printf("V32HFmode test completed\n");
        } else {
            printf("Skipping V32HFmode (requires AVX-512BW)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32bf(ITERATIONS);
            printf("V32BFmode test completed\n");
        } else {
            printf("Skipping V32BFmode (requires AVX-512BW)\n");
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
