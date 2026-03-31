#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile variables to prevent optimization
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
uint64_t pipeline_v64qi_to_v16si(__m512i data1, __m512i data2, int seed);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_v32hf_to_v16sf(__m512h data1, __m512h data2, int seed);

// Implementation of blend functions
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime-derived mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    // Additional computation to create data dependency
    return _mm512_add_epi8(result, _mm512_set1_epi8(1));
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create complex mask based on comparison
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 final_mask = mask ^ cmp_mask;  // XOR to prevent constant folding
    
    __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
    
    v32hi_result = result;
    
    // Convert and blend with different mode
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_mask_blend_epi16(mask, result, shifted);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Generate mask from comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    __mmask32 final_mask = mask & cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(final_mask, a, b);
    
    v32hf_result = result;
    
    // Mixed precision: convert to float and back
    __m512 float_vec = _mm512_cvtph_ps(result);
    __m512h reconverted = _mm512_cvtps_ph(float_vec, _MM_FROUND_CUR_DIRECTION);
    
    return _mm512_mask_blend_ph(mask, result, reconverted);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use same intrinsic as V32HF but with bfloat16 type
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    v32bf_result = result;
    
    // Convert to float and perform operation
    __m512 float_a = _mm512_cvtpbh_ps(a);
    __m512 float_b = _mm512_cvtpbh_ps(b);
    __m512 float_result = _mm512_add_ps(float_a, float_b);
    
    // Convert back and blend again
    __m512bh converted = _mm512_cvtne2ps_pbh(float_result, float_result);
    return _mm512_mask_blend_ph(mask, result, converted);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Create data-dependent mask
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    __mmask16 final_mask = mask | sign_mask;
    
    __m512i result = _mm512_mask_blend_epi32(final_mask, a, b);
    
    v16si_result = result;
    
    // Chain with another operation
    __m512i squared = _mm512_mullo_epi32(result, result);
    return _mm512_mask_blend_epi32(mask, result, squared);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask computation
    __mmask8 eq_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 ne_mask = ~eq_mask;
    __mmask8 final_mask = mask & ne_mask;
    
    __m512i result = _mm512_mask_blend_epi64(final_mask, a, b);
    
    v8di_result = result;
    
    // Mix with floating point version
    __m512d double_vec = _mm512_cvtepi64_pd(result);
    __m512i reconverted = _mm512_cvtpd_epi64(double_vec);
    
    return _mm512_mask_blend_epi64(mask, result, reconverted);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Generate mask from floating comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 final_mask = mask ^ cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(final_mask, a, b);
    
    v8df_result = result;
    
    // Convert to integer and back
    __m512i int_vec = _mm512_cvtpd_epi64(result);
    __m512d reconverted = _mm512_cvtepi64_pd(int_vec);
    
    return _mm512_mask_blend_pd(mask, result, reconverted);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Create complex predicate
    __m512 threshold = _mm512_set1_ps(0.5f);
    __mmask16 gt_mask = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
    __mmask16 lt_mask = _mm512_cmp_ps_mask(b, threshold, _CMP_LT_OQ);
    __mmask16 final_mask = (mask & gt_mask) | lt_mask;
    
    __m512 result = _mm512_mask_blend_ps(final_mask, a, b);
    
    v16sf_result = result;
    
    // Fused multiply-add chain
    __m512 scaled = _mm512_fmadd_ps(result, _mm512_set1_ps(2.0f), _mm512_set1_ps(1.0f));
    return _mm512_mask_blend_ps(mask, result, scaled);
}

__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_v64qi_to_v16si(__m512i data1, __m512i data2, int seed) {
    // Stage 1: V64QI blend
    __mmask64 mask64 = (__mmask64)(seed * 0x5DEECE66DLLU);
    __m512i blended_qi = blend_v64qi(data1, data2, mask64);
    
    // Stage 2: Convert to V32HI and blend
    __m512i hi1 = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_qi, 0));
    __m512i hi2 = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_qi, 1));
    __mmask32 mask32 = (__mmask32)(seed * 0x9E3779B9U);
    __m512i blended_hi = blend_v32hi(hi1, hi2, mask32);
    
    // Stage 3: Convert to V16SI and blend
    __m512i si1 = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(blended_hi, 0));
    __m512i si2 = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(blended_hi, 1));
    __mmask16 mask16 = (__mmask16)(seed * 0x01010101U);
    __m512i blended_si = blend_v16si(si1, si2, mask16);
    
    // Horizontal sum
    return _mm512_reduce_add_epi32(blended_si);
}

__attribute__((target("avx512f,avx512fp16")))
float pipeline_v32hf_to_v16sf(__m512h data1, __m512h data2, int seed) {
    // Stage 1: V32HF blend
    __mmask32 mask32 = (__mmask32)(seed * 0x9E3779B9U);
    __m512h blended_hf = blend_v32hf(data1, data2, mask32);
    
    // Stage 2: Convert to V16SF and blend
    __m512 sf1 = _mm512_cvtph_ps(_mm512_extractf32x8_ps(blended_hf, 0));
    __m512 sf2 = _mm512_cvtph_ps(_mm512_extractf32x8_ps(blended_hf, 1));
    __mmask16 mask16 = (__mmask16)(seed * 0x01010101U);
    __m512 blended_sf = blend_v16sf(sf1, sf2, mask16);
    
    // Horizontal sum
    return _mm512_reduce_add_ps(blended_sf);
}

int main(int argc, char *argv[]) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    uint64_t total_checksum = 0;
    
    // Initialize test data with runtime values
    alignas(64) int8_t qi_data1[64], qi_data2[64];
    alignas(64) int16_t hi_data1[32], hi_data2[32];
    alignas(64) uint16_t hf_data1[32], hf_data2[32];  // Half-precision storage
    alignas(64) uint16_t bf_data1[32], bf_data2[32];  // Bfloat16 storage
    alignas(64) int32_t si_data1[16], si_data2[16];
    alignas(64) int64_t di_data1[8], di_data2[8];
    alignas(64) double df_data1[8], df_data2[8];
    alignas(64) float sf_data1[16], sf_data2[16];
    
    // Fill with pseudo-random data based on seed
    for (int i = 0; i < 64; i++) {
        qi_data1[i] = (int8_t)(rand() % 256 - 128);
        qi_data2[i] = (int8_t)(rand() % 256 - 128);
    }
    for (int i = 0; i < 32; i++) {
        hi_data1[i] = (int16_t)(rand() % 65536 - 32768);
        hi_data2[i] = (int16_t)(rand() % 65536 - 32768);
        hf_data1[i] = rand() % 0x7C00;  // Valid half-precision range
        hf_data2[i] = rand() % 0x7C00;
        bf_data1[i] = rand() % 0x7F80;  // Valid bfloat16 range
        bf_data2[i] = rand() % 0x7F80;
    }
    for (int i = 0; i < 16; i++) {
        si_data1[i] = rand() - RAND_MAX/2;
        si_data2[i] = rand() - RAND_MAX/2;
        sf_data1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        sf_data2[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    }
    for (int i = 0; i < 8; i++) {
        di_data1[i] = ((int64_t)rand() << 32) | rand();
        di_data2[i] = ((int64_t)rand() << 32) | rand();
        df_data1[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
        df_data2[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
    }
    
    // Load vectors
    __m512i v64qi_1 = _mm512_load_si512(qi_data1);
    __m512i v64qi_2 = _mm512_load_si512(qi_data2);
    __m512i v32hi_1 = _mm512_load_si512(hi_data1);
    __m512i v32hi_2 = _mm512_load_si512(hi_data2);
    __m512h v32hf_1 = _mm512_load_ph(hf_data1);
    __m512h v32hf_2 = _mm512_load_ph(hf_data2);
    __m512bh v32bf_1 = _mm512_load_bh(bf_data1);
    __m512bh v32bf_2 = _mm512_load_bh(bf_data2);
    __m512i v16si_1 = _mm512_load_si512(si_data1);
    __m512i v16si_2 = _mm512_load_si512(si_data2);
    __m512i v8di_1 = _mm512_load_si512(di_data1);
    __m512i v8di_2 = _mm512_load_si512(di_data2);
    __m512d v8df_1 = _mm512_load_pd(df_data1);
    __m512d v8df_2 = _mm512_load_pd(df_data2);
    __m512 v16sf_1 = _mm512_load_ps(sf_data1);
    __m512 v16sf_2 = _mm512_load_ps(sf_data2);
    
    // Generate runtime masks (prevents constant folding)
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) mask64 |= (1ULL << i);
    }
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 4 == 0) mask32 |= (1U << i);
    }
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 5 == 0) mask16 |= (1U << i);
    }
    for (int i = 0; i < 8; i++) {
        if ((i + seed) % 6 == 0) mask8 |= (1U << i);
    }
    
    // Execute all blend operations
    __m512i r64qi = blend_v64qi(v64qi_1, v64qi_2, mask64);
    __m512i r32hi = blend_v32hi(v32hi_1, v32hi_2, mask32);
    __m512h r32hf = blend_v32hf(v32hf_1, v32hf_2, mask32);
    __m512bh r32bf = blend_v32bf(v32bf_1, v32bf_2, mask32);
    __m512i r16si = blend_v16si(v16si_1, v16si_2, mask16);
    __m512i r8di = blend_v8di(v8di_1, v8di_2, mask8);
    __m512d r8df = blend_v8df(v8df_1, v8df_2, mask8);
    __m512 r16sf = blend_v16sf(v16sf_1, v16sf_2, mask16);
    
    // Execute pipeline functions
    uint64_t pipe1_result = pipeline_v64qi_to_v16si(v64qi_1, v64qi_2, seed);
    float pipe2_result = pipeline_v32hf_to_v16sf(v32hf_1, v32hf_2, seed);
    
    // Compute checksum from all results
    alignas(64) int8_t qi_result[64];
    alignas(64) int16_t hi_result[32];
    alignas(64) uint16_t hf_result[32];
    alignas(64) uint16_t bf_result[32];
    alignas(64) int32_t si_result[16];
    alignas(64) int64_t di_result[8];
    alignas(64) double df_result[8];
    alignas(64) float sf_result[16];
    
    _mm512_store_si512(qi_result, r64qi);
    _mm512_store_si512(hi_result, r32hi);
    _mm512_store_ph(hf_result, r32hf);
    _mm512_store_bh(bf_result, r32bf);
    _mm512_store_si512(si_result, r16si);
    _mm512_store_si512(di_result, r8di);
    _mm512_store_pd(df_result, r8df);
    _mm512_store_ps(sf_result, r16sf);
    
    for (int i = 0; i < 64; i++) total_checksum += qi_result[i];
    for (int i = 0; i < 32; i++) total_checksum += hi_result[i];
    for (int i = 0; i < 32; i++) total_checksum += hf_result[i];
    for (int i = 0; i < 32; i++) total_checksum += bf_result[i];
    for (int i = 0; i < 16; i++) total_checksum += si_result[i];
    for (int i = 0; i < 8; i++) total_checksum += di_result[i];
    for (int i = 0; i < 8; i++) total_checksum += (uint64_t)(df_result[i] * 1000);
    for (int i = 0; i < 16; i++) total_checksum += (uint64_t)(sf_result[i] * 1000);
    
    total_checksum += pipe1_result;
    total_checksum += (uint64_t)(pipe2_result * 1000);
    
    printf("Final checksum: %lu\n", total_checksum);
    printf("All blend operations executed successfully.\n");
    
    return 0;
}
