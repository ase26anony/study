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
    // Mix with runtime value to prevent constant folding
    volatile int seed = rand();
    mask ^= (__mmask64)(seed & 0xFF);
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    __mmask32 mask = _mm512_cmpneq_epi16_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_NEQ_OQ);
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    // For bfloat16, we need to convert to float for comparison
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    __mmask16 mask32 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_NEQ_OQ);
    
    // Expand 16-bit mask to 32-bit for blend
    __mmask32 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (mask32 & (1 << i)) {
            mask |= (3 << (i * 2));
        }
    }
    
    volatile int seed = rand();
    mask ^= (__mmask32)(seed & 0xFFFF);
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    __mmask16 mask = _mm512_cmpneq_epi32_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask16)(seed & 0xFFFF);
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    __mmask8 mask = _mm512_cmpneq_epi64_mask(a, b);
    volatile int seed = rand();
    mask ^= (__mmask8)(seed & 0xFF);
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_NEQ_OQ);
    volatile int seed = rand();
    mask ^= (__mmask8)(seed & 0xFF);
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_NEQ_OQ);
    volatile int seed = rand();
    mask ^= (__mmask16)(seed & 0xFFFF);
    return _mm512_mask_blend_ps(mask, a, b);
}

// Multi-stage pipeline: V64QI -> V16SI conversion and blending
__attribute__((target("avx512bw,avx512f")))
int pipeline_v64qi_to_v16si(unsigned int seed) {
    // Stage 1: Blend 64x char
    char data1[64], data2[64];
    for (int i = 0; i < 64; i++) {
        data1[i] = (char)((seed + i) & 0xFF);
        data2[i] = (char)((seed * i) & 0xFF);
    }
    
    __m512i v1 = _mm512_loadu_si512(data1);
    __m512i v2 = _mm512_loadu_si512(data2);
    
    // Dynamic mask based on data
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((data1[i] ^ data2[i]) & 1) {
            mask64 |= (1ULL << i);
        }
    }
    
    __m512i blended64qi = _mm512_mask_blend_epi8(mask64, v1, v2);
    v64qi_result = blended64qi;
    
    // Stage 2: Convert to 16x int and blend
    __m512i v1_32 = _mm512_cvtepi8_epi32(_mm512_castsi512_si256(blended64qi));
    __m512i v2_32 = _mm512_slli_epi32(v1_32, 1);
    
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if (((seed >> i) & 1) ^ (i & 1)) {
            mask16 |= (1 << i);
        }
    }
    
    __m512i blended16si = _mm512_mask_blend_epi32(mask16, v1_32, v2_32);
    v16si_result = blended16si;
    
    // Horizontal sum for checksum
    return _mm512_reduce_add_epi32(blended16si);
}

// Mixed precision pipeline: Float <-> Int conversions with blending
__attribute__((target("avx512f")))
double pipeline_mixed_precision(unsigned int seed) {
    // Start with integers
    int data1[16], data2[16];
    for (int i = 0; i < 16; i++) {
        data1[i] = seed + i;
        data2[i] = seed * i;
    }
    
    __m512i v1_int = _mm512_loadu_si512(data1);
    __m512i v2_int = _mm512_loadu_si512(data2);
    
    // Blend integers
    __mmask16 mask_int = 0;
    for (int i = 0; i < 16; i++) {
        if ((data1[i] < data2[i]) ^ (i & 1)) {
            mask_int |= (1 << i);
        }
    }
    
    __m512i blended_int = _mm512_mask_blend_epi32(mask_int, v1_int, v2_int);
    
    // Convert to float and blend
    __m512 v1_float = _mm512_cvtepi32_ps(blended_int);
    __m512 v2_float = _mm512_mul_ps(v1_float, _mm512_set1_ps(2.0f));
    
    __mmask16 mask_float = mask_int ^ 0xAAAA; // Alternate pattern
    __m512 blended_float = _mm512_mask_blend_ps(mask_float, v1_float, v2_float);
    v16sf_result = blended_float;
    
    // Convert to double and blend
    __m512d v1_double = _mm512_cvtps_pd(_mm512_castps512_ps256(blended_float));
    __m512d v2_double = _mm512_mul_pd(v1_double, _mm512_set1_pd(1.5));
    
    __mmask8 mask_double = 0;
    for (int i = 0; i < 8; i++) {
        mask_double |= ((seed >> i) & 1) << i;
    }
    
    __m512d blended_double = _mm512_mask_blend_pd(mask_double, v1_double, v2_double);
    v8df_result = blended_double;
    
    return _mm512_reduce_add_pd(blended_double);
}

// Main test driver
int main(int argc, char** argv) {
    // Use argc for pseudo-random seed
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    int checksum = 0;
    double fp_checksum = 0.0;
    
    // Initialize test data
    int8_t data_epi8[64];
    int16_t data_epi16[32];
    float data_ps[16];
    double data_pd[8];
    
    for (int i = 0; i < 64; i++) data_epi8[i] = (int8_t)((seed + i * 3) & 0xFF);
    for (int i = 0; i < 32; i++) data_epi16[i] = (int16_t)((seed + i * 5) & 0xFFFF);
    for (int i = 0; i < 16; i++) data_ps[i] = (float)(seed + i) * 0.1f;
    for (int i = 0; i < 8; i++) data_pd[i] = (double)(seed + i) * 0.2;
    
    // Test V64QI
    __m512i v64qi_a = _mm512_loadu_si512(data_epi8);
    __m512i v64qi_b = _mm512_slli_epi16(v64qi_a, 1);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        mask64 |= ((uint64_t)((data_epi8[i] & 1) ^ (i & 1)) << i);
    }
    __m512i res64qi = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
    v64qi_result = res64qi;
    
    // Test V32HI
    __m512i v32hi_a = _mm512_loadu_si512(data_epi16);
    __m512i v32hi_b = _mm512_add_epi16(v32hi_a, _mm512_set1_epi16(100));
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        mask32 |= ((uint32_t)((data_epi16[i] > 0) ^ (i & 1)) << i);
    }
    __m512i res32hi = _mm512_mask_blend_epi16(mask32, v32hi_a, v32hi_b);
    v32hi_result = res32hi;
    
    // Test V16SI
    __m512i v16si_a = _mm512_loadu_si512((int*)data_ps); // Reuse float array as int
    __m512i v16si_b = _mm512_slli_epi32(v16si_a, 2);
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        mask16 |= ((uint16_t)((((int*)data_ps)[i] & 4) != 0) << i);
    }
    __m512i res16si = _mm512_mask_blend_epi32(mask16, v16si_a, v16si_b);
    v16si_result = res16si;
    
    // Test V8DI
    __m512i v8di_a = _mm512_loadu_si512(data_pd); // Reuse double array as long
    __m512i v8di_b = _mm512_add_epi64(v8di_a, _mm512_set1_epi64(1000));
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        mask8 |= ((uint8_t)((((int64_t*)data_pd)[i] & 8) != 0) << i);
    }
    __m512i res8di = _mm512_mask_blend_epi64(mask8, v8di_a, v8di_b);
    v8di_result = res8di;
    
    // Test V16SF
    __m512 v16sf_a = _mm512_loadu_ps(data_ps);
    __m512 v16sf_b = _mm512_mul_ps(v16sf_a, _mm512_set1_ps(3.0f));
    mask16 = 0;
    for (int i = 0; i < 16; i++) {
        mask16 |= ((uint16_t)(data_ps[i] > 0.5f) << i);
    }
    __m512 res16sf = _mm512_mask_blend_ps(mask16, v16sf_a, v16sf_b);
    v16sf_result = res16sf;
    
    // Test V8DF
    __m512d v8df_a = _mm512_loadu_pd(data_pd);
    __m512d v8df_b = _mm512_mul_pd(v8df_a, _mm512_set1_pd(2.5));
    mask8 = 0;
    for (int i = 0; i < 8; i++) {
        mask8 |= ((uint8_t)(data_pd[i] > 1.0) << i);
    }
    __m512d res8df = _mm512_mask_blend_pd(mask8, v8df_a, v8df_b);
    v8df_result = res8df;
    
    // Run pipeline tests
    checksum += pipeline_v64qi_to_v16si(seed);
    fp_checksum += pipeline_mixed_precision(seed);
    
    // Compute final checksum
    checksum += _mm512_reduce_add_epi16(res32hi);
    checksum += _mm512_reduce_add_epi32(res16si);
    checksum += (int)_mm512_reduce_add_pd(res8df);
    
    printf("Checksum: %d, FP Checksum: %f\n", checksum, fp_checksum);
    
    return 0;
}
