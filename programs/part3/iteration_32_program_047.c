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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b, int seed) {
    // Create data-dependent mask using comparisons
    __m512i cmp_result = _mm512_cmpgt_epi8_mask(a, b);
    __mmask64 base_mask = cmp_result;
    
    // Mix with seed to prevent constant folding
    __mmask64 dynamic_mask = base_mask ^ (seed & 0xFF);
    
    // Create pattern mask
    __mmask64 pattern_mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            pattern_mask |= (1ULL << i);
        }
    }
    
    return _mm512_mask_blend_epi8(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b, int seed) {
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 dynamic_mask = cmp_mask ^ (seed & 0xFFFF);
    
    __mmask32 pattern_mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 5 == 0) {
            pattern_mask |= (1U << i);
        }
    }
    
    return _mm512_mask_blend_epi16(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b, int seed) {
    // Convert seed to _Float16 pattern
    _Float16 seed_f16 = (_Float16)(seed & 0xFF);
    __m512h seed_vec = _mm512_set1_ph(seed_f16);
    
    // Compare with threshold
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, seed_vec, _CMP_GT_OQ);
    __mmask32 dynamic_mask = cmp_mask ^ (seed & 0xFFFFFFFF);
    
    // Alternating pattern
    __mmask32 pattern_mask = 0xAAAAAAAA; // 1010... pattern
    
    return _mm512_mask_blend_ph(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b, int seed) {
    // For bfloat16, we need to work with __m512i and convert
    __m512i a_int = _mm512_castps_si512(_mm512_cvtpbh_ps(a));
    __m512i b_int = _mm512_castps_si512(_mm512_cvtpbh_ps(b));
    
    // Create data-dependent mask
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a_int, b_int);
    __mmask32 dynamic_mask = cmp_mask ^ (seed & 0xFFFFFFFF);
    
    // Checkerboard pattern
    __mmask32 pattern_mask = 0x55555555; // 0101... pattern
    
    return _mm512_mask_blend_ph(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b, int seed) {
    __mmask16 cmp_mask = _mm512_cmpgt_epi32_mask(a, b);
    __mmask16 dynamic_mask = cmp_mask ^ (seed & 0xFFFF);
    
    // Every third element
    __mmask16 pattern_mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 3 == 0) {
            pattern_mask |= (1U << i);
        }
    }
    
    return _mm512_mask_blend_epi32(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b, int seed) {
    __mmask8 cmp_mask = _mm512_cmpgt_epi64_mask(a, b);
    __mmask8 dynamic_mask = cmp_mask ^ (seed & 0xFF);
    
    // Alternating mask
    __mmask8 pattern_mask = 0xAA; // 10101010
    
    return _mm512_mask_blend_epi64(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b, int seed) {
    double seed_d = (double)(seed & 0xFF);
    __m512d seed_vec = _mm512_set1_pd(seed_d);
    
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, seed_vec, _CMP_GT_OQ);
    __mmask8 dynamic_mask = cmp_mask ^ (seed & 0xFF);
    
    // Pattern: select elements where index is even
    __mmask8 pattern_mask = 0x55; // 01010101
    
    return _mm512_mask_blend_pd(dynamic_mask | pattern_mask, a, b);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b, int seed) {
    float seed_f = (float)(seed & 0xFF);
    __m512 seed_vec = _mm512_set1_ps(seed_f);
    
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, seed_vec, _CMP_GT_OQ);
    __mmask16 dynamic_mask = cmp_mask ^ (seed & 0xFFFF);
    
    // Pattern based on seed
    __mmask16 pattern_mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i * seed) % 4 == 0) {
            pattern_mask |= (1U << i);
        }
    }
    
    return _mm512_mask_blend_ps(dynamic_mask | pattern_mask, a, b);
}

// Multi-stage pipeline: V64QI -> V16SI conversion and blending
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(int seed) {
    // Stage 1: Blend V64QI
    int8_t data1[64], data2[64];
    for (int i = 0; i < 64; i++) {
        data1[i] = (int8_t)((i + seed) & 0xFF);
        data2[i] = (int8_t)((i * seed) & 0xFF);
    }
    
    __m512i vec1 = _mm512_loadu_si512(data1);
    __m512i vec2 = _mm512_loadu_si512(data2);
    
    // Dynamic mask based on data
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if (data1[i] > data2[i]) {
            mask64 |= (1ULL << i);
        }
    }
    mask64 ^= (seed & 0xFFFFFFFF);
    
    __m512i blended_qi = _mm512_mask_blend_epi8(mask64, vec1, vec2);
    
    // Stage 2: Convert to V16SI and blend
    // Pack bytes into 32-bit integers
    __m512i extended1 = _mm512_cvtepi8_epi32(_mm512_castsi512_si256(blended_qi));
    __m512i extended2 = _mm512_slli_epi32(extended1, 1);
    
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 2 == 0) {
            mask16 |= (1U << i);
        }
    }
    
    __m512i blended_si = _mm512_mask_blend_epi32(mask16, extended1, extended2);
    
    // Horizontal sum
    return _mm512_reduce_add_epi32(blended_si);
}

// Mixed precision pipeline: integer -> float conversion
__attribute__((target("avx512f")))
double pipeline_int_to_float(int seed) {
    // Start with integers
    int32_t int_data1[16], int_data2[16];
    for (int i = 0; i < 16; i++) {
        int_data1[i] = (i + seed) * 100;
        int_data2[i] = (i * seed) * 50;
    }
    
    __m512i int_vec1 = _mm512_loadu_si512(int_data1);
    __m512i int_vec2 = _mm512_loadu_si512(int_data2);
    
    // Blend integers
    __mmask16 int_mask = 0xAAAA; // 1010... pattern
    __m512i blended_int = _mm512_mask_blend_epi32(int_mask, int_vec1, int_vec2);
    
    // Convert to float and blend
    __m512 float_vec1 = _mm512_cvtepi32_ps(blended_int);
    __m512 float_vec2 = _mm512_mul_ps(float_vec1, _mm512_set1_ps(0.5f));
    
    __mmask16 float_mask = 0x5555; // 0101... pattern
    __m512 blended_float = _mm512_mask_blend_ps(float_mask, float_vec1, float_vec2);
    
    // Convert to double and blend
    __m512d double_vec1 = _mm512_cvtps_pd(_mm512_castps512_ps256(blended_float));
    __m512d double_vec2 = _mm512_mul_pd(double_vec1, _mm512_set1_pd(2.0));
    
    __mmask8 double_mask = 0xAA; // 10101010
    __m512d blended_double = _mm512_mask_blend_pd(double_mask, double_vec1, double_vec2);
    
    // Horizontal sum
    return _mm512_reduce_add_pd(blended_double);
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics with seed: %d\n", seed);
    
    // Initialize test data with seed-dependent values
    int8_t qi_data1[64], qi_data2[64];
    int16_t hi_data1[32], hi_data2[32];
    int32_t si_data1[16], si_data2[16];
    int64_t di_data1[8], di_data2[8];
    float sf_data1[16], sf_data2[16];
    double df_data1[8], df_data2[8];
    
    for (int i = 0; i < 64; i++) {
        qi_data1[i] = (int8_t)((i * seed + 1) & 0xFF);
        qi_data2[i] = (int8_t)((i + seed * 2) & 0xFF);
    }
    
    for (int i = 0; i < 32; i++) {
        hi_data1[i] = (int16_t)((i * seed + 3) & 0xFFFF);
        hi_data2[i] = (int16_t)((i + seed * 4) & 0xFFFF);
    }
    
    for (int i = 0; i < 16; i++) {
        si_data1[i] = (i * seed + 5) * 100;
        si_data2[i] = (i + seed * 6) * 200;
        sf_data1[i] = (float)((i * seed + 7) * 0.1f);
        sf_data2[i] = (float)((i + seed * 8) * 0.2f);
    }
    
    for (int i = 0; i < 8; i++) {
        di_data1[i] = (int64_t)(i * seed + 9) * 1000LL;
        di_data2[i] = (int64_t)(i + seed * 10) * 2000LL;
        df_data1[i] = (double)((i * seed + 11) * 0.01);
        df_data2[i] = (double)((i + seed * 12) * 0.02);
    }
    
    // Load vectors
    __m512i v64qi_1 = _mm512_loadu_si512(qi_data1);
    __m512i v64qi_2 = _mm512_loadu_si512(qi_data2);
    
    __m512i v32hi_1 = _mm512_loadu_si512(hi_data1);
    __m512i v32hi_2 = _mm512_loadu_si512(hi_data2);
    
    __m512i v16si_1 = _mm512_loadu_si512(si_data1);
    __m512i v16si_2 = _mm512_loadu_si512(si_data2);
    
    __m512i v8di_1 = _mm512_loadu_si512(di_data1);
    __m512i v8di_2 = _mm512_loadu_si512(di_data2);
    
    __m512 v16sf_1 = _mm512_loadu_ps(sf_data1);
    __m512 v16sf_2 = _mm512_loadu_ps(sf_data2);
    
    __m512d v8df_1 = _mm512_loadu_pd(df_data1);
    __m512d v8df_2 = _mm512_loadu_pd(df_data2);
    
    // For half-precision types, we need to convert
    _Float16 hf_data1[32], hf_data2[32];
    __bf16 bf_data1[32], bf_data2[32];
    
    for (int i = 0; i < 32; i++) {
        hf_data1[i] = (_Float16)((i * seed + 13) * 0.05f);
        hf_data2[i] = (_Float16)((i + seed * 14) * 0.1f);
        bf_data1[i] = (__bf16)((i * seed + 15) * 0.05f);
        bf_data2[i] = (__bf16)((i + seed * 16) * 0.1f);
    }
    
    __m512h v32hf_1 = _mm512_loadu_ph(hf_data1);
    __m512h v32hf_2 = _mm512_loadu_ph(hf_data2);
    
    __m512bh v32bf_1 = _mm512_loadu_bf16(bf_data1);
    __m512bh v32bf_2 = _mm512_loadu_bf16(bf_data2);
    
    // Create dynamic masks using runtime values
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 2 == 0) {
            mask64 |= (1ULL << i);
        }
    }
    
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 3 == 0) {
            mask32 |= (1U << i);
        }
    }
    
    for (int i = 0; i < 16; i++) {
        if ((i + seed * 2) % 4 == 0) {
            mask16 |= (1U << i);
        }
    }
    
    for (int i = 0; i < 8; i++) {
        if ((i * seed * 3) % 5 == 0) {
            mask8 |= (1U << i);
        }
    }
    
    // Execute all blend operations
    __m512i result_v64qi = compute_v64qi_mask(v64qi_1, v64qi_2, seed);
    __m512i result_v32hi = compute_v32hi_mask(v32hi_1, v32hi_2, seed);
    __m512h result_v32hf = compute_v32hf_mask(v32hf_1, v32hf_2, seed);
    __m512bh result_v32bf = compute_v32bf_mask(v32bf_1, v32bf_2, seed);
    __m512i result_v16si = compute_v16si_mask(v16si_1, v16si_2, seed);
    __m512i result_v8di = compute_v8di_mask(v8di_1, v8di_2, seed);
    __m512d result_v8df = compute_v8df_mask(v8df_1, v8df_2, seed);
    __m512 result_v16sf = compute_v16sf_mask(v16sf_1, v16sf_2, seed);
    
    // Store to volatile to prevent optimization
    v64qi_result = result_v64qi;
    v32hi_result = result_v32hi;
    v32hf_result = result_v32hf;
    v32bf_result = result_v32bf;
    v16si_result = result_v16si;
    v8di_result = result_v8di;
    v8df_result = result_v8df;
    v16sf_result = result_v16sf;
    
    // Execute pipeline operations
    int64_t pipeline_result1 = pipeline_v64qi_to_v16si(seed);
    double pipeline_result2 = pipeline_int_to_float(seed);
    
    // Compute checksum
    int64_t checksum = 0;
    
    // Add integer results
    checksum += _mm512_reduce_add_epi64(result_v64qi);
    checksum += _mm512_reduce_add_epi64(result_v32hi);
    checksum += _mm512_reduce_add_epi64(result_v16si);
    checksum += _mm512_reduce_add_epi64(result_v8di);
    
    // Add pipeline results
    checksum += pipeline_result1;
    checksum += (int64_t)pipeline_result2;
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
