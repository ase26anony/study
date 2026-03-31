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
        
        // Use different masks based on iteration
        __mmask64 mask = (i % 3 == 0) ? mask1 : 
                        (i % 3 == 1) ? mask2 : mask3;
        
        __m512i a = _mm512_set1_epi8(i);
        __m512i b = _mm512_set1_epi8(i * 2);
        
        // The key blend operation for V64QImode
        __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory to prevent optimization
        volatile __m512i store_var;
        store_var = blended;
        
        // Extract and accumulate a result
        result += _mm512_extract_epi8(blended, i % 64);
    }
    
    return result;
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Different mask generation strategies
        __mmask32 mask;
        if (i % 2 == 0) {
            // Constant pattern
            mask = 0xAAAAAAAA;
        } else {
            // Data-dependent pattern
            mask = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        }
        
        __m512i a = _mm512_set1_epi16(i);
        __m512i b = _mm512_set1_epi16(i + 100);
        
        // Blend operation for V32HImode
        __m512i blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512i store_var;
        store_var = blended;
        
        // Reduce and accumulate
        result += _mm512_extract_epi16(blended, i % 32);
    }
    
    return result;
}

/* V32HFmode: 32 x half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask = (__mmask32)((i * 0x88888888) & 0xFFFFFFFF);
        
        // Create half-precision vectors (using _Float16 if available)
        #ifdef __STDC_IEC_559__
        _Float16 val_a = (_Float16)(i * 1.5f);
        _Float16 val_b = (_Float16)(i * 2.5f);
        __m512h a = _mm512_set1_ph(val_a);
        __m512h b = _mm512_set1_ph(val_b);
        
        // Blend operation for V32HFmode
        __m512h blended = _mm512_mask_blend_ph(mask, a, b);
        
        volatile __m512h store_var;
        store_var = blended;
        
        // Extract and convert to integer for accumulation
        result += (int)_mm512_extract_ph(blended, i % 32);
        #else
        // Fallback: use integer operations if half-float not supported
        result += i;
        #endif
    }
    
    return result;
}

/* V32BFmode: 32 x bfloat16 floats */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask = (__mmask32)((i * 0xCCCCCCCC) & 0xFFFFFFFF);
        
        #ifdef __AVX512BF16__
        // Create bfloat16 vectors
        __m512bh a = _mm512_set1_epi16(i << 8);  // Simulate bfloat16
        __m512bh b = _mm512_set1_epi16((i + 64) << 8);
        
        // Blend operation for V32BFmode
        __m512bh blended = _mm512_mask_blend_epi16(mask, a, b);
        
        volatile __m512bh store_var;
        store_var = blended;
        
        result += _mm512_extract_epi16((__m512i)blended, i % 32);
        #else
        result += i * 2;
        #endif
    }
    
    return result;
}

#endif  /* __AVX512BW__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Multiple mask generation methods
        __mmask16 mask;
        switch (i % 4) {
            case 0: mask = 0xAAAA; break;
            case 1: mask = (__mmask16)(i * 0x5555); break;
            case 2: mask = _mm512_cmpeq_epi32_mask(
                _mm512_set1_epi32(i),
                _mm512_set1_epi32(i / 2)
            ); break;
            default: mask = 0xFFFF; break;
        }
        
        __m512i a = _mm512_set1_epi32(i);
        __m512i b = _mm512_set1_epi32(i * 3);
        
        // Blend operation for V16SImode
        __m512i blended = _mm512_mask_blend_epi32(mask, a, b);
        
        volatile __m512i store_var;
        store_var = blended;
        
        // Reduce and accumulate
        result += _mm512_extract_epi32(blended, i % 16);
    }
    
    return result;
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask = (__mmask8)((i * 0x55) & 0xFF);
        
        __m512i a = _mm512_set1_epi64(i);
        __m512i b = _mm512_set1_epi64(i + 1000);
        
        // Blend operation for V8DImode
        __m512i blended = _mm512_mask_blend_epi64(mask, a, b);
        
        volatile __m512i store_var;
        store_var = blended;
        
        result += (int)_mm512_extract_epi64(blended, i % 8);
    }
    
    return result;
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask = (__mmask8)(((i * 0xAA) ^ 0xFF) & 0xFF);
        
        __m512d a = _mm512_set1_pd(i * 1.1);
        __m512d b = _mm512_set1_pd(i * 2.2);
        
        // Blend operation for V8DFmode
        __m512d blended = _mm512_mask_blend_pd(mask, a, b);
        
        volatile __m512d store_var;
        store_var = blended;
        
        result += (int)_mm512_extract_pd(blended, i % 8);
    }
    
    return result;
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Complex mask generation
        __mmask16 mask;
        if (i < iter / 2) {
            mask = _mm512_cmp_ps_mask(
                _mm512_set1_ps(i),
                _mm512_set1_ps(iter / 4),
                _CMP_LT_OS
            );
        } else {
            mask = (__mmask16)((i * 0x3333) & 0xFFFF);
        }
        
        __m512 a = _mm512_set1_ps(i * 0.5f);
        __m512 b = _mm512_set1_ps(i * 1.5f);
        
        // Blend operation for V16SFmode
        __m512 blended = _mm512_mask_blend_ps(mask, a, b);
        
        volatile __m512 store_var;
        store_var = blended;
        
        result += (int)_mm512_extract_ps(blended, i % 16);
    }
    
    return result;
}

#endif  /* __AVX512F__ */

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
    // Check CPU support at runtime
    int has_avx512f = __builtin_cpu_supports("avx512f");
    int has_avx512bw = __builtin_cpu_supports("avx512bw");
    int has_avx512vl = __builtin_cpu_supports("avx512vl");
    
    if (!has_avx512f) {
        printf("AVX-512F not supported on this CPU\n");
        return 0;
    }
    
    if (!has_avx512bw) {
        printf("AVX-512BW not supported (needed for some blend operations)\n");
        // Continue anyway for F-only operations
    }
    
    printf("CPU supports: AVX512F=%d, AVX512BW=%d, AVX512VL=%d\n",
           has_avx512f, has_avx512bw, has_avx512vl);
    
    const int iterations = 50;
    volatile int total_sum = 0;
    
    // Determine which test to run based on command line
    const char *test_mode = "all";
    if (argc > 1) {
        test_mode = argv[1];
    }
    
    // Run tests in a loop to increase expansion opportunities
    for (int loop = 0; loop < 10; loop++) {
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
            total_sum += test_blend_v64qi(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
            total_sum += test_blend_v32hi(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
            total_sum += test_blend_v32hf(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
            total_sum += test_blend_v32bf(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
            total_sum += test_blend_v16si(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
            total_sum += test_blend_v8di(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
            total_sum += test_blend_v8df(iterations);
        }
        
        if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
            total_sum += test_blend_v16sf(iterations);
        }
    }
    
    printf("Total accumulated result: %d\n", total_sum);
    
    return 0;
}
