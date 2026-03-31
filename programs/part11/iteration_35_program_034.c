#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QImode - 64-byte integers */
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
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i vec_b = _mm512_set1_epi8(i);
        __mmask64 mask3 = _mm512_cmpeq_epi8_mask(vec_a, vec_b);
        
        // Blend with different masks
        __m512i blended1 = _mm512_mask_blend_epi8(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi8(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi8(mask3, vec_a, vec_b);
        
        // Extract and accumulate results
        volatile char temp[64];
        _mm512_storeu_si512((void*)temp, blended1);
        result += temp[0] + temp[63];
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += temp[31] + temp[32];
        
        _mm512_storeu_si512((void*)temp, blended3);
        result += temp[16] + temp[47];
    }
    
    return result;
}

/* V32HImode - 32 half-word integers */
__attribute__((noinline))
int test_blend_v32hi(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        // Various mask generation methods
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512i vec_a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i vec_b = _mm512_set1_epi16(i);
        __mmask32 mask3 = _mm512_cmpeq_epi16_mask(vec_a, vec_b);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi16(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi16(mask3, vec_a, vec_b);
        
        // Extract results
        volatile short temp[32];
        _mm512_storeu_si512((void*)temp, blended1);
        result += temp[0] + temp[31];
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += temp[15] + temp[16];
        
        _mm512_storeu_si512((void*)temp, blended3);
        result += temp[8] + temp[23];
    }
    
    return result;
}

/* V32HFmode - 32 half-precision floats */
__attribute__((noinline))
int test_blend_v32hf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        __m512h vec_a = _mm512_set_ph(
            31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
            23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __m512h vec_b = _mm512_set1_ph((_Float16)i);
        
        // Blend operations for half-precision
        __m512h blended1 = _mm512_mask_blend_ph(mask1, vec_a, vec_b);
        __m512h blended2 = _mm512_mask_blend_ph(mask2, vec_a, vec_b);
        
        // Store and accumulate
        volatile _Float16 temp[32];
        _mm512_storeu_ph((void*)temp, blended1);
        result += (int)temp[0] + (int)temp[31];
        
        _mm512_storeu_ph((void*)temp, blended2);
        result += (int)temp[15] + (int)temp[16];
    }
    
    return result;
}

/* V32BFmode - 32 brain-float */
__attribute__((noinline))
int test_blend_v32bf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask32 mask1 = 0xAAAAAAAA;
        __mmask32 mask2 = (__mmask32)(i * 0x55555555);
        
        // Use __m512bh for brain float
        __m512bh vec_a = _mm512_set1_epi16(0x3C00); // 1.0 in bfloat16
        __m512bh vec_b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Blend operations for brain float
        __m512bh blended1 = _mm512_mask_blend_epi16(mask1, vec_a, vec_b);
        __m512bh blended2 = _mm512_mask_blend_epi16(mask2, vec_a, vec_b);
        
        // Store and accumulate
        volatile unsigned short temp[32];
        _mm512_storeu_si512((void*)temp, (__m512i)blended1);
        result += temp[0] + temp[31];
        
        _mm512_storeu_si512((void*)temp, (__m512i)blended2);
        result += temp[15] + temp[16];
    }
    
    return result;
}

#endif // __AVX512BW__
#endif // __AVX512F__

#ifdef __AVX512F__

/* V16SImode - 16 single-word integers */
__attribute__((noinline))
int test_blend_v16si(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512i vec_a = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i vec_b = _mm512_set1_epi32(i);
        __mmask16 mask3 = _mm512_cmpeq_epi32_mask(vec_a, vec_b);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi32(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi32(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi32(mask3, vec_a, vec_b);
        
        // Extract results
        volatile int temp[16];
        _mm512_storeu_si512((void*)temp, blended1);
        result += temp[0] + temp[15];
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += temp[7] + temp[8];
        
        _mm512_storeu_si512((void*)temp, blended3);
        result += temp[4] + temp[11];
    }
    
    return result;
}

/* V8DImode - 8 double-word integers */
__attribute__((noinline))
int test_blend_v8di(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512i vec_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i vec_b = _mm512_set1_epi64(i);
        __mmask8 mask3 = _mm512_cmpeq_epi64_mask(vec_a, vec_b);
        
        // Blend operations
        __m512i blended1 = _mm512_mask_blend_epi64(mask1, vec_a, vec_b);
        __m512i blended2 = _mm512_mask_blend_epi64(mask2, vec_a, vec_b);
        __m512i blended3 = _mm512_mask_blend_epi64(mask3, vec_a, vec_b);
        
        // Extract results
        volatile long long temp[8];
        _mm512_storeu_si512((void*)temp, blended1);
        result += (int)(temp[0] + temp[7]);
        
        _mm512_storeu_si512((void*)temp, blended2);
        result += (int)(temp[3] + temp[4]);
        
        _mm512_storeu_si512((void*)temp, blended3);
        result += (int)(temp[2] + temp[5]);
    }
    
    return result;
}

/* V8DFmode - 8 double-precision floats */
__attribute__((noinline))
int test_blend_v8df(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask8 mask1 = 0xAA;
        __mmask8 mask2 = (__mmask8)(i * 0x55);
        
        __m512d vec_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d vec_b = _mm512_set1_pd((double)i);
        __mmask8 mask3 = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        // Blend operations
        __m512d blended1 = _mm512_mask_blend_pd(mask1, vec_a, vec_b);
        __m512d blended2 = _mm512_mask_blend_pd(mask2, vec_a, vec_b);
        __m512d blended3 = _mm512_mask_blend_pd(mask3, vec_a, vec_b);
        
        // Extract results
        volatile double temp[8];
        _mm512_storeu_pd((void*)temp, blended1);
        result += (int)(temp[0] + temp[7]);
        
        _mm512_storeu_pd((void*)temp, blended2);
        result += (int)(temp[3] + temp[4]);
        
        _mm512_storeu_pd((void*)temp, blended3);
        result += (int)(temp[2] + temp[5]);
    }
    
    return result;
}

/* V16SFmode - 16 single-precision floats */
__attribute__((noinline))
int test_blend_v16sf(int iter) {
    volatile int result = 0;
    
    for (int i = 0; i < iter; i++) {
        __mmask16 mask1 = 0xAAAA;
        __mmask16 mask2 = (__mmask16)(i * 0x5555);
        
        __m512 vec_a = _mm512_set_ps(
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __m512 vec_b = _mm512_set1_ps((float)i);
        __mmask16 mask3 = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_EQ_OQ);
        
        // Blend operations
        __m512 blended1 = _mm512_mask_blend_ps(mask1, vec_a, vec_b);
        __m512 blended2 = _mm512_mask_blend_ps(mask2, vec_a, vec_b);
        __m512 blended3 = _mm512_mask_blend_ps(mask3, vec_a, vec_b);
        
        // Extract results
        volatile float temp[16];
        _mm512_storeu_ps((void*)temp, blended1);
        result += (int)(temp[0] + temp[15]);
        
        _mm512_storeu_ps((void*)temp, blended2);
        result += (int)(temp[7] + temp[8]);
        
        _mm512_storeu_ps((void*)temp, blended3);
        result += (int)(temp[4] + temp[11]);
    }
    
    return result;
}

#endif // __AVX512F__

#ifndef __AVX512F__
// Dummy implementations for non-AVX512 builds
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
    const int ITERATIONS = 20;
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
    
    printf("Running AVX-512 blend tests (iterations: %d)\n", ITERATIONS);
    
    // Run selected tests
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v64qi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v64qi(ITERATIONS);
            printf("  V64QImode test completed\n");
        } else {
            printf("  V64QImode skipped (AVX-512BW required)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hi") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hi(ITERATIONS);
            printf("  V32HImode test completed\n");
        } else {
            printf("  V32HImode skipped (AVX-512BW required)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32hf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32hf(ITERATIONS);
            printf("  V32HFmode test completed\n");
        } else {
            printf("  V32HFmode skipped (AVX-512BW required)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v32bf") == 0) {
        if (has_avx512bw) {
            total_sum += test_blend_v32bf(ITERATIONS);
            printf("  V32BFmode test completed\n");
        } else {
            printf("  V32BFmode skipped (AVX-512BW required)\n");
        }
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16si") == 0) {
        total_sum += test_blend_v16si(ITERATIONS);
        printf("  V16SImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8di") == 0) {
        total_sum += test_blend_v8di(ITERATIONS);
        printf("  V8DImode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v8df") == 0) {
        total_sum += test_blend_v8df(ITERATIONS);
        printf("  V8DFmode test completed\n");
    }
    
    if (strcmp(test_mode, "all") == 0 || strcmp(test_mode, "v16sf") == 0) {
        total_sum += test_blend_v16sf(ITERATIONS);
        printf("  V16SFmode test completed\n");
    }
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
