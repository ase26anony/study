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
        
        // Method 1: Constant mask pattern
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAAULL;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a, _mm512_set1_epi8(1)),
            _mm512_setzero_si512()
        );
        
        // Method 3: Loop-dependent mask
        __mmask64 mask3 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x5555555555555555ULL;
        
        // Use different masks to ensure various code paths
        __m512i blend1 = _mm512_mask_blend_epi8(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi8(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi8(mask3, blend2, b);
        
        // Extract and accumulate to prevent optimization
        volatile char temp[64];
        _mm512_storeu_si512((void*)temp, blend3);
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
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        // Multiple mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = _mm512_cmpeq_epi16_mask(
            _mm512_and_si512(a, _mm512_set1_epi16(1)),
            _mm512_setzero_si512()
        );
        __mmask32 mask3 = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        
        __m512i blend1 = _mm512_mask_blend_epi16(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi16(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi16(mask3, blend2, b);
        
        // Reduce and accumulate
        result += _mm512_reduce_add_epi16(blend3);
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
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
        
        __mmask32 mask = 0x55555555 ^ (i * 0x11111111);
        __m512h blend = _mm512_mask_blend_ph(mask, a, b);
        
        // Store and accumulate
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
        __m512bh a = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512bh b = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __mmask32 mask = 0xAAAAAAAA | (i & 0x55555555);
        __m512bh blend = _mm512_mask_blend_ph(mask, a, b);
        
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, (_Float16*)&blend);
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
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        // Multiple blending strategies
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = _mm512_cmpeq_epi32_mask(
            _mm512_and_si512(a, _mm512_set1_epi32(1)),
            _mm512_setzero_si512()
        );
        __mmask16 mask3 = (i % 3) ? 0xFFFF : 0x5555;
        
        __m512i blend1 = _mm512_mask_blend_epi32(mask1, a, b);
        __m512i blend2 = _mm512_mask_blend_epi32(mask2, blend1, a);
        __m512i blend3 = _mm512_mask_blend_epi32(mask3, blend2, b);
        
        result += _mm512_reduce_add_epi32(blend3);
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        
        __mmask8 mask = 0xAA ^ (i & 0x55);
        __m512i blend = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blend);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        
        __mmask8 mask = 0x55 | ((i % 2) << 1);
        __m512d blend = _mm512_mask_blend_pd(mask, a, b);
        
        // Horizontal add and accumulate
        __m256d low = _mm512_castpd512_pd256(blend);
        __m256d high = _mm512_extractf64x4_pd(blend, 1);
        __m256d sum = _mm256_add_pd(low, high);
        __m128d sum128 = _mm_add_pd(_mm256_castpd256_pd128(sum), 
                                   _mm256_extractf128_pd(sum, 1));
        result += (int)_mm_cvtsd_f64(sum128);
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
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
        
        __mmask16 mask = 0xAAAA ^ (i * 0x1111);
        __m512 blend = _mm512_mask_blend_ps(mask, a, b);
        
        // Reduce and accumulate
        result += (int)_mm512_reduce_add_ps(blend);
    }
    
    return result;
}

#endif // __AVX512F__

// Fallback dummy implementations for compilation without AVX-512
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
    
    const int iterations = 50;
    volatile int total_sum = 0;
    
    // Determine which test to run based on command line
    char *test_to_run = (argc > 1) ? argv[1] : "all";
    
    if (strcmp(test_to_run, "v64qi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v64qi(iterations);
            }
            printf("V64QImode tests completed\n");
        } else {
            printf("AVX-512BW required for V64QImode\n");
        }
    }
    
    if (strcmp(test_to_run, "v32hi") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hi(iterations);
            }
            printf("V32HImode tests completed\n");
        } else {
            printf("AVX-512BW required for V32HImode\n");
        }
    }
    
    if (strcmp(test_to_run, "v32hf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hf(iterations);
            }
            printf("V32HFmode tests completed\n");
        } else {
            printf("AVX-512BW required for V32HFmode\n");
        }
    }
    
    if (strcmp(test_to_run, "v32bf") == 0 || strcmp(test_to_run, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32bf(iterations);
            }
            printf("V32BFmode tests completed\n");
        } else {
            printf("AVX-512BW required for V32BFmode\n");
        }
    }
    
    if (strcmp(test_to_run, "v16si") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16si(iterations);
        }
        printf("V16SImode tests completed\n");
    }
    
    if (strcmp(test_to_run, "v8di") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8di(iterations);
        }
        printf("V8DImode tests completed\n");
    }
    
    if (strcmp(test_to_run, "v8df") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8df(iterations);
        }
        printf("V8DFmode tests completed\n");
    }
    
    if (strcmp(test_to_run, "v16sf") == 0 || strcmp(test_to_run, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16sf(iterations);
        }
        printf("V16SFmode tests completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
