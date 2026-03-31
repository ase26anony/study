#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile __m512i g_v64qi_result;
volatile __m512i g_v32hi_result;
volatile __m512h g_v32hf_result;
volatile __m512bh g_v32bf_result;
volatile __m512i g_v16si_result;
volatile __m512i g_v8di_result;
volatile __m512d g_v8df_result;
volatile __m512 g_v16sf_result;

// Function prototypes with explicit target attributes
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask);

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask);

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask);

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask);

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask);

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask);

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask);

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask);

// Multi-stage pipeline functions
__attribute__((target("avx512bw,avx512f")))
void pipeline_char_to_int(uint64_t* checksum);

__attribute__((target("avx512f,avx512fp16")))
void pipeline_float_to_half(uint64_t* checksum);

// Data-dependent computation with loops
__attribute__((target("avx512bw")))
void compute_blend_masks(int argc, __mmask64* mask64, __mmask32* mask32, 
                         __mmask16* mask16, __mmask8* mask8);

// Implementation of blend functions with runtime-derived masks
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using intrinsic with runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile global to prevent elimination
    g_v64qi_result = result;
    
    // Additional computation to create data dependency
    __m512i shuffled = _mm512_shuffle_epi8(result, _mm512_set1_epi8(0x07));
    return _mm512_add_epi8(result, shuffled);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    g_v32hi_result = result;
    
    // Create complex dataflow
    __m512i abs_diff = _mm512_abs_epi16(_mm512_sub_epi16(a, b));
    return _mm512_add_epi16(result, abs_diff);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    g_v32hf_result = result;
    
    // Mixed precision operations
    __m512h scaled = _mm512_mul_ph(result, _mm512_set1_ph(2.0f));
    return _mm512_add_ph(result, scaled);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    g_v32bf_result = result;
    
    // Type conversion stress
    __m512i as_int = _mm512_castps_si512(_mm512_castph_ps(result));
    __m512bh reconverted = _mm512_castsi_ph(as_int);
    return reconverted;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    g_v16si_result = result;
    
    // Conditional computation based on blend result
    __mmask16 nonzero = _mm512_test_epi32_mask(result, result);
    __m512i adjusted = _mm512_mask_add_epi32(result, nonzero, result, _mm512_set1_epi32(1));
    return adjusted;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    g_v8di_result = result;
    
    // Cross-lane operations
    __m512i reversed = _mm512_permutexvar_epi64(_mm512_set_epi64(0,1,2,3,4,5,6,7), result);
    return _mm512_xor_si512(result, reversed);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    g_v8df_result = result;
    
    // Mathematical transformation
    __m512d squared = _mm512_mul_pd(result, result);
    __m512d normalized = _mm512_div_pd(result, _mm512_add_pd(squared, _mm512_set1_pd(1.0)));
    return normalized;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    g_v16sf_result = result;
    
    // Trigonometric approximation
    __m512 sin_approx = _mm512_sub_ps(result, _mm512_mul_ps(_mm512_mul_ps(result, result), result));
    return _mm512_add_ps(result, sin_approx);
}

__attribute__((target("avx512bw,avx512f")))
void pipeline_char_to_int(uint64_t* checksum) {
    // Initialize with runtime values
    volatile int seed = *checksum & 0xFF;
    __m512i chars = _mm512_set1_epi8(seed);
    __m512i chars2 = _mm512_set1_epi8(seed + 1);
    
    // First blend at V64QI mode
    __mmask64 mask64 = (__mmask64)(*checksum ^ 0xAAAAAAAAAAAAAAAA);
    __m512i blended_chars = blend_v64qi(chars, chars2, mask64);
    
    // Convert to V16SI mode through intermediate operations
    __m512i expanded = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(blended_chars));
    __m512i expanded2 = _mm512_set1_epi32(seed + 2);
    
    // Second blend at V16SI mode
    __mmask16 mask16 = (__mmask16)(*checksum & 0xFFFF);
    __m512i blended_ints = blend_v16si(expanded, expanded2, mask16);
    
    // Horizontal add for checksum
    *checksum += _mm512_reduce_add_epi32(blended_ints);
}

__attribute__((target("avx512f,avx512fp16")))
void pipeline_float_to_half(uint64_t* checksum) {
    // Start with V16SF
    volatile float fseed = (float)(*checksum % 1000) / 100.0f;
    __m512 floats = _mm512_set1_ps(fseed);
    __m512 floats2 = _mm512_set1_ps(fseed + 0.5f);
    
    // Blend at V16SF mode
    __mmask16 mask16 = (__mmask16)(*checksum & 0xFFFF);
    __m512 blended_floats = blend_v16sf(floats, floats2, mask16);
    
    // Convert to V32HF
    __m512h halves = _mm512_cvtps_ph(blended_floats, _MM_FROUND_TO_NEAREST_INT);
    __m512h halves2 = _mm512_set1_ph(fseed + 0.25f);
    
    // Blend at V32HF mode
    __mmask32 mask32 = (__mmask32)(*checksum ^ 0xFFFFFFFF);
    __m512h blended_halves = blend_v32hf(halves, halves2, mask32);
    
    // Convert back and accumulate
    __m512 reconverted = _mm512_cvtph_ps(blended_halves);
    *checksum += (uint64_t)_mm512_reduce_add_ps(reconverted);
}

__attribute__((target("avx512bw")))
void compute_blend_masks(int argc, __mmask64* mask64, __mmask32* mask32, 
                         __mmask16* mask16, __mmask8* mask8) {
    // Compute masks based on argc to prevent constant folding
    uint64_t base = (uint64_t)argc * 0x123456789ABCDEF;
    
    // Create pattern-based masks
    *mask64 = (__mmask64)(base ^ (base >> 32));
    *mask32 = (__mmask32)((base >> 16) & 0xFFFFFFFF);
    *mask16 = (__mmask16)((base >> 8) & 0xFFFF);
    *mask8 = (__mmask8)(base & 0xFF);
    
    // Ensure non-zero masks
    if (*mask64 == 0) *mask64 = 0xAAAAAAAAAAAAAAAA;
    if (*mask32 == 0) *mask32 = 0x55555555;
    if (*mask16 == 0) *mask16 = 0xAAAA;
    if (*mask8 == 0) *mask8 = 0x55;
}

int main(int argc, char** argv) {
    uint64_t checksum = 0;
    
    // Initialize test data with argc-dependent seed
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Compute runtime-derived masks
    __mmask64 mask64;
    __mmask32 mask32;
    __mmask16 mask16;
    __mmask8 mask8;
    compute_blend_masks(argc, &mask64, &mask32, &mask16, &mask8);
    
    // Test V64QI blend
    {
        __m512i v64qi_a = _mm512_set_epi8(
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
            17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
            33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
            49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64
        );
        __m512i v64qi_b = _mm512_set1_epi8(rand() % 256);
        __m512i result = blend_v64qi(v64qi_a, v64qi_b, mask64);
        checksum += _mm512_reduce_add_epi8(result);
    }
    
    // Test V32HI blend
    {
        __m512i v32hi_a = _mm512_set_epi16(
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
            17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32
        );
        __m512i v32hi_b = _mm512_set1_epi16(rand() % 65536);
        __m512i result = blend_v32hi(v32hi_a, v32hi_b, mask32);
        checksum += _mm512_reduce_add_epi16(result);
    }
    
    // Test V32HF blend (if supported)
    #ifdef __AVX512FP16__
    {
        __m512h v32hf_a = _mm512_set_ph(
            1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,
            9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f,
            17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,24.0f,
            25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f,32.0f
        );
        __m512h v32hf_b = _mm512_set1_ph((_Float16)(rand() % 1000) / 10.0f);
        __m512h result = blend_v32hf(v32hf_a, v32hf_b, mask32);
        
        // Convert to float for checksum
        __m512 floats = _mm512_cvtph_ps(result);
        checksum += (uint64_t)_mm512_reduce_add_ps(floats);
    }
    #endif
    
    // Test V32BF blend (if supported)
    #ifdef __AVX512BF16__
    {
        __m512bh v32bf_a = _mm512_castsi_ph(_mm512_set1_epi32(0x3F800000)); // 1.0f in bfloat16
        __m512bh v32bf_b = _mm512_castsi_ph(_mm512_set1_epi32(0x40000000)); // 2.0f in bfloat16
        __m512bh result = blend_v32bf(v32bf_a, v32bf_b, mask32);
        
        // Convert to float for checksum
        __m512 floats = _mm512_cvtneobf16_ps(_mm512_castph_si512(result));
        checksum += (uint64_t)_mm512_reduce_add_ps(floats);
    }
    #endif
    
    // Test V16SI blend
    {
        __m512i v16si_a = _mm512_set_epi32(
            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
        );
        __m512i v16si_b = _mm512_set1_epi32(rand());
        __m512i result = blend_v16si(v16si_a, v16si_b, mask16);
        checksum += _mm512_reduce_add_epi32(result);
    }
    
    // Test V8DI blend
    {
        __m512i v8di_a = _mm512_set_epi64(1,2,3,4,5,6,7,8);
        __m512i v8di_b = _mm512_set1_epi64(rand() | ((uint64_t)rand() << 32));
        __m512i result = blend_v8di(v8di_a, v8di_b, mask8);
        checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V8DF blend
    {
        __m512d v8df_a = _mm512_set_pd(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0);
        __m512d v8df_b = _mm512_set1_pd((double)rand() / RAND_MAX * 100.0);
        __m512d result = blend_v8df(v8df_a, v8df_b, mask8);
        checksum += (uint64_t)_mm512_reduce_add_pd(result);
    }
    
    // Test V16SF blend
    {
        __m512 v16sf_a = _mm512_set_ps(
            1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,8.0f,
            9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,16.0f
        );
        __m512 v16sf_b = _mm512_set1_ps((float)rand() / RAND_MAX * 100.0f);
        __m512 result = blend_v16sf(v16sf_a, v16sf_b, mask16);
        checksum += (uint64_t)_mm512_reduce_add_ps(result);
    }
    
    // Execute multi-stage pipelines
    pipeline_char_to_int(&checksum);
    pipeline_float_to_half(&checksum);
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
