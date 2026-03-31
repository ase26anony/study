#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64 x 8-bit integers */
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
        __mmask64 mask1 = 0xAAAAAAAAAAAAAAAA;
        
        // Method 2: Data-dependent mask
        __mmask64 mask2 = _mm512_cmpeq_epi8_mask(
            _mm512_and_si512(a, _mm512_set1_epi8(1)),
            _mm512_setzero_si512()
        );
        
        // Method 3: Alternating pattern based on iteration
        __mmask64 mask3 = (i % 2) ? 0xFFFFFFFFFFFFFFFF : 0xAAAAAAAAAAAAAAAA;
        
        // Use different masks to ensure various code paths
        __m512i blended;
        if (i % 3 == 0) {
            blended = _mm512_mask_blend_epi8(mask1, a, b);
        } else if (i % 3 == 1) {
            blended = _mm512_mask_blend_epi8(mask2, a, b);
        } else {
            blended = _mm512_mask_blend_epi8(mask3, a, b);
        }
        
        // Extract and accumulate result to prevent optimization
        volatile char temp[64];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 64];
    }
    
    return result;
}

/* V32HImode - 32 x 16-bit integers */
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
        
        // Various mask generation strategies
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        __mmask32 mask3 = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        
        __m512i blended;
        switch (i % 4) {
            case 0: blended = _mm512_mask_blend_epi16(mask1, a, b); break;
            case 1: blended = _mm512_mask_blend_epi16(mask2, a, b); break;
            case 2: blended = _mm512_mask_blend_epi16(mask3, a, b); break;
            default: blended = _mm512_mask_blend_epi16(0x55555555, a, b); break;
        }
        
        // Extract and accumulate
        volatile short temp[32];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 32];
    }
    
    return result;
}

/* V32HFmode - 32 x half-precision floats */
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
        
        __mmask32 mask = 0xAAAAAAAA;
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        // Extract and accumulate
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended);
        result += (int)temp[i % 32];
    }
    
    return result;
}

/* V32BFmode - 32 x bfloat16 floats */
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
        
        __mmask32 mask = 0x55555555;
        __m512bh blended = _mm512_mask_blend_ph(mask, a, b);
        
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

/* V16SImode - 16 x 32-bit integers */
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
        
        // Multiple mask generation approaches
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(7));
        __mmask16 mask3 = (__mmask16)((i * 0x1111) & 0xFFFF);
        
        __m512i blended;
        if (i % 2 == 0) {
            blended = _mm512_mask_blend_epi32(mask1, a, b);
        } else {
            blended = _mm512_mask_blend_epi32(mask2, a, b);
        }
        
        // Alternate with third mask
        if (i % 5 == 0) {
            blended = _mm512_mask_blend_epi32(mask3, a, b);
        }
        
        // Extract and accumulate
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 16];
    }
    
    return result;
}

/* V8DImode - 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile long long result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(3));
        __mmask8 mask3 = (__mmask8)((i * 0x55) & 0xFF);
        
        __m512i blended;
        switch (i % 3) {
            case 0: blended = _mm512_mask_blend_epi64(mask1, a, b); break;
            case 1: blended = _mm512_mask_blend_epi64(mask2, a, b); break;
            case 2: blended = _mm512_mask_blend_epi64(mask3, a, b); break;
        }
        
        // Extract and accumulate
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended);
        result += temp[i % 8];
    }
    
    return (int)result;
}

/* V8DFmode - 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        __mmask8 mask3 = (__mmask8)((0x55 << (i % 8)) & 0xFF);
        
        __m512d blended;
        if (i % 4 == 0) {
            blended = _mm512_mask_blend_pd(mask1, a, b);
        } else if (i % 4 == 1) {
            blended = _mm512_mask_blend_pd(mask2, a, b);
        } else {
            blended = _mm512_mask_blend_pd(mask3, a, b);
        }
        
        // Extract and accumulate
        volatile double temp[8];
        _mm512_storeu_pd(temp, blended);
        result += (int)temp[i % 8];
    }
    
    return result;
}

/* V16SFmode - 16 x single-precision floats */
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
        
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_GT_OQ);
        __mmask16 mask3 = (__mmask16)((0x5555 << (i % 4)) & 0xFFFF);
        
        __m512 blended;
        if (i < iter/2) {
            blended = _mm512_mask_blend_ps(mask1, a, b);
        } else {
            blended = _mm512_mask_blend_ps(mask2, a, b);
        }
        
        // Use third mask occasionally
        if (i % 7 == 0) {
            blended = _mm512_mask_blend_ps(mask3, a, b);
        }
        
        // Extract and accumulate
        volatile float temp[16];
        _mm512_storeu_ps(temp, blended);
        result += (int)temp[i % 16];
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
    int has_avx512bf16 = __builtin_cpu_supports("avx512bf16");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    printf("CPU supports: AVX-512F=%d, AVX-512BW=%d, AVX-512BF16=%d\n",
           has_avx512f, has_avx512bw, has_avx512bf16);
    
    // Determine which test to run
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    volatile int total_result = 0;
    const int iterations = 50;  // Enough to trigger expansion
    
    // Run tests based on mode
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            total_result += test_blend_v64qi(iterations);
            printf("Ran V64QImode test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            total_result += test_blend_v32hi(iterations);
            printf("Ran V32HImode test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            total_result += test_blend_v32hf(iterations);
            printf("Ran V32HFmode test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw && has_avx512bf16) {
            total_result += test_blend_v32bf(iterations);
            printf("Ran V32BFmode test\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        total_result += test_blend_v16si(iterations);
        printf("Ran V16SImode test\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        total_result += test_blend_v8di(iterations);
        printf("Ran V8DImode test\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        total_result += test_blend_v8df(iterations);
        printf("Ran V8DFmode test\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        total_result += test_blend_v16sf(iterations);
        printf("Ran V16SFmode test\n");
    }
    
    printf("Final accumulated result: %d\n", total_result);
    
    return 0;
}
