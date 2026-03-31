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
uint64_t pipeline_char_to_int(uint8_t* data1, uint8_t* data2, int size);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_float_to_half(float* data1, float* data2, int size);

// V64QI: 64 x 8-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Data-dependent computation: compare and blend
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __m512i blended_cmp = _mm512_mask_blend_epi8(cmp, result, a);
    
    // Store to volatile to prevent elimination
    g_v64qi_result = blended_cmp;
    
    return blended_cmp;
}

// V32HI: 32 x 16-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create conditional mask based on data
    __m512i diff = _mm512_sub_epi16(a, b);
    __mmask32 lt_mask = _mm512_cmplt_epi16_mask(diff, _mm512_setzero_si512());
    
    // Blend using combined mask
    __mmask32 final_mask = mask & lt_mask;
    __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
    
    // Additional computation to prevent folding
    result = _mm512_add_epi16(result, _mm512_set1_epi16(1));
    g_v32hi_result = result;
    
    return result;
}

// V32HF: 32 x half-precision floats (requires AVX512-FP16)
#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Generate mask from comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    __mmask32 blend_mask = mask | cmp_mask;
    
    // Perform the blend
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Mixed precision: convert to float and back
    __m512 temp_f = _mm512_cvtph_ps(result);
    temp_f = _mm512_add_ps(temp_f, _mm512_set1_ps(1.0f));
    result = _mm512_cvtps_ph(temp_f, _MM_FROUND_CUR_DIRECTION);
    
    g_v32hf_result = result;
    return result;
}
#endif

// V32BF: 32 x bfloat16 (requires AVX512-BF16)
#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Convert to float for computation
    __m512 a_f32 = _mm512_cvtneobf16_ps(a);
    __m512 b_f32 = _mm512_cvtneobf16_ps(b);
    
    // Create mask from float comparison
    __mmask16 cmp_mask_f32 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
    
    // Expand to 32-bit mask for bf16 blending
    __mmask32 blend_mask = mask;
    for (int i = 0; i < 16; i++) {
        if (cmp_mask_f32 & (1 << i)) {
            blend_mask |= (3 << (i * 2));  // Set both bf16 elements
        }
    }
    
    // Perform bf16 blend
    __m512bh result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    g_v32bf_result = result;
    return result;
}
#endif

// V16SI: 16 x 32-bit integers
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Complex data-dependent mask generation
    __m512i abs_a = _mm512_abs_epi32(a);
    __m512i abs_b = _mm512_abs_epi32(b);
    __mmask16 abs_cmp = _mm512_cmplt_epi32_mask(abs_a, abs_b);
    
    __mmask16 final_mask = mask ^ abs_cmp;  // XOR for non-trivial combination
    
    __m512i result = _mm512_mask_blend_epi32(final_mask, a, b);
    
    // Additional computation
    result = _mm512_mullo_epi32(result, _mm512_set1_epi32(2));
    g_v16si_result = result;
    
    return result;
}

// V8DI: 8 x 64-bit integers
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Generate mask from population count
    __m512i popcnt_a = _mm512_popcnt_epi64(a);
    __m512i popcnt_b = _mm512_popcnt_epi64(b);
    __mmask8 popcnt_mask = _mm512_cmplt_epi64_mask(popcnt_a, popcnt_b);
    
    __mmask8 final_mask = mask & popcnt_mask;
    
    __m512i result = _mm512_mask_blend_epi64(final_mask, a, b);
    
    // Chain with other operation
    result = _mm512_slli_epi64(result, 1);
    g_v8di_result = result;
    
    return result;
}

// V8DF: 8 x double-precision floats
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Trigonometric-based mask generation
    __m512d sin_a = _mm512_sin_pd(a);
    __m512d sin_b = _mm512_sin_pd(b);
    __mmask8 sin_mask = _mm512_cmp_pd_mask(sin_a, sin_b, _CMP_LT_OQ);
    
    __mmask8 final_mask = mask | sin_mask;
    
    __m512d result = _mm512_mask_blend_pd(final_mask, a, b);
    
    // Mixed computation
    result = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    g_v8df_result = result;
    
    return result;
}

// V16SF: 16 x single-precision floats
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Exponential-based mask
    __m512 exp_a = _mm512_exp_ps(a);
    __m512 exp_b = _mm512_exp_ps(b);
    __mmask16 exp_mask = _mm512_cmp_ps_mask(exp_a, exp_b, _CMP_GT_OQ);
    
    __mmask16 final_mask = mask ^ exp_mask;
    
    __m512 result = _mm512_mask_blend_ps(final_mask, a, b);
    
    // Reciprocal and blend
    __m512 recip = _mm512_rcp14_ps(result);
    result = _mm512_mask_blend_ps(final_mask, result, recip);
    
    g_v16sf_result = result;
    return result;
}

// Multi-stage pipeline: char -> int
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_char_to_int(uint8_t* data1, uint8_t* data2, int size) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < size; i += 64) {
        // Load char data
        __m512i chars1 = _mm512_loadu_si512(data1 + i);
        __m512i chars2 = _mm512_loadu_si512(data2 + i);
        
        // Create runtime-dependent mask
        __mmask64 char_mask = 0;
        for (int j = 0; j < 64; j++) {
            if ((data1[i + j] ^ data2[i + j]) & 0x1) {
                char_mask |= (1ULL << j);
            }
        }
        
        // Blend chars (V64QI)
        __m512i blended_chars = _mm512_mask_blend_epi8(char_mask, chars1, chars2);
        
        // Convert to 32-bit integers (16 x int)
        __m512i ints1 = _mm512_cvtepu8_epi32(_mm512_castsi512_si256(blended_chars));
        __m512i ints2 = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_chars, 1));
        
        // Create int mask from char mask
        __mmask16 int_mask = 0;
        for (int j = 0; j < 16; j++) {
            uint8_t byte_mask = (char_mask >> (j * 4)) & 0xF;
            if (byte_mask == 0xF) {
                int_mask |= (1 << j);
            }
        }
        
        // Blend ints (V16SI)
        __m512i blended_ints = _mm512_mask_blend_epi32(int_mask, ints1, ints2);
        
        // Accumulate checksum
        checksum += _mm512_reduce_add_epi32(blended_ints);
    }
    
    return checksum;
}

// Multi-stage pipeline: float -> half
#ifdef __AVX512FP16__
__attribute__((target("avx512f,avx512fp16")))
float pipeline_float_to_half(float* data1, float* data2, int size) {
    float checksum = 0.0f;
    
    for (int i = 0; i < size; i += 16) {
        // Load float data
        __m512 floats1 = _mm512_loadu_ps(data1 + i);
        __m512 floats2 = _mm512_loadu_ps(data2 + i);
        
        // Create mask from data
        __mmask16 float_mask = _mm512_cmp_ps_mask(floats1, floats2, _CMP_NEQ_UQ);
        
        // Blend floats (V16SF)
        __m512 blended_floats = _mm512_mask_blend_ps(float_mask, floats1, floats2);
        
        // Convert to half precision
        __m512h halves = _mm512_cvtps_ph(blended_floats, _MM_FROUND_CUR_DIRECTION);
        
        // Create another half vector
        __m512h halves2 = _mm512_set1_ph(1.0f);
        
        // Expand float mask to half mask (2:1 ratio)
        __mmask32 half_mask = 0;
        for (int j = 0; j < 16; j++) {
            if (float_mask & (1 << j)) {
                half_mask |= (3 << (j * 2));
            }
        }
        
        // Blend halves (V32HF)
        __m512h blended_halves = _mm512_mask_blend_ph(half_mask, halves, halves2);
        
        // Convert back to float for checksum
        __m512 result_floats = _mm512_cvtph_ps(blended_halves);
        checksum += _mm512_reduce_add_ps(result_floats);
    }
    
    return checksum;
}
#endif

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic behavior
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Initialize test data
    const int DATA_SIZE = 1024;
    
    // Character data
    uint8_t char_data1[DATA_SIZE];
    uint8_t char_data2[DATA_SIZE];
    
    // Integer data
    int32_t int_data1[DATA_SIZE];
    int32_t int_data2[DATA_SIZE];
    
    // Float data
    float float_data1[DATA_SIZE];
    float float_data2[DATA_SIZE];
    
    // Double data
    double double_data1[DATA_SIZE];
    double double_data2[DATA_SIZE];
    
    // Initialize with pseudo-random values
    for (int i = 0; i < DATA_SIZE; i++) {
        char_data1[i] = rand() % 256;
        char_data2[i] = rand() % 256;
        int_data1[i] = rand() - RAND_MAX/2;
        int_data2[i] = rand() - RAND_MAX/2;
        float_data1[i] = (float)rand() / RAND_MAX * 100.0f;
        float_data2[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data1[i] = (double)rand() / RAND_MAX * 100.0;
        double_data2[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    uint64_t total_checksum = 0;
    
    // Test V64QI (64 x char)
    {
        __m512i vec1 = _mm512_loadu_si512(char_data1);
        __m512i vec2 = _mm512_loadu_si512(char_data2);
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            if (char_data1[i] > char_data2[i]) {
                mask |= (1ULL << i);
            }
        }
        __m512i result = blend_v64qi(vec1, vec2, mask);
        total_checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V32HI (32 x short)
    {
        // Convert char to short
        __m512i vec1 = _mm512_cvtepu8_epi16(_mm256_loadu_si256((__m256i*)char_data1));
        __m512i vec2 = _mm512_cvtepu8_epi16(_mm256_loadu_si256((__m256i*)char_data2));
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((char_data1[i] ^ char_data2[i]) & 0x80) {
                mask |= (1U << i);
            }
        }
        __m512i result = blend_v32hi(vec1, vec2, mask);
        total_checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V16SI (16 x int)
    {
        __m512i vec1 = _mm512_loadu_si512(int_data1);
        __m512i vec2 = _mm512_loadu_si512(int_data2);
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            if (int_data1[i] < int_data2[i]) {
                mask |= (1 << i);
            }
        }
        __m512i result = blend_v16si(vec1, vec2, mask);
        total_checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V8DI (8 x long)
    {
        __m512i vec1 = _mm512_loadu_si512((__m512i*)int_data1);
        __m512i vec2 = _mm512_loadu_si512((__m512i*)int_data2);
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            if ((int_data1[i * 2] + int_data2[i * 2]) > 0) {
                mask |= (1 << i);
            }
        }
        __m512i result = blend_v8di(vec1, vec2, mask);
        total_checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V16SF (16 x float)
    {
        __m512 vec1 = _mm512_loadu_ps(float_data1);
        __m512 vec2 = _mm512_loadu_ps(float_data2);
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            if (float_data1[i] != float_data2[i]) {
                mask |= (1 << i);
            }
        }
        __m512 result = blend_v16sf(vec1, vec2, mask);
        total_checksum += (uint64_t)_mm512_reduce_add_ps(result);
    }
    
    // Test V8DF (8 x double)
    {
        __m512d vec1 = _mm512_loadu_pd(double_data1);
        __m512d vec2 = _mm512_loadu_pd(double_data2);
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            if (double_data1[i] > double_data2[i]) {
                mask |= (1 << i);
            }
        }
        __m512d result = blend_v8df(vec1, vec2, mask);
        total_checksum += (uint64_t)_mm512_reduce_add_pd(result);
    }
    
    // Test V32HF (32 x half) if supported
#ifdef __AVX512FP16__
    {
        // Convert float to half
        __m512h vec1 = _mm512_cvtps_ph(_mm512_loadu_ps(float_data1), _MM_FROUND_CUR_DIRECTION);
        __m512h vec2 = _mm512_cvtps_ph(_mm512_loadu_ps(float_data2), _MM_FROUND_CUR_DIRECTION);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            if ((i % 4) == (seed % 4)) {
                mask |= (1U << i);
            }
        }
        __m512h result = blend_v32hf(vec1, vec2, mask);
        // Convert back for checksum
        __m512 result_f = _mm512_cvtph_ps(result);
        total_checksum += (uint64_t)_mm512_reduce_add_ps(result_f);
    }
#endif
    
    // Test V32BF (32 x bfloat16) if supported
#ifdef __AVX512BF16__
    {
        // Convert float to bfloat16
        __m512bh vec1 = _mm512_cvtne2ps_pbh(_mm512_loadu_ps(float_data1), _mm512_loadu_ps(float_data1 + 16));
        __m512bh vec2 = _mm512_cvtne2ps_pbh(_mm512_loadu_ps(float_data2), _mm512_loadu_ps(float_data2 + 16));
        __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
        __m512bh result = blend_v32bf(vec1, vec2, mask);
        // Store to volatile global
        g_v32bf_result = result;
    }
#endif
    
    // Execute multi-stage pipelines
    uint64_t pipeline1_result = pipeline_char_to_int(char_data1, char_data2, DATA_SIZE);
    total_checksum += pipeline1_result;
    
#ifdef __AVX512FP16__
    float pipeline2_result = pipeline_float_to_half(float_data1, float_data2, DATA_SIZE);
    total_checksum += (uint64_t)pipeline2_result;
#endif
    
    printf("Total checksum: %lu\n", total_checksum);
    printf("Seed used: %u\n", seed);
    
    return 0;
}
