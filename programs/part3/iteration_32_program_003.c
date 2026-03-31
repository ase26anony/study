#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
void pipeline_blend_operations(int seed);

__attribute__((target("avx512f,avx512fp16,avx512bf16")))
void mixed_precision_blend(int seed);

// Implementation of blend functions
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Data-dependent computation: mix with index-based pattern
    __m512i indices = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Additional operation to prevent constant folding
    result = _mm512_add_epi8(result, _mm512_and_si512(indices, _mm512_set1_epi8(1)));
    
    // Store to volatile to force materialization
    v64qi_result = result;
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create data-dependent mask based on vector comparison
    __m512i cmp = _mm512_cmpgt_epi16(a, b);
    __mmask32 dynamic_mask = _mm512_cmp_epi16_mask(cmp, _mm512_setzero_si512(), _MM_CMPINT_EQ);
    
    // Combine with input mask
    __mmask32 final_mask = mask ^ dynamic_mask;
    
    __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
    
    // Additional computation
    result = _mm512_slli_epi16(result, 1);
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Half-precision blend with data-dependent mask
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    __mmask32 final_mask = mask | cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(final_mask, a, b);
    
    // Additional FP operation
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // BF16 blend - use same intrinsic as half-precision
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store to volatile
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Create dynamic mask from arithmetic
    __m512i sum = _mm512_add_epi32(a, b);
    __mmask16 dynamic_mask = _mm512_cmplt_epi32_mask(sum, _mm512_set1_epi32(1000));
    
    __m512i result = _mm512_mask_blend_epi32(mask ^ dynamic_mask, a, b);
    
    // Additional computation
    result = _mm512_mullo_epi32(result, _mm512_set1_epi32(2));
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // 64-bit integer blend with data-dependent mask
    __mmask8 cmp_mask = _mm512_cmp_epi64_mask(a, b, _MM_CMPINT_GT);
    __mmask8 final_mask = mask & cmp_mask;
    
    __m512i result = _mm512_mask_blend_epi64(final_mask, a, b);
    
    // Additional operation
    result = _mm512_srli_epi64(result, 1);
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Double-precision blend
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    __mmask8 final_mask = mask | cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(final_mask, a, b);
    
    // Additional computation
    result = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Single-precision blend with complex mask generation
    __m512 abs_a = _mm512_abs_ps(a);
    __m512 abs_b = _mm512_abs_ps(b);
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(abs_a, abs_b, _CMP_LT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask ^ cmp_mask, a, b);
    
    // Additional operation
    result = _mm512_sqrt_ps(_mm512_add_ps(result, _mm512_set1_ps(1.0f)));
    
    v16sf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512f")))
void pipeline_blend_operations(int seed) {
    // Multi-stage pipeline: V64QI -> V32HI -> V16SI -> V8DI
    
    // Stage 1: V64QI blend
    int8_t data64qi_a[64], data64qi_b[64];
    for (int i = 0; i < 64; i++) {
        data64qi_a[i] = (int8_t)((i + seed) % 128);
        data64qi_b[i] = (int8_t)((i * 3 + seed) % 128);
    }
    
    __m512i v64qi_a = _mm512_loadu_si512(data64qi_a);
    __m512i v64qi_b = _mm512_loadu_si512(data64qi_b);
    __mmask64 mask64 = (__mmask64)(seed & 0xFFFFFFFFFFFFFFFFULL);
    
    __m512i blended_64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Stage 2: Convert to V32HI and blend
    __m512i v32hi_a = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_64qi, 0));
    __m512i v32hi_b = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_64qi, 1));
    __mmask32 mask32 = (__mmask32)(seed & 0xFFFFFFFF);
    
    __m512i blended_32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Stage 3: Convert to V16SI and blend
    __m512i v16si_a = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(blended_32hi, 0));
    __m512i v16si_b = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(blended_32hi, 1));
    __mmask16 mask16 = (__mmask16)(seed & 0xFFFF);
    
    __m512i blended_16si = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Stage 4: Convert to V8DI and blend
    __m512i v8di_a = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(blended_16si, 0));
    __m512i v8di_b = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(blended_16si, 1));
    __mmask8 mask8 = (__mmask8)(seed & 0xFF);
    
    blend_v8di(v8di_a, v8di_b, mask8);
}

__attribute__((target("avx512f,avx512fp16,avx512bf16")))
void mixed_precision_blend(int seed) {
    // Mixed precision: V16SF -> V8DF -> V32HF -> V32BF
    
    // Stage 1: V16SF blend
    float data16sf_a[16], data16sf_b[16];
    for (int i = 0; i < 16; i++) {
        data16sf_a[i] = (float)((i + seed) % 100) / 10.0f;
        data16sf_b[i] = (float)((i * 2 + seed) % 100) / 10.0f;
    }
    
    __m512 v16sf_a = _mm512_loadu_ps(data16sf_a);
    __m512 v16sf_b = _mm512_loadu_ps(data16sf_b);
    __mmask16 mask16 = (__mmask16)(seed & 0xFFFF);
    
    __m512 blended_16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Stage 2: Convert to V8DF and blend
    __m512d v8df_a = _mm512_cvtps_pd(_mm512_extractf32x8_ps(blended_16sf, 0));
    __m512d v8df_b = _mm512_cvtps_pd(_mm512_extractf32x8_ps(blended_16sf, 1));
    __mmask8 mask8 = (__mmask8)(seed & 0xFF);
    
    __m512d blended_8df = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Stage 3: Convert back to V32HF and blend
    // Note: This requires AVX512-FP16
    __m512h v32hf_a = _mm512_cvtps_ph(blended_16sf, _MM_FROUND_TO_NEAREST_INT);
    __m512h v32hf_b = _mm512_cvtps_ph(_mm512_set1_ps(5.0f), _MM_FROUND_TO_NEAREST_INT);
    __mmask32 mask32 = (__mmask32)(seed & 0xFFFFFFFF);
    
    blend_v32hf(v32hf_a, v32hf_b, mask32);
    
    // Stage 4: V32BF blend (using bfloat16)
    // Load some data as bfloat16
    uint16_t bf_data[32];
    for (int i = 0; i < 32; i++) {
        bf_data[i] = (uint16_t)((i + seed) % 65535);
    }
    
    __m512bh v32bf_a = _mm512_loadu_si512(bf_data);
    __m512bh v32bf_b = _mm512_set1_epi16(0x3F80); // bfloat16 1.0
    
    blend_v32bf(v32bf_a, v32bf_b, mask32);
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics with seed: %d\n", seed);
    
    // Initialize test data arrays
    int8_t char_data[64];
    int16_t short_data[32];
    int32_t int_data[16];
    int64_t long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t half_data[32];
    uint16_t bfloat_data[32];
    
    for (int i = 0; i < 64; i++) char_data[i] = (int8_t)((i * 3 + seed) % 128);
    for (int i = 0; i < 32; i++) short_data[i] = (int16_t)((i * 5 + seed) % 32768);
    for (int i = 0; i < 16; i++) int_data[i] = (int32_t)((i * 7 + seed) % 65536);
    for (int i = 0; i < 8; i++) long_data[i] = (int64_t)((i * 11 + seed) % 65536);
    for (int i = 0; i < 16; i++) float_data[i] = (float)((i * 13 + seed) % 100) / 10.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)((i * 17 + seed) % 100) / 10.0;
    for (int i = 0; i < 32; i++) half_data[i] = (uint16_t)((i * 19 + seed) % 65535);
    for (int i = 0; i < 32; i++) bfloat_data[i] = (uint16_t)((i * 23 + seed) % 65535);
    
    // Call each blend function with runtime-derived masks
    __m512i v64qi_a = _mm512_loadu_si512(char_data);
    __m512i v64qi_b = _mm512_set1_epi8(64);
    __mmask64 mask64 = (__mmask64)(seed & 0xFFFFFFFFFFFFFFFFULL);
    blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    __m512i v32hi_a = _mm512_loadu_si512(short_data);
    __m512i v32hi_b = _mm512_set1_epi16(100);
    __mmask32 mask32 = (__mmask32)(seed & 0xFFFFFFFF);
    blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_set1_epi32(1000);
    __mmask16 mask16 = (__mmask16)(seed & 0xFFFF);
    blend_v16si(v16si_a, v16si_b, mask16);
    
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_set1_epi64(10000);
    __mmask8 mask8 = (__mmask8)(seed & 0xFF);
    blend_v8di(v8di_a, v8di_b, mask8);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_set1_ps(10.0f);
    blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_set1_pd(10.0);
    blend_v8df(v8df_a, v8df_b, mask8);
    
    // Execute pipeline operations
    pipeline_blend_operations(seed);
    mixed_precision_blend(seed);
    
    printf("All blend operations completed.\n");
    
    // Compute checksum from volatile results
    uint64_t checksum = 0;
    uint8_t* ptr;
    
    ptr = (uint8_t*)&v64qi_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v32hi_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v16si_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v8di_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    printf("Checksum: %lu\n", checksum);
    
    return 0;
}
