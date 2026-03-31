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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    // Compare and create dynamic mask
    __mmask64 mask = _mm512_cmpneq_epi8_mask(a, b);
    // Modify mask based on data pattern
    mask = mask ^ 0xAAAAAAAAAAAAAAAA;  // XOR with pattern
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    __mmask32 mask = _mm512_cmpneq_epi16_mask(a, b);
    mask = mask ^ 0x55555555;  // Data-dependent modification
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_NEQ_OQ);
    mask = mask ^ 0xAAAAAAAA;  // Pattern modification
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    // For bfloat16, we need to use integer comparison
    __m512i a_int = _mm512_castps_si512(_mm512_cvtpbh_ps(a));
    __m512i b_int = _mm512_castps_si512(_mm512_cvtpbh_ps(b));
    __mmask32 mask = _mm512_cmpneq_epi16_mask(a_int, b_int);
    mask = mask ^ 0x55555555;
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    __mmask16 mask = _mm512_cmpneq_epi32_mask(a, b);
    mask = mask ^ 0xAAAA;  // Pattern modification
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    __mmask8 mask = _mm512_cmpneq_epi64_mask(a, b);
    mask = mask ^ 0xAA;  // Pattern modification
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_NEQ_OQ);
    mask = mask ^ 0x55;  // Pattern modification
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_NEQ_OQ);
    mask = mask ^ 0x5555;  // Pattern modification
    return _mm512_mask_blend_ps(mask, a, b);
}

// Multi-stage pipeline: V64QI -> V16SI conversion and blending
__attribute__((target("avx512bw,avx512f")))
__m512i pipeline_v64qi_to_v16si(__m512i data) {
    // First blend at V64QI level
    __m512i pattern = _mm512_set1_epi8(0x55);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(data, pattern);
    __m512i blended_8bit = _mm512_mask_blend_epi8(mask, data, pattern);
    
    // Convert to 16-bit and blend again
    __m512i extended = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_8bit, 0));
    __m512i pattern16 = _mm512_set1_epi16(0x3333);
    __mmask32 mask16 = _mm512_cmpgt_epi16_mask(extended, pattern16);
    __m512i blended_16bit = _mm512_mask_blend_epi16(mask16, extended, pattern16);
    
    // Convert to 32-bit and final blend
    __m512i extended32 = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(blended_16bit, 0));
    __m512i pattern32 = _mm512_set1_epi32(0x77777777);
    __mmask16 mask32 = _mm512_cmpgt_epi32_mask(extended32, pattern32);
    
    return _mm512_mask_blend_epi32(mask32, extended32, pattern32);
}

// Mixed precision pipeline: integer -> float -> blend
__attribute__((target("avx512f")))
float mixed_precision_pipeline(__m512i int_data) {
    // Convert integer to float
    __m512 float_data = _mm512_cvtepi32_ps(int_data);
    
    // Create comparison pattern
    __m512 pattern = _mm512_set1_ps(0.5f);
    
    // Generate dynamic mask
    __mmask16 mask = _mm512_cmp_ps_mask(float_data, pattern, _CMP_GT_OQ);
    
    // Blend float vectors
    __m512 blended = _mm512_mask_blend_ps(mask, float_data, pattern);
    
    // Horizontal add for checksum
    return _mm512_reduce_add_ps(blended);
}

// Main test function
int main(int argc, char** argv) {
    // Use argc for pseudo-random seed
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Initialize test data arrays
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    __fp16 half_data[32];
    __bf16 bfloat_data[32];
    
    // Fill with pseudo-random values
    for (int i = 0; i < 64; i++) char_data[i] = (char)(rand() % 256 - 128);
    for (int i = 0; i < 32; i++) short_data[i] = (short)(rand() % 65536 - 32768);
    for (int i = 0; i < 16; i++) int_data[i] = rand() - RAND_MAX/2;
    for (int i = 0; i < 8; i++) long_data[i] = ((long long)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX * 2.0 - 1.0;
    for (int i = 0; i < 32; i++) half_data[i] = (__fp16)((float)rand() / RAND_MAX * 2.0f - 1.0f);
    for (int i = 0; i < 32; i++) bfloat_data[i] = (__bf16)((float)rand() / RAND_MAX * 2.0f - 1.0f);
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_set1_epi8(0x55);
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_set1_epi16(0x3333);
    
    __m512h v32hf_a = _mm512_loadu_ph(half_data);
    __m512h v32hf_b = _mm512_set1_ph((__fp16)0.5f);
    
    __m512bh v32bf_a = _mm512_loadu_ph(bfloat_data);
    __m512bh v32bf_b = _mm512_set1_ph((__bf16)0.5f);
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_set1_epi32(0x77777777);
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAALL);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_set1_pd(0.5);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_set1_ps(0.5f);
    
    // Create dynamic masks based on array indices and data
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if (char_data[i] > 0) mask64 |= (1ULL << i);
    }
    
    for (int i = 0; i < 32; i++) {
        if (short_data[i] > 0) mask32 |= (1U << i);
    }
    
    for (int i = 0; i < 16; i++) {
        if (int_data[i] > 0) mask16 |= (1U << i);
    }
    
    for (int i = 0; i < 8; i++) {
        if (long_data[i] > 0) mask8 |= (1U << i);
    }
    
    // Execute all blend operations
    __m512i result_v64qi = compute_v64qi_mask(v64qi_a, v64qi_b);
    __m512i result_v32hi = compute_v32hi_mask(v32hi_a, v32hi_b);
    __m512h result_v32hf = compute_v32hf_mask(v32hf_a, v32hf_b);
    __m512bh result_v32bf = compute_v32bf_mask(v32bf_a, v32bf_b);
    __m512i result_v16si = compute_v16si_mask(v16si_a, v16si_b);
    __m512i result_v8di = compute_v8di_mask(v8di_a, v8di_b);
    __m512d result_v8df = compute_v8df_mask(v8df_a, v8df_b);
    __m512 result_v16sf = compute_v16sf_mask(v16sf_a, v16sf_b);
    
    // Store to volatile globals to prevent optimization
    v64qi_result = result_v64qi;
    v32hi_result = result_v32hi;
    v32hf_result = result_v32hf;
    v32bf_result = result_v32bf;
    v16si_result = result_v16si;
    v8di_result = result_v8di;
    v8df_result = result_v8df;
    v16sf_result = result_v16sf;
    
    // Execute multi-stage pipeline
    __m512i pipeline_result = pipeline_v64qi_to_v16si(v64qi_a);
    
    // Execute mixed precision pipeline
    float checksum = mixed_precision_pipeline(v16si_a);
    
    // Additional data-dependent blending with runtime values
    __m512i final_result = _mm512_mask_blend_epi32(
        mask16, 
        result_v16si, 
        _mm512_set1_epi32(0xFFFFFFFF)
    );
    
    // Compute final checksum
    int64_t final_checksum = 0;
    int32_t* final_ptr = (int32_t*)&final_result;
    for (int i = 0; i < 16; i++) {
        final_checksum += final_ptr[i];
    }
    
    // Use checksum to prevent dead code elimination
    printf("Checksum: %ld, Float checksum: %f\n", final_checksum, checksum);
    
    return 0;
}
