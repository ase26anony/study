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

// V64QI: 64x 8-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional data-dependent computation
    result = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    return result;
}

// V32HI: 32x 16-bit integers
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Complex mask computation to prevent folding
    __mmask32 dynamic_mask = mask ^ 0xAAAAAAAA; // XOR with pattern
    
    __m512i result = _mm512_mask_blend_epi16(dynamic_mask, a, b);
    
    // Feed into another operation
    result = _mm512_mullo_epi16(result, _mm512_set1_epi16(2));
    
    v32hi_result = result;
    return result;
}

// V32HF: 32x half-precision floats (requires AVX512-FP16)
#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Use computed mask
    __mmask32 inv_mask = ~mask;
    
    __m512h result = _mm512_mask_blend_ph(inv_mask, a, b);
    
    // Mixed precision operation
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    
    v32hf_result = result;
    return result;
}
#endif

// V32BF: 32x bfloat16 (requires AVX512-BF16)
#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Alternate mask pattern
    __mmask32 alt_mask = mask | 0x55555555;
    
    __m512bh result = _mm512_mask_blend_ph(alt_mask, a, b);
    
    v32bf_result = result;
    return result;
}
#endif

// V16SI: 16x 32-bit integers
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Create mask from comparison
    __m512i cmp = _mm512_cmpgt_epi32(a, b);
    __mmask16 cmp_mask = _mm512_cmp_epi32_mask(cmp, _mm512_setzero_si512(), _MM_CMPINT_EQ);
    
    __m512i result = _mm512_mask_blend_epi32(mask & cmp_mask, a, b);
    
    // Multi-stage: convert to float and back
    __m512 float_vec = _mm512_cvtepi32_ps(result);
    result = _mm512_cvtps_epi32(float_vec);
    
    v16si_result = result;
    return result;
}

// V8DI: 8x 64-bit integers
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask derivation
    __mmask8 dynamic_mask = (mask * 3) & 0xFF;
    
    __m512i result = _mm512_mask_blend_epi64(dynamic_mask, a, b);
    
    // Horizontal add pattern
    result = _mm512_add_epi64(result, _mm512_srli_epi64(result, 32));
    
    v8di_result = result;
    return result;
}

// V8DF: 8x double-precision floats
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Mask from floating comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask | cmp_mask, a, b);
    
    // Mathematical operation
    result = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    
    v8df_result = result;
    return result;
}

// V16SF: 16x single-precision floats
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Reciprocal and blend
    __m512 recip_a = _mm512_rcp14_ps(a);
    __m512 recip_b = _mm512_rcp14_ps(b);
    
    __m512 result = _mm512_mask_blend_ps(mask, recip_a, recip_b);
    
    // Fused multiply-add
    result = _mm512_fmadd_ps(result, a, b);
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline: V64QI -> V16SI conversion chain
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(uint8_t* data, int size) {
    __m512i v64qi_acc = _mm512_setzero_si512();
    __m512i v16si_acc = _mm512_setzero_si512();
    
    for (int i = 0; i < size; i += 64) {
        // Load V64QI data
        __m512i v64qi_data = _mm512_loadu_si512(data + i);
        __m512i v64qi_pattern = _mm512_set1_epi8(i & 0xFF);
        
        // Create runtime mask
        __mmask64 mask64 = 0;
        for (int j = 0; j < 64; j++) {
            mask64 |= ((data[i + j] > 128) ? 1ULL : 0ULL) << j;
        }
        
        // Blend V64QI
        __m512i blended_qi = blend_v64qi(v64qi_data, v64qi_pattern, mask64);
        
        // Convert to V16SI (pack bytes to ints)
        __m512i v16si_low = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_qi, 0));
        __m512i v16si_high = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_qi, 1));
        
        // Create mask for V16SI blend
        __mmask16 mask16 = (i / 64) & 0xFFFF;
        
        // Blend V16SI
        __m512i blended_si = blend_v16si(v16si_low, v16si_high, mask16);
        
        v16si_acc = _mm512_add_epi32(v16si_acc, blended_si);
    }
    
    // Horizontal sum
    int64_t sum = 0;
    int32_t* ptr = (int32_t*)&v16si_acc;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic data
    unsigned int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    // Initialize test arrays
    uint8_t data_8bit[1024];
    uint16_t data_16bit[512];
    uint32_t data_32bit[256];
    uint64_t data_64bit[128];
    float data_float[256];
    double data_double[128];
    
    for (int i = 0; i < 1024; i++) data_8bit[i] = rand() % 256;
    for (int i = 0; i < 512; i++) data_16bit[i] = rand() % 65536;
    for (int i = 0; i < 256; i++) data_32bit[i] = rand();
    for (int i = 0; i < 128; i++) data_64bit[i] = ((uint64_t)rand() << 32) | rand();
    for (int i = 0; i < 256; i++) data_float[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 128; i++) data_double[i] = (double)rand() / RAND_MAX;
    
    int64_t checksum = 0;
    
    // Test V64QI blend
    {
        __m512i a = _mm512_loadu_si512(data_8bit);
        __m512i b = _mm512_loadu_si512(data_8bit + 64);
        __mmask64 mask = 0;
        for (int i = 0; i < 64; i++) {
            mask |= ((i % 3 == 0) ? 1ULL : 0ULL) << i;
        }
        __m512i result = blend_v64qi(a, b, mask);
        checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V32HI blend
    {
        __m512i a = _mm512_loadu_si512(data_16bit);
        __m512i b = _mm512_loadu_si512(data_16bit + 32);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((data_16bit[i] > 32768) ? 1U : 0U) << i;
        }
        __m512i result = blend_v32hi(a, b, mask);
        checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V16SI blend
    {
        __m512i a = _mm512_loadu_si512(data_32bit);
        __m512i b = _mm512_loadu_si512(data_32bit + 16);
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            mask |= ((i % 2) ? 1U : 0U) << i;
        }
        __m512i result = blend_v16si(a, b, mask);
        checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V8DI blend
    {
        __m512i a = _mm512_loadu_si512(data_64bit);
        __m512i b = _mm512_loadu_si512(data_64bit + 8);
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            mask |= ((data_64bit[i] % 2) ? 1U : 0U) << i;
        }
        __m512i result = blend_v8di(a, b, mask);
        checksum += _mm512_reduce_add_epi64(result);
    }
    
    // Test V16SF blend
    {
        __m512 a = _mm512_loadu_ps(data_float);
        __m512 b = _mm512_loadu_ps(data_float + 16);
        __mmask16 mask = 0;
        for (int i = 0; i < 16; i++) {
            mask |= ((data_float[i] > 0.5f) ? 1U : 0U) << i;
        }
        __m512 result = blend_v16sf(a, b, mask);
        // Convert to integer for checksum
        __m512i int_result = _mm512_cvtps_epi32(result);
        checksum += _mm512_reduce_add_epi64(int_result);
    }
    
    // Test V8DF blend
    {
        __m512d a = _mm512_loadu_pd(data_double);
        __m512d b = _mm512_loadu_pd(data_double + 8);
        __mmask8 mask = 0;
        for (int i = 0; i < 8; i++) {
            mask |= ((data_double[i] > 0.5) ? 1U : 0U) << i;
        }
        __m512d result = blend_v8df(a, b, mask);
        // Convert to integer for checksum
        __m512i int_result = _mm512_cvtpd_epi64(result);
        checksum += _mm512_reduce_add_epi64(int_result);
    }
    
    // Test V32HF blend (if available)
#ifdef __AVX512FP16__
    {
        _Float16 data_half[512];
        for (int i = 0; i < 512; i++) {
            data_half[i] = (_Float16)((float)rand() / RAND_MAX);
        }
        
        __m512h a = _mm512_loadu_ph(data_half);
        __m512h b = _mm512_loadu_ph(data_half + 32);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((i % 4 == 0) ? 1U : 0U) << i;
        }
        __m512h result = blend_v32hf(a, b, mask);
        // Store to volatile array
        _mm512_storeu_ph((_Float16*)data_half, result);
        checksum += (int64_t)data_half[0];
    }
#endif
    
    // Test V32BF blend (if available)
#ifdef __AVX512BF16__
    {
        __bfloat16 data_bf16[512];
        for (int i = 0; i < 512; i++) {
            uint16_t val = rand() % 65536;
            memcpy(&data_bf16[i], &val, sizeof(__bfloat16));
        }
        
        __m512bh a = _mm512_loadu_ph(data_bf16);
        __m512bh b = _mm512_loadu_ph(data_bf16 + 32);
        __mmask32 mask = 0;
        for (int i = 0; i < 32; i++) {
            mask |= ((i % 5 == 0) ? 1U : 0U) << i;
        }
        __m512bh result = blend_v32bf(a, b, mask);
        // Store to volatile array
        _mm512_storeu_ph(data_bf16, result);
        checksum += (int64_t)data_bf16[0];
    }
#endif
    
    // Execute multi-stage pipeline
    checksum += pipeline_v64qi_to_v16si(data_8bit, 512);
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
