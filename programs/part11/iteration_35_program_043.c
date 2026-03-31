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
        
        // Method 1: Constant mask pattern
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAA; // Alternating pattern
        
        // Method 2: Data-dependent mask based on iteration
        __mmask64 mask2 = (i & 1) ? 0xFFFFFFFFFFFFFFFF : 0xAAAAAAAAAAAAAAAA;
        
        // Method 3: Comparison-based mask
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask3 = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Use all three masks in sequence
        __m512i blend1 = _mm512_mask_blend_epi8(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi8(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi8(mask3, blend2, b);
        
        // Extract and accumulate result
        volatile char temp[64];
        _mm512_storeu_epi8((void*)temp, blend3);
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
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        
        __m512i b = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Different mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA; // Alternating pattern
        
        // Data-dependent mask
        __mmask32 mask2 = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        
        // Comparison-based mask
        __m512i cmp_val = _mm512_set1_epi16(16);
        __mmask32 mask3 = _mm512_cmpgt_epi16_mask(a, cmp_val);
        
        __m512i blend1 = _mm512_mask_blend_epi16(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi16(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi16(mask3, blend2, b);
        
        volatile short temp[32];
        _mm512_storeu_epi16((void*)temp, blend3);
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
        
        __mmask32 mask = 0x55555555; // Checkerboard pattern
        __m512h blend = _mm512_mask_blend_ph(mask, a, b);
        
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blend);
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
        
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        __m512bh blend = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __bf16 temp[32];
        _mm512_storeu_ph((void*)temp, blend);
        result += (int)temp[i % 32];
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
        __m512i a = _mm512_set_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        
        __m512i b = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        
        // Multiple mask generation methods
        __mmask16 mask1 = 0xAAAA; // Alternating
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x5555) & 0xFFFF);
        
        // Comparison-based mask
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask3 = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        __m512i blend1 = _mm512_mask_blend_epi32(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi32(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi32(mask3, blend2, b);
        
        volatile int temp[16];
        _mm512_storeu_epi32((void*)temp, blend3);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        
        __mmask8 mask1 = 0xAA; // Alternating pattern
        
        // Data-dependent mask
        __mmask8 mask2 = (__mmask8)((i * 0x55) & 0xFF);
        
        // Comparison-based mask
        __m512i cmp_val = _mm512_set1_epi64(4);
        __mmask8 mask3 = _mm512_cmpgt_epi64_mask(a, cmp_val);
        
        __m512i blend1 = _mm512_mask_blend_epi64(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi64(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi64(mask3, blend2, b);
        
        volatile long long temp[8];
        _mm512_storeu_epi64((void*)temp, blend3);
        result += (int)(temp[i % 8] & 0xFFFFFFFF);
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
        
        __mmask8 mask1 = 0xAA; // Alternating
        
        // Data-dependent mask
        __mmask8 mask2 = (__mmask8)((i * 0x55) & 0xFF);
        
        // Comparison-based mask
        __m512d cmp_val = _mm512_set1_pd(4.0);
        __mmask8 mask3 = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        
        __m512d blend1 = _mm512_mask_blend_pd(mask1, a, b);
        __m512d blend2 = _mm512_mask_blend_pd(mask2, blend1, a);
        __m512d blend3 = _mm512_mask_blend_pd(mask3, blend2, b);
        
        volatile double temp[8];
        _mm512_storeu_pd(temp, blend3);
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
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        
        __mmask16 mask1 = 0xAAAA; // Alternating
        
        // Data-dependent mask
        __mmask16 mask2 = (__mmask16)((i * 0x5555) & 0xFFFF);
        
        // Comparison-based mask
        __m512 cmp_val = _mm512_set1_ps(8.0f);
        __mmask16 mask3 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        
        __m512 blend1 = _mm512_mask_blend_ps(mask1, a, b);
        __m512 blend2 = _mm512_mask_blend_ps(mask2, blend1, a);
        __m512 blend3 = _mm512_mask_blend_ps(mask3, blend2, b);
        
        volatile float temp[16];
        _mm512_storeu_ps(temp, blend3);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif // __AVX512F__

/* Fallback implementations for non-AVX512 builds */
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
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    printf("Running AVX-512 blend tests (mode: %s)\n", test_mode);
    
    // Run tests based on command-line argument
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v64qi(ITERATIONS);
            }
        } else {
            printf("AVX-512BW required for v64qi test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hi(ITERATIONS);
            }
        } else {
            printf("AVX-512BW required for v32hi test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hf(ITERATIONS);
            }
        } else {
            printf("AVX-512BW required for v32hf test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32bf(ITERATIONS);
            }
        } else {
            printf("AVX-512BW required for v32bf test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16si(ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8di(ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8df(ITERATIONS);
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16sf(ITERATIONS);
        }
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
