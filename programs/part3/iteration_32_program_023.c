#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Volatile globals to prevent optimization
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512h v32hf_result;
volatile __m512bh v32bf_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512d v8df_result;
volatile __m512 v16sf_result;

// Function prototypes with target attributes
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
uint64_t pipeline_char_to_int(uint8_t* data, int len);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_float_to_half(float* data, int len);

// Implementation of blend functions with data-dependent computations
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional computation to prevent folding
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __m512i masked = _mm512_mask_blend_epi8(cmp, result, a);
    
    return masked;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Generate mask from data comparison
    __mmask32 data_mask = _mm512_cmpeq_epi16_mask(a, b);
    __mmask32 combined_mask = mask & data_mask;
    
    // Blend with combined mask
    __m512i result = _mm512_mask_blend_epi16(combined_mask, a, b);
    
    // Conditional operation based on blend result
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_mask_blend_epi16(mask, result, shifted);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Compare and blend half-precision floats
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    __mmask32 blend_mask = mask | cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Additional operation to prevent optimization
    __m512h scaled = _mm512_mul_ph(result, _mm512_set1_ph(2.0f));
    return _mm512_mask_blend_ph(mask, result, scaled);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Blend brain float16 vectors
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert and blend to create data dependency
    __m512i int_vec = _mm512_castps_si512(_mm512_castbh_ps(result));
    __m512i shifted = _mm512_slli_epi32(int_vec, 1);
    __m512bh converted = _mm512_castsi512_bh(shifted);
    
    return _mm512_mask_blend_ph(mask, result, converted);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Generate mask from arithmetic operation
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 diff_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    __mmask16 final_mask = mask ^ diff_mask;
    
    __m512i result = _mm512_mask_blend_epi32(final_mask, a, b);
    
    // Multi-stage blending
    __m512i abs_result = _mm512_abs_epi32(result);
    return _mm512_mask_blend_epi32(mask, result, abs_result);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask generation
    __mmask8 eq_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 lt_mask = _mm512_cmplt_epi64_mask(a, b);
    __mmask8 blend_mask = (mask & eq_mask) | (~mask & lt_mask);
    
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    
    // Arithmetic with blended result
    __m512i squared = _mm512_mullo_epi64(result, result);
    return _mm512_mask_blend_epi64(mask, result, squared);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Data-dependent mask from comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 blend_mask = mask & cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(blend_mask, a, b);
    
    // Mathematical operation on blended result
    __m512d sqrt_val = _mm512_sqrt_pd(_mm512_abs_pd(result));
    return _mm512_mask_blend_pd(mask, result, sqrt_val);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Generate mask from multiple conditions
    __mmask16 lt_mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    __mmask16 gt_mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    __mmask16 blend_mask = (mask & lt_mask) | (~mask & gt_mask);
    
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    
    // Trigonometric operation
    __m512 sin_val = _mm512_sin_ps(result);
    return _mm512_mask_blend_ps(mask, result, sin_val);
}

// Multi-stage pipeline: char -> packed -> int blending
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_char_to_int(uint8_t* data, int len) {
    uint64_t checksum = 0;
    
    // Process in chunks of 64 bytes
    for (int i = 0; i + 64 <= len; i += 64) {
        // Load 64 bytes
        __m512i chars = _mm512_loadu_si512(data + i);
        
        // Create pattern mask (alternating pattern)
        __mmask64 char_mask = 0;
        for (int j = 0; j < 64; j++) {
            char_mask |= ((data[i + j] & 1) << j);
        }
        
        // Blend V64QI
        __m512i alt_chars = _mm512_set1_epi8(0xAA);
        __m512i blended_chars = _mm512_mask_blend_epi8(char_mask, chars, alt_chars);
        
        // Pack bytes into 32 shorts
        __m512i packed = _mm512_maddubs_epi16(blended_chars, _mm512_set1_epi8(1));
        
        // Blend V32HI
        __mmask32 short_mask = (__mmask32)char_mask; // Use lower 32 bits
        __m512i alt_shorts = _mm512_set1_epi16(0x5555);
        __m512i blended_shorts = _mm512_mask_blend_epi16(short_mask, packed, alt_shorts);
        
        // Pack shorts into 16 ints
        __m512i ints = _mm512_madd_epi16(blended_shorts, _mm512_set1_epi16(1));
        
        // Blend V16SI
        __mmask16 int_mask = (__mmask16)(char_mask & 0xFFFF);
        __m512i alt_ints = _mm512_set1_epi32(0x33333333);
        __m512i blended_ints = _mm512_mask_blend_epi32(int_mask, ints, alt_ints);
        
        // Horizontal add and accumulate
        __m256i sum256 = _mm512_extracti64x4_epi64(blended_ints, 0);
        sum256 = _mm256_add_epi32(sum256, _mm512_extracti64x4_epi64(blended_ints, 1));
        __m128i sum128 = _mm_add_epi32(_mm256_extracti128_si256(sum256, 0),
                                      _mm256_extracti128_si256(sum256, 1));
        sum128 = _mm_add_epi32(sum128, _mm_srli_si128(sum128, 8));
        sum128 = _mm_add_epi32(sum128, _mm_srli_si128(sum128, 4));
        
        checksum += _mm_cvtsi128_si32(sum128);
    }
    
    return checksum;
}

// Mixed precision pipeline
__attribute__((target("avx512f,avx512fp16")))
float pipeline_float_to_half(float* data, int len) {
    float sum = 0.0f;
    
    // Process in chunks of 16 floats
    for (int i = 0; i + 16 <= len; i += 16) {
        // Load floats
        __m512 floats = _mm512_loadu_ps(data + i);
        
        // Generate mask from data
        __mmask16 float_mask = 0;
        for (int j = 0; j < 16; j++) {
            float_mask |= ((data[i + j] > 0.0f) << j);
        }
        
        // Blend V16SF
        __m512 alt_floats = _mm512_set1_ps(-1.0f);
        __m512 blended_floats = _mm512_mask_blend_ps(float_mask, floats, alt_floats);
        
        // Convert to half precision
        __m512h halves = _mm512_cvtps_ph(blended_floats, _MM_FROUND_TO_NEAREST_INT);
        
        // Blend V32HF
        __mmask32 half_mask = float_mask | (float_mask << 16);
        __m512h alt_halves = _mm512_set1_ph(0.5f);
        __m512h blended_halves = _mm512_mask_blend_ph(half_mask, halves, alt_halves);
        
        // Convert back to float and accumulate
        __m512 reconverted = _mm512_cvtph_ps(blended_halves);
        
        // Horizontal sum
        sum += _mm512_reduce_add_ps(reconverted);
    }
    
    return sum;
}

int main(int argc, char** argv) {
    // Use argc for pseudo-random seed
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Initialize test data
    const int DATA_SIZE = 1024;
    
    // Allocate and initialize arrays
    uint8_t* char_data = (uint8_t*)aligned_alloc(64, DATA_SIZE);
    uint16_t* short_data = (uint16_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint16_t));
    uint32_t* int_data = (uint32_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint32_t));
    uint64_t* long_data = (uint64_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint64_t));
    float* float_data = (float*)aligned_alloc(64, DATA_SIZE * sizeof(float));
    double* double_data = (double*)aligned_alloc(64, DATA_SIZE * sizeof(double));
    uint16_t* half_data = (uint16_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint16_t));
    uint16_t* bf16_data = (uint16_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint16_t));
    
    // Fill with pseudo-random values
    for (int i = 0; i < DATA_SIZE; i++) {
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 65536;
        int_data[i] = rand();
        long_data[i] = ((uint64_t)rand() << 32) | rand();
        float_data[i] = (float)rand() / RAND_MAX * 100.0f - 50.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
        half_data[i] = rand() % 65536;
        bf16_data[i] = rand() % 65536;
    }
    
    uint64_t checksum = 0;
    
    // Test V64QI blend
    {
        __m512i a = _mm512_loadu_si512(char_data);
        __m512i b = _mm512_set1_epi8(0xFF);
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            mask |= ((char_data[i] & 1) << i);
        }
        __m512i result = blend_v64qi(a, b, mask);
        v64qi_result = result; // Volatile store
        
        // Accumulate checksum
        uint8_t* r = (uint8_t*)&result;
        for (int i = 0; i < 64; i++) {
            checksum += r[i];
        }
    }
    
    // Test V32HI blend
    {
        __m512i a = _mm512_loadu_si512(short_data);
        __m512i b = _mm512_set1_epi16(0x8000);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((short_data[i] & 0x8000) ? (1 << i) : 0);
        }
        __m512i result = blend_v32hi(a, b, mask);
        v32hi_result = result;
        
        uint16_t* r = (uint16_t*)&result;
        for (int i = 0; i < 32; i++) {
            checksum += r[i];
        }
    }
    
    // Test V32HF blend (requires -mavx512fp16)
    #ifdef __AVX512FP16__
    {
        __m512h a = _mm512_loadu_ph(half_data);
        __m512h b = _mm512_set1_ph(1.0f);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((half_data[i] & 0x8000) ? (1 << i) : 0);
        }
        __m512h result = blend_v32hf(a, b, mask);
        v32hf_result = result;
        
        uint16_t* r = (uint16_t*)&result;
        for (int i = 0; i < 32; i++) {
            checksum += r[i];
        }
    }
    #endif
    
    // Test V32BF blend (requires -mavx512bf16)
    #ifdef __AVX512BF16__
    {
        __m512bh a = _mm512_loadu_bh(bf16_data);
        __m512bh b = _mm512_set1_bh(0.5f);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((bf16_data[i] & 0x4000) ? (1 << i) : 0);
        }
        __m512bh result = blend_v32bf(a, b, mask);
        v32bf_result = result;
        
        uint16_t* r = (uint16_t*)&result;
        for (int i = 0; i < 32; i++) {
            checksum += r[i];
        }
    }
    #endif
    
    // Test V16SI blend
    {
        __m512i a = _mm512_loadu_si512(int_data);
        __m512i b = _mm512_set1_epi32(0xFFFFFFFF);
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            mask |= ((int_data[i] & 0x80000000) ? (1 << i) : 0);
        }
        __m512i result = blend_v16si(a, b, mask);
        v16si_result = result;
        
        uint32_t* r = (uint32_t*)&result;
        for (int i = 0; i < 16; i++) {
            checksum += r[i];
        }
    }
    
    // Test V8DI blend
    {
        __m512i a = _mm512_loadu_si512(long_data);
        __m512i b = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFF);
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            mask |= ((long_data[i] & 0x8000000000000000) ? (1 << i) : 0);
        }
        __m512i result = blend_v8di(a, b, mask);
        v8di_result = result;
        
        uint64_t* r = (uint64_t*)&result;
        for (int i = 0; i < 8; i++) {
            checksum += r[i];
        }
    }
    
    // Test V8DF blend
    {
        __m512d a = _mm512_loadu_pd(double_data);
        __m512d b = _mm512_set1_pd(3.141592653589793);
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            mask |= ((double_data[i] > 0.0) ? (1 << i) : 0);
        }
        __m512d result = blend_v8df(a, b, mask);
        v8df_result = result;
        
        double* r = (double*)&result;
        for (int i = 0; i < 8; i++) {
            checksum += (uint64_t)(r[i] * 1000.0);
        }
    }
    
    // Test V16SF blend
    {
        __m512 a = _mm512_loadu_ps(float_data);
        __m512 b = _mm512_set1_ps(2.718281828459045f);
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            mask |= ((float_data[i] > 0.0f) ? (1 << i) : 0);
        }
        __m512 result = blend_v16sf(a, b, mask);
        v16sf_result = result;
        
        float* r = (float*)&result;
        for (int i = 0; i < 16; i++) {
            checksum += (uint32_t)(r[i] * 1000.0f);
        }
    }
    
    // Execute multi-stage pipelines
    checksum += pipeline_char_to_int(char_data, DATA_SIZE);
    
    #ifdef __AVX512FP16__
    float fp_sum = pipeline_float_to_half(float_data, DATA_SIZE);
    checksum += (uint64_t)(fp_sum * 1000.0f);
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    
    // Cleanup
    free(char_data);
    free(short_data);
    free(int_data);
    free(long_data);
    free(float_data);
    free(double_data);
    free(half_data);
    free(bf16_data);
    
    return 0;
}
