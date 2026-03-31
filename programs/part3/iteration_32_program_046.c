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
    // Generate mask based on comparison
    return _mm512_cmpgt_epi8_mask(a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    // Generate mask based on comparison
    return _mm512_cmpgt_epi16_mask(a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    // Generate mask based on comparison
    return _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    // Convert to float for comparison
    __m512 a_f = _mm512_cvtpbh_ps(a);
    __m512 b_f = _mm512_cvtpbh_ps(b);
    return _mm512_cmp_ps_mask(a_f, b_f, _CMP_GT_OQ);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    return _mm512_cmpgt_epi32_mask(a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    return _mm512_cmpgt_epi64_mask(a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    return _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
}

// Blend implementations
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Multi-stage pipeline: blend then shuffle
    result = _mm512_shuffle_epi8(result, _mm512_set1_epi8(0x01));
    
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Data-dependent computation
    __m512i temp = _mm512_add_epi16(result, _mm512_set1_epi16(1));
    result = _mm512_mask_blend_epi16(mask >> 1, result, temp);
    
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision: blend then convert
    __m512 single_precision = _mm512_cvtph_ps(result);
    result = _mm512_cvtps_ph(single_precision, _MM_FROUND_TO_NEAREST_INT);
    
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float and back to ensure expansion
    __m512 float_vec = _mm512_cvtpbh_ps(result);
    result = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), float_vec);
    
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Pipeline: blend then arithmetic
    result = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Conditional blend based on mask
    __m512i alt = _mm512_slli_epi64(result, 1);
    result = _mm512_mask_blend_epi64(mask ^ 0xFF, result, alt);
    
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Mixed operations
    result = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Type conversion sequence
    __m512i int_vec = _mm512_cvtps_epi32(result);
    result = _mm512_cvtepi32_ps(int_vec);
    
    return result;
}

// Multi-stage pipeline that uses multiple blend types
__attribute__((target("avx512f,avx512bw")))
uint64_t multi_stage_pipeline(uint8_t* char_data, uint16_t* short_data,
                             int32_t* int_data, int64_t* long_data,
                             float* float_data, double* double_data,
                             __fp16* half_data, __bf16* bf16_data) {
    uint64_t checksum = 0;
    
    // Stage 1: V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 64));
    __mmask64 mask64 = compute_v64qi_mask(v64qi_a, v64qi_b);
    __m512i v64qi_result = blend_v64qi(v64qi_a, v64qi_b, mask64);
    v64qi_result = v64qi_result; // Store to volatile global
    checksum += _mm512_reduce_add_epi64(v64qi_result);
    
    // Stage 2: V32HI blend (using packed result from stage 1)
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 32));
    __mmask32 mask32 = compute_v32hi_mask(v32hi_a, v32hi_b);
    __m512i v32hi_result = blend_v32hi(v32hi_a, v32hi_b, mask32);
    v32hi_result = v32hi_result;
    checksum += _mm512_reduce_add_epi64(v32hi_result);
    
    // Stage 3: V32HF blend
    __m512h v32hf_a = _mm512_loadu_ph(half_data);
    __m512h v32hf_b = _mm512_loadu_ph(half_data + 32);
    __mmask32 mask32_hf = compute_v32hf_mask(v32hf_a, v32hf_b);
    __m512h v32hf_result = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
    v32hf_result = v32hf_result;
    
    // Stage 4: V32BF blend
    __m512bh v32bf_a = _mm512_loadu_si512((__m512i*)bf16_data);
    __m512bh v32bf_b = _mm512_loadu_si512((__m512i*)(bf16_data + 32));
    __mmask32 mask32_bf = compute_v32bf_mask(v32bf_a, v32bf_b);
    __m512bh v32bf_result = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
    v32bf_result = v32bf_result;
    
    // Stage 5: V16SI blend
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 16));
    __mmask16 mask16 = compute_v16si_mask(v16si_a, v16si_b);
    __m512i v16si_result = blend_v16si(v16si_a, v16si_b, mask16);
    v16si_result = v16si_result;
    checksum += _mm512_reduce_add_epi64(v16si_result);
    
    // Stage 6: V8DI blend
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 8));
    __mmask8 mask8 = compute_v8di_mask(v8di_a, v8di_b);
    __m512i v8di_result = blend_v8di(v8di_a, v8di_b, mask8);
    v8di_result = v8di_result;
    checksum += _mm512_reduce_add_epi64(v8di_result);
    
    // Stage 7: V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 8);
    __mmask8 mask8_df = compute_v8df_mask(v8df_a, v8df_b);
    __m512d v8df_result = blend_v8df(v8df_a, v8df_b, mask8_df);
    v8df_result = v8df_result;
    
    // Stage 8: V16SF blend
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 16);
    __mmask16 mask16_sf = compute_v16sf_mask(v16sf_a, v16sf_b);
    __m512 v16sf_result = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
    v16sf_result = v16sf_result;
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic data
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Allocate and initialize test data
    uint8_t char_data[128];
    uint16_t short_data[64];
    int32_t int_data[32];
    int64_t long_data[16];
    float float_data[32];
    double double_data[16];
    __fp16 half_data[64];
    __bf16 bf16_data[64];
    
    // Initialize with pattern that creates varied masks
    for (int i = 0; i < 128; i++) {
        char_data[i] = (uint8_t)(rand() % 256);
        if (i < 64) {
            short_data[i] = (uint16_t)(rand() % 65536);
            half_data[i] = (__fp16)((rand() % 100) / 10.0f);
            bf16_data[i] = (__bf16)((rand() % 100) / 10.0f);
        }
        if (i < 32) {
            int_data[i] = rand() % 1000;
            float_data[i] = (rand() % 1000) / 10.0f;
        }
        if (i < 16) {
            long_data[i] = rand() % 10000;
            double_data[i] = (rand() % 10000) / 10.0;
        }
    }
    
    // Run the multi-stage pipeline
    uint64_t checksum = multi_stage_pipeline(char_data, short_data, int_data,
                                            long_data, float_data, double_data,
                                            half_data, bf16_data);
    
    printf("Blend operations checksum: %lu\n", checksum);
    
    // Force all volatile results to be used
    printf("Volatile results stored\n");
    
    return 0;
}
