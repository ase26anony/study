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
        
        // Use different masks in different iterations
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i a = _mm512_set1_epi8(i);
        __m512i b = _mm512_set1_epi8(i * 2);
        
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Extract and accumulate result
        volatile char temp[64];
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
        // Multiple mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512i vec_a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, vec_b);
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i a = _mm512_set1_epi16(i);
        __m512i b = _mm512_set1_epi16(i * 3);
        
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        // Extract and accumulate
        volatile short temp[32];
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
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Create comparison mask
        __m512h a_half = _mm512_set1_ph((_Float16)i);
        __m512h b_half = _mm512_set1_ph((_Float16)(i + 1));
        __mmask32 mask3 = _mm512_cmp_ph_mask(a_half, b_half, _CMP_LT_OQ);
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512h blended = _mm512_mask_blend_ph(mask, a_half, b_half);
        
        // Extract and accumulate
        volatile _Float16 temp[32];
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
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Create bfloat16 vectors
        __m512bh a_bfloat = _mm512_set1_bh((__bf16)i);
        __m512bh b_bfloat = _mm512_set1_bh((__bf16)(i + 2));
        
        // Comparison for mask (requires AVX512BF16)
        #ifdef __AVX512BF16__
        __mmask32 mask3 = _mm512_cmp_ph_mask(
            _mm512_cvtne2ps_pbh(_mm512_set1_ps(i), _mm512_set1_ps(i)),
            _mm512_cvtne2ps_pbh(_mm512_set1_ps(i+1), _mm512_set1_ps(i+1)),
            _CMP_LT_OQ
        );
        #else
        __mmask32 mask3 = mask1;
        #endif
        
        __mmask32 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512bh blended = _mm512_mask_blend_ph(mask, a_bfloat, b_bfloat);
        
        // Extract and accumulate
        volatile __bf16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
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
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512i vec_a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        __m512i vec_b = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, vec_b);
        
        __mmask16 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i a = _mm512_set1_epi32(i);
        __m512i b = _mm512_set1_epi32(i * 4);
        
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        // Extract and accumulate
        volatile int temp[16];
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
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512i vec_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i vec_b = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec_a, vec_b);
        
        __mmask8 mask = (i % 3 == 0) ? mask1 : 
                       (i % 3 == 1) ? mask2 : mask3;
        
        __m512i a = _mm512_set1_epi64(i);
        __m512i b = _mm512_set1_epi64(i * 5);
        
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += (int)(temp[i % 8] & 0xFFFFFFFF);
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
        __m512d vec_b = _mm512_set1_pd(i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_LT_OQ);
        
        __mmask8 mask = (i % 3 == 0) ? mask1 : 
                       (i % 3 == 1) ? mask2 : mask3;
        
        __m512d a = _mm512_set1_pd(i);
        __m512d b = _mm512_set1_pd(i * 6.0);
        
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        // Extract and accumulate
        volatile double temp[8];
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
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512 vec_a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        __m512 vec_b = _mm512_set1_ps(i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_LT_OQ);
        
        __mmask16 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512 a = _mm512_set1_ps(i);
        __m512 b = _mm512_set1_ps(i * 7.0f);
        
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        // Extract and accumulate
        volatile float temp[16];
        _mm512_storeu_ps(temp, blended);
        result += (int)temp[i % 16];
    }
    
    return result;
}

#endif // __AVX512F__

// Fallback implementations for non-AVX512 builds
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
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported (needed for some operations)\n");
        // Continue anyway for F-only operations
    }
    
    // Determine which test to run based on command line
    const char *test_mode = (argc > 1) ? argv[1] : "all";
    
    printf("Running blend tests with %d iterations...\n", ITERATIONS);
    
    if (strcmp(test_mode, "v64qi") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v64qi(ITERATIONS);
            }
            printf("V64QImode tests completed\n");
        }
    }
    
    if (strcmp(test_mode, "v32hi") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hi(ITERATIONS);
            }
            printf("V32HImode tests completed\n");
        }
    }
    
    if (strcmp(test_mode, "v32hf") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32hf(ITERATIONS);
            }
            printf("V32HFmode tests completed\n");
        }
    }
    
    if (strcmp(test_mode, "v32bf") == 0 || strcmp(test_mode, "all") == 0) {
        if (has_avx512bw) {
            for (int i = 0; i < 20; i++) {
                total_sum += test_blend_v32bf(ITERATIONS);
            }
            printf("V32BFmode tests completed\n");
        }
    }
    
    if (strcmp(test_mode, "v16si") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16si(ITERATIONS);
        }
        printf("V16SImode tests completed\n");
    }
    
    if (strcmp(test_mode, "v8di") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8di(ITERATIONS);
        }
        printf("V8DImode tests completed\n");
    }
    
    if (strcmp(test_mode, "v8df") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v8df(ITERATIONS);
        }
        printf("V8DFmode tests completed\n");
    }
    
    if (strcmp(test_mode, "v16sf") == 0 || strcmp(test_mode, "all") == 0) {
        for (int i = 0; i < 20; i++) {
            total_sum += test_blend_v16sf(ITERATIONS);
        }
        printf("V16SFmode tests completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
