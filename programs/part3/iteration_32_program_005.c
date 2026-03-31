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

// V64QI blend using _mm512_mask_blend_epi8
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    // Data-dependent computation
    __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    return _mm512_xor_si512(temp, _mm512_set1_epi8(0x55));
}

// V32HI blend using _mm512_mask_blend_epi16
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Complex dataflow with comparisons
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(result, _mm512_setzero_si512());
    __m512i blended_again = _mm512_mask_blend_epi16(cmp_mask, 
                                                   _mm512_set1_epi16(-1), 
                                                   result);
    
    v32hi_result = blended_again;
    return blended_again;
}

// V32HF blend using _mm512_mask_blend_ph
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision: convert to float, operate, convert back
    __m512 float_vec = _mm512_cvtph_ps(result);
    __m512 scaled = _mm512_mul_ps(float_vec, _mm512_set1_ps(2.0f));
    __m512h reconverted = _mm512_cvtps_ph(scaled, _MM_FROUND_TO_NEAREST_INT);
    
    // Blend again with different mask
    __mmask32 alt_mask = mask ^ 0xAAAAAAAA; // Alter mask pattern
    __m512h final = _mm512_mask_blend_ph(alt_mask, reconverted, result);
    
    v32hf_result = final;
    return final;
}

// V32BF blend using _mm512_mask_blend_ph with bf16
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float for computation
    __m512 float_a = _mm512_cvtpbh_ps(a);
    __m512 float_b = _mm512_cvtpbh_ps(b);
    __m512 float_result = _mm512_add_ps(float_a, float_b);
    
    // Convert back and blend again
    __m512bh bf_result = _mm512_cvtne2ps_pbh(float_result, float_result);
    __m512bh final = _mm512_mask_blend_ph(mask >> 1, result, bf_result);
    
    v32bf_result = final;
    return final;
}

// V16SI blend using _mm512_mask_blend_epi32
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Multi-stage pipeline: use result in another blend
    __m512i temp = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    __mmask16 new_mask = _mm512_cmplt_epi32_mask(temp, _mm512_set1_epi32(100));
    __m512i final = _mm512_mask_blend_epi32(new_mask, result, temp);
    
    v16si_result = final;
    return final;
}

// V8DI blend using _mm512_mask_blend_epi64
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Create data-dependent mask
    __mmask8 lt_mask = _mm512_cmplt_epi64_mask(result, _mm512_set1_epi64(0));
    __m512i blended = _mm512_mask_blend_epi64(lt_mask, 
                                             _mm512_abs_epi64(result), 
                                             result);
    
    v8di_result = blended;
    return blended;
}

// V8DF blend using _mm512_mask_blend_pd
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Type conversion sequence
    __m512i int_vec = _mm512_cvtpd_epi64(result);
    __m512d reconverted = _mm512_cvtepi64_pd(int_vec);
    
    // Blend with converted data
    __m512d final = _mm512_mask_blend_pd(mask ^ 0x0F, result, reconverted);
    
    v8df_result = final;
    return final;
}

// V16SF blend using _mm512_mask_blend_ps
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Complex floating-point pipeline
    __m512 squared = _mm512_mul_ps(result, result);
    __m512 sqrt_val = _mm512_sqrt_ps(_mm512_add_ps(squared, _mm512_set1_ps(1.0f)));
    
    // Final blend with conditional
    __mmask16 alt_mask = _mm512_cmp_ps_mask(sqrt_val, result, _CMP_GT_OQ);
    __m512 final = _mm512_mask_blend_ps(alt_mask, result, sqrt_val);
    
    v16sf_result = final;
    return final;
}

// Initialize test data with pseudo-random values
void init_test_data(int argc, char** argv) {
    unsigned int seed = (unsigned int)argc;
    srand(seed);
}

// Main execution flow with data-dependent masks
int main(int argc, char** argv) {
    init_test_data(argc, argv);
    
    // Initialize vectors with pattern data
    int8_t char_data[64];
    int16_t short_data[32];
    int32_t int_data[16];
    int64_t long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t fp16_data[32];
    uint16_t bf16_data[32];
    
    // Fill with pseudo-random but deterministic values
    for (int i = 0; i < 64; i++) char_data[i] = (i * 13 + argc) % 256 - 128;
    for (int i = 0; i < 32; i++) short_data[i] = (i * 17 + argc) % 65536 - 32768;
    for (int i = 0; i < 16; i++) int_data[i] = (i * 23 + argc);
    for (int i = 0; i < 8; i++) long_data[i] = (int64_t)(i * 29 + argc) * 1000;
    for (int i = 0; i < 16; i++) float_data[i] = (i * 37 + argc) * 0.1f;
    for (int i = 0; i < 8; i++) double_data[i] = (i * 41 + argc) * 0.01;
    for (int i = 0; i < 32; i++) fp16_data[i] = (i * 43 + argc) & 0x7FFF;
    for (int i = 0; i < 32; i++) bf16_data[i] = (i * 47 + argc) & 0x7FFF;
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 16));
    
    __m512h v32hf_a = _mm512_loadu_ph(fp16_data);
    __m512h v32hf_b = _mm512_loadu_ph(fp16_data + 16);
    
    __m512bh v32bf_a = _mm512_loadu_bh(bf16_data);
    __m512bh v32bf_b = _mm512_loadu_bh(bf16_data + 16);
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 4));
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    // Create runtime-dependent masks
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((i + argc) % 3 == 0) mask64 |= (1ULL << i);
    }
    for (int i = 0; i < 32; i++) {
        if ((i + argc) % 2 == 0) mask32 |= (1U << i);
    }
    for (int i = 0; i < 16; i++) {
        if ((i + argc) % 4 == 0) mask16 |= (1U << i);
    }
    for (int i = 0; i < 8; i++) {
        if ((i + argc) % 5 == 0) mask8 |= (1U << i);
    }
    
    // Execute all blend operations in sequence
    __m512i r1 = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r2 = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h r3 = blend_v32hf(v32hf_a, v32hf_b, mask32);
    __m512bh r4 = blend_v32bf(v32bf_a, v32bf_b, mask32);
    __m512i r5 = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r6 = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r7 = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 r8 = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum to verify execution
    int64_t checksum = 0;
    
    // Extract and sum results
    int8_t char_res[64];
    _mm512_storeu_si512((__m512i*)char_res, r1);
    for (int i = 0; i < 64; i++) checksum += char_res[i];
    
    int16_t short_res[32];
    _mm512_storeu_si512((__m512i*)short_res, r2);
    for (int i = 0; i < 32; i++) checksum += short_res[i];
    
    // Add contributions from all results
    checksum += (int64_t)v64qi_result[0];
    checksum += (int64_t)v32hi_result[0];
    checksum += (int64_t)v16si_result[0];
    checksum += v8di_result[0];
    
    printf("Final checksum: %ld\n", checksum);
    return 0;
}
