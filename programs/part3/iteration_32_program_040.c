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
__m512i compute_v64qi_mask(__m512i a, __m512i b, int seed) {
    // Create data-dependent mask using runtime seed
    __m512i cmp = _mm512_cmpgt_epi8_mask(a, b);
    __mmask64 mask = cmp;
    
    // Mix with seed to prevent constant folding
    for (int i = 0; i < 64; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask ^= (1ULL << i);
        }
    }
    
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b, int seed) {
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 mask = cmp;
    
    for (int i = 0; i < 32; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask ^= (1UL << i);
        }
    }
    
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b, int seed) {
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    for (int i = 0; i < 32; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask ^= (1UL << i);
        }
    }
    
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b, int seed) {
    // Convert to float for comparison
    __m512 a_f = _mm512_cvtpbh_ps(a);
    __m512 b_f = _mm512_cvtpbh_ps(b);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a_f, b_f, _CMP_GT_OQ);
    
    // Expand to 32-bit mask for blend
    __mmask32 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (mask16 & (1 << i)) {
            mask |= (3UL << (i * 2));
        }
    }
    
    for (int i = 0; i < 32; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask ^= (1UL << i);
        }
    }
    
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b, int seed) {
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
    
    for (int i = 0; i < 16; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask ^= (1 << i);
        }
    }
    
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b, int seed) {
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
    
    for (int i = 0; i < 8; i++) {
        if ((seed >> i) & 1) {
            mask ^= (1 << i);
        }
    }
    
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b, int seed) {
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    
    for (int i = 0; i < 8; i++) {
        if ((seed >> i) & 1) {
            mask ^= (1 << i);
        }
    }
    
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b, int seed) {
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    
    for (int i = 0; i < 16; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask ^= (1 << i);
        }
    }
    
    return _mm512_mask_blend_ps(mask, a, b);
}

// Multi-stage pipeline: V64QI -> V32HI -> V16SI -> V8DI
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_integer_blends(int seed) {
    // Stage 1: V64QI blend
    int8_t data8_a[64], data8_b[64];
    for (int i = 0; i < 64; i++) {
        data8_a[i] = (int8_t)((i * seed) & 0xFF);
        data8_b[i] = (int8_t)((i * seed + 1) & 0xFF);
    }
    
    __m512i v64qi_a = _mm512_loadu_si512(data8_a);
    __m512i v64qi_b = _mm512_loadu_si512(data8_b);
    __m512i v64qi_result = compute_v64qi_mask(v64qi_a, v64qi_b, seed);
    
    // Store to volatile to force materialization
    v64qi_result = v64qi_result;
    
    // Stage 2: Convert to V32HI and blend
    int16_t data16_a[32], data16_b[32];
    for (int i = 0; i < 32; i++) {
        data16_a[i] = (int16_t)((i * seed * 2) & 0xFFFF);
        data16_b[i] = (int16_t)((i * seed * 2 + 1) & 0xFFFF);
    }
    
    __m512i v32hi_a = _mm512_loadu_si512(data16_a);
    __m512i v32hi_b = _mm512_loadu_si512(data16_b);
    __m512i v32hi_result = compute_v32hi_mask(v32hi_a, v32hi_b, seed);
    v32hi_result = v32hi_result;
    
    // Stage 3: V16SI blend
    int32_t data32_a[16], data32_b[16];
    for (int i = 0; i < 16; i++) {
        data32_a[i] = i * seed * 3;
        data32_b[i] = i * seed * 3 + 1;
    }
    
    __m512i v16si_a = _mm512_loadu_si512(data32_a);
    __m512i v16si_b = _mm512_loadu_si512(data32_b);
    __m512i v16si_result = compute_v16si_mask(v16si_a, v16si_b, seed);
    v16si_result = v16si_result;
    
    // Stage 4: V8DI blend
    int64_t data64_a[8], data64_b[8];
    for (int i = 0; i < 8; i++) {
        data64_a[i] = (int64_t)i * seed * 4;
        data64_b[i] = (int64_t)i * seed * 4 + 1;
    }
    
    __m512i v8di_a = _mm512_loadu_si512(data64_a);
    __m512i v8di_b = _mm512_loadu_si512(data64_b);
    __m512i v8di_result = compute_v8di_mask(v8di_a, v8di_b, seed);
    v8di_result = v8di_result;
    
    // Compute checksum
    uint64_t checksum = 0;
    int8_t* p8 = (int8_t*)&v64qi_result;
    for (int i = 0; i < 64; i++) checksum += p8[i];
    
    int16_t* p16 = (int16_t*)&v32hi_result;
    for (int i = 0; i < 32; i++) checksum += p16[i];
    
    int32_t* p32 = (int32_t*)&v16si_result;
    for (int i = 0; i < 16; i++) checksum += p32[i];
    
    int64_t* p64 = (int64_t*)&v8di_result;
    for (int i = 0; i < 8; i++) checksum += p64[i];
    
    return checksum;
}

// Multi-stage pipeline: V32HF -> V32BF -> V16SF -> V8DF
__attribute__((target("avx512fp16,avx512bf16,avx512f")))
float pipeline_float_blends(int seed) {
    // Stage 1: V32HF blend
    _Float16 data16_a[32], data16_b[32];
    for (int i = 0; i < 32; i++) {
        data16_a[i] = (_Float16)(i * 0.1f * seed);
        data16_b[i] = (_Float16)(i * 0.1f * seed + 0.5f);
    }
    
    __m512h v32hf_a = _mm512_loadu_ph(data16_a);
    __m512h v32hf_b = _mm512_loadu_ph(data16_b);
    __m512h v32hf_result = compute_v32hf_mask(v32hf_a, v32hf_b, seed);
    v32hf_result = v32hf_result;
    
    // Stage 2: V32BF blend
    __bf16 databf_a[32], databf_b[32];
    for (int i = 0; i < 32; i++) {
        databf_a[i] = (__bf16)(i * 0.05f * seed);
        databf_b[i] = (__bf16)(i * 0.05f * seed + 0.25f);
    }
    
    __m512bh v32bf_a = _mm512_loadu_si512(databf_a);
    __m512bh v32bf_b = _mm512_loadu_si512(databf_b);
    __m512bh v32bf_result = compute_v32bf_mask(v32bf_a, v32bf_b, seed);
    v32bf_result = v32bf_result;
    
    // Stage 3: V16SF blend
    float data32_a[16], data32_b[16];
    for (int i = 0; i < 16; i++) {
        data32_a[i] = i * 0.2f * seed;
        data32_b[i] = i * 0.2f * seed + 1.0f;
    }
    
    __m512 v16sf_a = _mm512_loadu_ps(data32_a);
    __m512 v16sf_b = _mm512_loadu_ps(data32_b);
    __m512 v16sf_result = compute_v16sf_mask(v16sf_a, v16sf_b, seed);
    v16sf_result = v16sf_result;
    
    // Stage 4: V8DF blend
    double data64_a[8], data64_b[8];
    for (int i = 0; i < 8; i++) {
        data64_a[i] = i * 0.3 * seed;
        data64_b[i] = i * 0.3 * seed + 0.5;
    }
    
    __m512d v8df_a = _mm512_loadu_pd(data64_a);
    __m512d v8df_b = _mm512_loadu_pd(data64_b);
    __m512d v8df_result = compute_v8df_mask(v8df_a, v8df_b, seed);
    v8df_result = v8df_result;
    
    // Compute checksum
    float checksum = 0.0f;
    _Float16* p16 = (_Float16*)&v32hf_result;
    for (int i = 0; i < 32; i++) checksum += p16[i];
    
    __bf16* pbf = (__bf16*)&v32bf_result;
    for (int i = 0; i < 32; i++) checksum += (float)pbf[i];
    
    float* p32 = (float*)&v16sf_result;
    for (int i = 0; i < 16; i++) checksum += p32[i];
    
    double* p64 = (double*)&v8df_result;
    for (int i = 0; i < 8; i++) checksum += p64[i];
    
    return checksum;
}

// Mixed precision conversion sequence
__attribute__((target("avx512bw,avx512f")))
double mixed_precision_conversions(int seed) {
    // Start with integer data
    int8_t int8_data[64];
    for (int i = 0; i < 64; i++) {
        int8_data[i] = (int8_t)((i * seed) % 128);
    }
    
    __m512i v_int8 = _mm512_loadu_si512(int8_data);
    
    // Convert to 16-bit integers and blend
    __m512i v_int16_a = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(v_int8, 0));
    __m512i v_int16_b = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(v_int8, 1));
    
    __mmask32 mask16 = 0xAAAAAAAA; // Alternating pattern
    for (int i = 0; i < 32; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask16 ^= (1UL << i);
        }
    }
    __m512i v_int16_blend = _mm512_mask_blend_epi16(mask16, v_int16_a, v_int16_b);
    
    // Convert to 32-bit floats and blend
    __m512 v_float_a = _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(v_int16_blend, 0)));
    __m512 v_float_b = _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(v_int16_blend, 1)));
    
    __mmask16 mask32 = 0xAAAA; // Alternating pattern
    for (int i = 0; i < 16; i++) {
        if ((seed >> (i % 8)) & 1) {
            mask32 ^= (1 << i);
        }
    }
    __m512 v_float_blend = _mm512_mask_blend_ps(mask32, v_float_a, v_float_b);
    
    // Convert to doubles and blend
    __m512d v_double_a = _mm512_cvtps_pd(_mm512_extractf32x8_ps(v_float_blend, 0));
    __m512d v_double_b = _mm512_cvtps_pd(_mm512_extractf32x8_ps(v_float_blend, 1));
    
    __mmask8 mask64 = 0xAA; // Alternating pattern
    for (int i = 0; i < 8; i++) {
        if ((seed >> i) & 1) {
            mask64 ^= (1 << i);
        }
    }
    __m512d v_double_blend = _mm512_mask_blend_pd(mask64, v_double_a, v_double_b);
    
    // Store to volatile
    v8df_result = v_double_blend;
    
    // Horizontal sum
    return _mm512_reduce_add_pd(v_double_blend);
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend operations with seed: %d\n", seed);
    
    // Execute all blend pipelines
    uint64_t int_checksum = pipeline_integer_blends(seed);
    float float_checksum = pipeline_float_blends(seed);
    double mixed_checksum = mixed_precision_conversions(seed);
    
    printf("Integer pipeline checksum: %lu\n", int_checksum);
    printf("Float pipeline checksum: %f\n", float_checksum);
    printf("Mixed precision checksum: %f\n", mixed_checksum);
    
    // Force all volatile results to be used
    printf("Volatile results: %lx %lx %lx %lx\n", 
           (long)v64qi_result[0], (long)v32hi_result[0],
           (long)v16si_result[0], (long)v8di_result[0]);
    
    return 0;
}
