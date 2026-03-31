#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    // Create mask based on comparison result
    __mmask64 mask = _mm512_cmpgt_epi8_mask(a, b);
    // Mix with runtime value to prevent constant folding
    volatile int seed = rand();
    mask ^= (__mmask64)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    __mmask32 mask = _mm512_cmpgt_epi16_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return mask;
}

__attribute__((target("avx512bw,avx512fp16")))
__mmask32 compute_v32hf_mask(__m512h a, __m512h b) {
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFFFFFF);
    return mask;
}

__attribute__((target("avx512bw,avx512bf16")))
__mmask32 compute_v32bf_mask(__m512bh a, __m512bh b) {
    // For bf16, we need to convert to float for comparison
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
    __mmask32 mask = _mm512_kunpackd(mask16, mask16);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFFFFFF);
    return mask;
}

__attribute__((target("avx512f")))
__mmask16 compute_v16si_mask(__m512i a, __m512i b) {
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask16)(seed & 0xFFFF);
    return mask;
}

__attribute__((target("avx512f")))
__mmask8 compute_v8di_mask(__m512i a, __m512i b) {
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask8)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512f")))
__mmask8 compute_v8df_mask(__m512d a, __m512d b) {
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    volatile int seed = rand();
    mask ^= (__mmask8)(seed & 0xFF);
    return mask;
}

__attribute__((target("avx512f")))
__mmask16 compute_v16sf_mask(__m512 a, __m512 b) {
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    volatile int seed = rand();
    mask ^= (__mmask16)(seed & 0xFFFF);
    return mask;
}

// Blend implementation functions
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion for V64QImode
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Multi-stage pipeline: use result in another operation
    __m512i shifted = _mm512_slli_epi16(result, 2);
    __mmask64 new_mask = compute_v64qi_mask(shifted, a);
    result = _mm512_mask_blend_epi8(new_mask, result, shifted);
    
    // Store to volatile to prevent elimination
    g_v64qi_result = result;
    
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Data-dependent computation with conversion
    __m512i converted = _mm512_mullo_epi16(result, _mm512_set1_epi16(2));
    __mmask32 new_mask = compute_v32hi_mask(converted, a);
    result = _mm512_mask_blend_epi16(new_mask, result, converted);
    
    g_v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision: convert to float and back
    __m512 temp_f32 = _mm512_cvtph_ps(result);
    __m512 scaled = _mm512_mul_ps(temp_f32, _mm512_set1_ps(1.5f));
    __m512h scaled_hf = _mm512_cvtps_ph(scaled, _MM_FROUND_CUR_DIRECTION);
    __mmask32 new_mask = compute_v32hf_mask(scaled_hf, a);
    result = _mm512_mask_blend_ph(new_mask, result, scaled_hf);
    
    g_v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float for computation
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    __m512 result_f32 = _mm512_cvtpbh_ps(result);
    
    // Blend in float domain and convert back
    __m512 blended_f32 = _mm512_add_ps(result_f32, _mm512_mul_ps(a_f32, b_f32));
    __m512bh blended_bf = _mm512_cvtne2ps_pbh(blended_f32, blended_f32);
    __mmask32 new_mask = compute_v32bf_mask(blended_bf, result);
    result = _mm512_mask_blend_ph(new_mask, result, blended_bf);
    
    g_v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Multi-stage from V64QI: pack bytes into ints
    __m512i byte_data = g_v64qi_result;
    __m512i packed = _mm512_maddubs_epi16(byte_data, _mm512_set1_epi8(1));
    __m512i ints = _mm512_madd_epi16(packed, _mm512_set1_epi16(1));
    __mmask16 new_mask = compute_v16si_mask(ints, result);
    result = _mm512_mask_blend_epi32(new_mask, result, ints);
    
    g_v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Convert from V16SI
    __m512i int_data = g_v16si_result;
    __m512i extended = _mm512_cvtepi32_epi64(_mm512_extracti32x8_epi32(int_data, 0));
    __mmask8 new_mask = compute_v8di_mask(extended, result);
    result = _mm512_mask_blend_epi64(new_mask, result, extended);
    
    g_v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Convert from integer to double
    __m512i long_data = g_v8di_result;
    __m512d converted = _mm512_cvtepi64_pd(long_data);
    __mmask8 new_mask = compute_v8df_mask(converted, result);
    result = _mm512_mask_blend_pd(new_mask, result, converted);
    
    g_v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Convert from double to float
    __m512d double_data = g_v8df_result;
    __m512 converted_lo = _mm512_cvtpd_ps(_mm512_extractf64x8_pd(double_data, 0));
    __m512 converted_hi = _mm512_cvtpd_ps(_mm512_extractf64x8_pd(double_data, 1));
    __m512 converted = _mm512_shuffle_f32x4(converted_lo, converted_hi, 0x44);
    __mmask16 new_mask = compute_v16sf_mask(converted, result);
    result = _mm512_mask_blend_ps(new_mask, result, converted);
    
    g_v16sf_result = result;
    return result;
}

// Accumulation function for checksum
__attribute__((target("avx512f")))
double compute_checksum() {
    double checksum = 0.0;
    
    // Horizontal add for V64QI
    __m512i v64qi = g_v64qi_result;
    checksum += _mm512_reduce_add_epi64(_mm512_cvtepi8_epi64(_mm512_extracti64x8_epi64(v64qi, 0)));
    
    // Horizontal add for V32HI
    __m512i v32hi = g_v32hi_result;
    checksum += _mm512_reduce_add_epi64(_mm512_cvtepi16_epi64(_mm512_extracti32x8_epi32(v32hi, 0)));
    
    // Horizontal add for V16SI
    __m512i v16si = g_v16si_result;
    checksum += _mm512_reduce_add_epi64(v16si);
    
    // Horizontal add for V8DI
    __m512i v8di = g_v8di_result;
    checksum += _mm512_reduce_add_epi64(v8di);
    
    // Horizontal add for V8DF
    __m512d v8df = g_v8df_result;
    checksum += _mm512_reduce_add_pd(v8df);
    
    // Horizontal add for V16SF
    __m512 v16sf = g_v16sf_result;
    checksum += _mm512_reduce_add_ps(v16sf);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic behavior
    srand(argc);
    
    // Initialize test data with runtime values
    uint8_t byte_data[64];
    uint16_t short_data[32];
    int32_t int_data[16];
    int64_t long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t half_data[32];
    uint16_t bfloat_data[32];
    
    for (int i = 0; i < 64; i++) byte_data[i] = (uint8_t)(rand() % 256);
    for (int i = 0; i < 32; i++) short_data[i] = (uint16_t)(rand() % 65536);
    for (int i = 0; i < 16; i++) int_data[i] = rand();
    for (int i = 0; i < 8; i++) long_data[i] = ((int64_t)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 32; i++) half_data[i] = (uint16_t)(rand() % 65536);
    for (int i = 0; i < 32; i++) bfloat_data[i] = (uint16_t)(rand() % 65536);
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)byte_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(byte_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 16));
    
    __m512h v32hf_a = _mm512_loadu_ph(half_data);
    __m512h v32hf_b = _mm512_loadu_ph(half_data + 16);
    
    __m512bh v32bf_a = _mm512_loadu_bh(bfloat_data);
    __m512bh v32bf_b = _mm512_loadu_bh(bfloat_data + 16);
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 4));
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    // Compute masks using runtime data
    __mmask64 mask64 = compute_v64qi_mask(v64qi_a, v64qi_b);
    __mmask32 mask32_hi = compute_v32hi_mask(v32hi_a, v32hi_b);
    __mmask32 mask32_hf = compute_v32hf_mask(v32hf_a, v32hf_b);
    __mmask32 mask32_bf = compute_v32bf_mask(v32bf_a, v32bf_b);
    __mmask16 mask16_si = compute_v16si_mask(v16si_a, v16si_b);
    __mmask8 mask8_di = compute_v8di_mask(v8di_a, v8di_b);
    __mmask8 mask8_df = compute_v8df_mask(v8df_a, v8df_b);
    __mmask16 mask16_sf = compute_v16sf_mask(v16sf_a, v16sf_b);
    
    // Call all blend functions in sequence
    blend_v64qi(v64qi_a, v64qi_b, mask64);
    blend_v32hi(v32hi_a, v32hi_b, mask32_hi);
    blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
    blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
    blend_v16si(v16si_a, v16si_b, mask16_si);
    blend_v8di(v8di_a, v8di_b, mask8_di);
    blend_v8df(v8df_a, v8df_b, mask8_df);
    blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
    
    // Compute and print checksum
    double checksum = compute_checksum();
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
