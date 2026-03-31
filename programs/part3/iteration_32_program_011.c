#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
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
__m512i compute_v64qi_mask(__m512i a, __m512i b, int seed) {
    __m512i cmp = _mm512_cmpgt_epi8_mask(a, b);
    // Use runtime seed to modify mask
    __mmask64 mask = cmp ^ (seed & 0xFF);
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b, int seed) {
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 mask = cmp ^ (seed & 0xFFFF);
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b, int seed) {
    __mmask32 cmp = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    __mmask32 mask = cmp ^ (seed & 0xFFFFFFFF);
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b, int seed) {
    // For bfloat16, we need to convert to float for comparison
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    __mmask16 cmp = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
    __mmask32 mask = 0;
    // Expand 16-bit mask to 32-bit for bfloat16 blending
    for (int i = 0; i < 16; i++) {
        if (cmp & (1 << i)) {
            mask |= (3 << (i * 2));
        }
    }
    mask ^= (seed & 0xFFFFFFFF);
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b, int seed) {
    __mmask16 cmp = _mm512_cmpgt_epi32_mask(a, b);
    __mmask16 mask = cmp ^ (seed & 0xFFFF);
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b, int seed) {
    __mmask8 cmp = _mm512_cmpgt_epi64_mask(a, b);
    __mmask8 mask = cmp ^ (seed & 0xFF);
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b, int seed) {
    __mmask8 cmp = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    __mmask8 mask = cmp ^ (seed & 0xFF);
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b, int seed) {
    __mmask16 cmp = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    __mmask16 mask = cmp ^ (seed & 0xFFFF);
    return _mm512_mask_blend_ps(mask, a, b);
}

// Multi-stage pipeline: V64QI -> V16SI conversion and blending
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(int8_t* data, int seed) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)data);
    __m512i v2 = _mm512_loadu_si512((__m512i*)(data + 64));
    
    // First blend at V64QI level
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        mask64 |= ((uint64_t)((data[i] ^ seed) & 1) << i);
    }
    __m512i blended_qi = _mm512_mask_blend_epi8(mask64, v1, v2);
    
    // Convert to V16SI and blend again
    __m512i v1_si = _mm512_cvtepi8_epi32(_mm512_extracti64x4_epi64(blended_qi, 0));
    __m512i v2_si = _mm512_cvtepi8_epi32(_mm512_extracti64x4_epi64(blended_qi, 1));
    
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        mask16 |= ((data[i * 4] ^ seed) & 1) << i;
    }
    __m512i blended_si = _mm512_mask_blend_epi32(mask16, v1_si, v2_si);
    
    // Horizontal sum
    return _mm512_reduce_add_epi32(blended_si);
}

// Mixed precision pipeline: integer -> float conversion and blending
__attribute__((target("avx512f")))
double pipeline_int_to_float(int32_t* int_data, float* float_data, int seed) {
    __m512i vi1 = _mm512_loadu_si512((__m512i*)int_data);
    __m512i vi2 = _mm512_loadu_si512((__m512i*)(int_data + 16));
    
    // Blend integers
    __mmask16 mask_int = 0;
    for (int i = 0; i < 16; i++) {
        mask_int |= ((int_data[i] ^ seed) & 1) << i;
    }
    __m512i blended_int = _mm512_mask_blend_epi32(mask_int, vi1, vi2);
    
    // Convert to float and blend floats
    __m512 vf1 = _mm512_cvtepi32_ps(blended_int);
    __m512 vf2 = _mm512_loadu_ps(float_data);
    
    __mmask16 mask_float = 0;
    for (int i = 0; i < 16; i++) {
        mask_float |= ((int_data[i] ^ seed) & 2) ? (1 << i) : 0;
    }
    __m512 blended_float = _mm512_mask_blend_ps(mask_float, vf1, vf2);
    
    // Horizontal sum
    return _mm512_reduce_add_ps(blended_float);
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    // Initialize test data with runtime values
    int8_t data_8bit[128];
    int16_t data_16bit[64];
    int32_t data_32bit[32];
    int64_t data_64bit[16];
    float data_float[32];
    double data_double[16];
    uint16_t data_half[32];  // FP16 storage
    uint16_t data_bf16[32];  // BF16 storage
    
    for (int i = 0; i < 128; i++) data_8bit[i] = (rand() % 256) - 128;
    for (int i = 0; i < 64; i++) data_16bit[i] = (rand() % 65536) - 32768;
    for (int i = 0; i < 32; i++) data_32bit[i] = rand();
    for (int i = 0; i < 16; i++) data_64bit[i] = ((int64_t)rand() << 32) | rand();
    for (int i = 0; i < 32; i++) data_float[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 16; i++) data_double[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 32; i++) data_half[i] = rand() & 0xFFFF;
    for (int i = 0; i < 32; i++) data_bf16[i] = rand() & 0xFFFF;
    
    // Create masks from runtime data
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        mask64 |= ((uint64_t)(data_8bit[i] & 1) << i);
        if (i < 32) {
            mask32 |= ((data_16bit[i] & 1) << i);
            mask16 |= ((data_32bit[i] & 1) << (i % 16));
        }
        if (i < 8) {
            mask8 |= ((data_64bit[i] & 1) << i);
        }
    }
    
    // Load vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)data_8bit);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(data_8bit + 64));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)data_16bit);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(data_16bit + 32));
    
    __m512h v32hf_a = _mm512_castsi512_ph(_mm512_loadu_si512((__m512i*)data_half));
    __m512h v32hf_b = _mm512_castsi512_ph(_mm512_loadu_si512((__m512i*)(data_half + 16)));
    
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_loadu_si512((__m512i*)data_bf16));
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_loadu_si512((__m512i*)(data_bf16 + 16)));
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)data_32bit);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(data_32bit + 16));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)data_64bit);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(data_64bit + 8));
    
    __m512d v8df_a = _mm512_loadu_pd(data_double);
    __m512d v8df_b = _mm512_loadu_pd(data_double + 8);
    
    __m512 v16sf_a = _mm512_loadu_ps(data_float);
    __m512 v16sf_b = _mm512_loadu_ps(data_float + 16);
    
    // Execute all blend operations with data-dependent masks
    __m512i res_v64qi = compute_v64qi_mask(v64qi_a, v64qi_b, seed);
    __m512i res_v32hi = compute_v32hi_mask(v32hi_a, v32hi_b, seed);
    __m512h res_v32hf = compute_v32hf_mask(v32hf_a, v32hf_b, seed);
    __m512bh res_v32bf = compute_v32bf_mask(v32bf_a, v32bf_b, seed);
    __m512i res_v16si = compute_v16si_mask(v16si_a, v16si_b, seed);
    __m512i res_v8di = compute_v8di_mask(v8di_a, v8di_b, seed);
    __m512d res_v8df = compute_v8df_mask(v8df_a, v8df_b, seed);
    __m512 res_v16sf = compute_v16sf_mask(v16sf_a, v16sf_b, seed);
    
    // Store to volatile globals to prevent optimization
    v64qi_result = res_v64qi;
    v32hi_result = res_v32hi;
    v32hf_result = res_v32hf;
    v32bf_result = res_v32bf;
    v16si_result = res_v16si;
    v8di_result = res_v8di;
    v8df_result = res_v8df;
    v16sf_result = res_v16sf;
    
    // Execute multi-stage pipelines
    int64_t pipeline_sum1 = pipeline_v64qi_to_v16si(data_8bit, seed);
    double pipeline_sum2 = pipeline_int_to_float(data_32bit, data_float, seed);
    
    // Compute checksum
    int64_t checksum = 0;
    checksum += _mm512_reduce_add_epi64(res_v64qi);
    checksum += _mm512_reduce_add_epi64(res_v32hi);
    checksum += _mm512_reduce_add_epi64(_mm512_castph_si512(res_v32hf));
    checksum += _mm512_reduce_add_epi64(_mm512_castbh_si512(res_v32bf));
    checksum += _mm512_reduce_add_epi64(res_v16si);
    checksum += _mm512_reduce_add_epi64(res_v8di);
    checksum += (int64_t)_mm512_reduce_add_pd(res_v8df);
    checksum += (int64_t)_mm512_reduce_add_ps(res_v16sf);
    checksum += pipeline_sum1;
    checksum += (int64_t)pipeline_sum2;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
