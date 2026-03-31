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

// V64QI: 64x 8-bit integers
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use runtime-derived mask to prevent constant folding
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to force materialization
    g_v64qi_result = result;
    
    // Additional data-dependent computation
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_xor_si512(result, shifted);
}

// V32HI: 32x 16-bit integers
__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create conditional blend based on comparison
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 final_mask = mask & cmp_mask;  // Combine masks
    
    __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
    
    // Multi-stage pipeline: feed result into another operation
    __m512i scaled = _mm512_mullo_epi16(result, _mm512_set1_epi16(2));
    g_v32hi_result = scaled;
    
    return _mm512_add_epi16(result, scaled);
}

// V32HF: 32x half-precision floats (requires AVX512-FP16)
#ifdef __AVX512FP16__
__attribute__((noinline, target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Mixed precision: compare and blend
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    __mmask32 blend_mask = mask ^ cmp_mask;  // XOR for non-trivial mask
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Type conversion stress: convert to float, operate, convert back
    __m512 float_vec = _mm512_cvtph_ps(result);
    __m512 scaled = _mm512_mul_ps(float_vec, _mm512_set1_ps(1.5f));
    __m512h converted = _mm512_cvtps_ph(scaled, _MM_FROUND_CUR_DIRECTION);
    
    g_v32hf_result = converted;
    return _mm512_add_ph(result, converted);
}
#endif

// V32BF: 32x brain float16 (requires AVX512-BF16)
#ifdef __AVX512BF16__
__attribute__((noinline, target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use same intrinsic as V32HF but with bfloat16 type
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float for computation
    __m512 float_a = _mm512_cvtneobf16_ps(a);
    __m512 float_b = _mm512_cvtneobf16_ps(b);
    __m512 float_result = _mm512_mask_blend_ps(mask, float_a, float_b);
    
    // Convert back to bfloat16
    __m512bh converted = _mm512_cvtneps_bf16(float_result);
    
    g_v32bf_result = converted;
    return _mm512_mask_blend_ph(mask, result, converted);
}
#endif

// V16SI: 16x 32-bit integers
__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Data-dependent mask from previous computation
    __m512i abs_diff = _mm512_abs_epi32(_mm512_sub_epi32(a, b));
    __mmask16 diff_mask = _mm512_cmpgt_epi32_mask(abs_diff, _mm512_set1_epi32(100));
    __mmask16 final_mask = mask | diff_mask;
    
    __m512i result = _mm512_mask_blend_epi32(final_mask, a, b);
    
    // Feed into next stage
    g_v16si_result = result;
    
    // Horizontal add to create dependency
    __m512i hadd1 = _mm512_add_epi32(result, _mm512_bsrli_epi128(result, 4));
    __m512i hadd2 = _mm512_add_epi32(hadd1, _mm512_bsrli_epi128(hadd1, 8));
    return hadd2;
}

// V8DI: 8x 64-bit integers
__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask computation
    __mmask8 cmp_mask = _mm512_cmpgt_epi64_mask(a, b);
    __mmask8 parity_mask = 0;
    for (int i = 0; i < 8; i++) {
        parity_mask |= ((mask >> i) & 1) ^ ((cmp_mask >> i) & 1) ? (1 << i) : 0;
    }
    
    __m512i result = _mm512_mask_blend_epi64(parity_mask, a, b);
    
    g_v8di_result = result;
    
    // Mixed with floating point
    __m512d double_vec = _mm512_cvtepi64_pd(result);
    __m512d scaled = _mm512_mul_pd(double_vec, _mm512_set1_pd(0.5));
    __m512i converted_back = _mm512_cvtpd_epi64(scaled);
    
    return _mm512_add_epi64(result, converted_back);
}

// V8DF: 8x double-precision floats
__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Conditional blend based on comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 final_mask = mask & cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(final_mask, a, b);
    
    // Store to volatile
    g_v8df_result = result;
    
    // Convert to integer and back
    __m512i int_vec = _mm512_cvtpd_epi64(result);
    __m512d reconverted = _mm512_cvtepi64_pd(int_vec);
    
    return _mm512_add_pd(result, reconverted);
}

// V16SF: 16x single-precision floats
__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Multi-stage blending
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, b, _CMP_NEQ_UQ);
    __mmask16 blend1_mask = mask | cmp_mask;
    
    __m512 temp = _mm512_mask_blend_ps(blend1_mask, a, b);
    
    // Second blend with modified data
    __m512 scaled_b = _mm512_mul_ps(b, _mm512_set1_ps(2.0f));
    __mmask16 cmp2_mask = _mm512_cmp_ps_mask(temp, scaled_b, _CMP_GT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(cmp2_mask, temp, scaled_b);
    
    g_v16sf_result = result;
    
    // Horizontal reduction
    __m512 hadd1 = _mm512_add_ps(result, _mm512_permute_ps(result, 0xB1));
    __m512 hadd2 = _mm512_add_ps(hadd1, _mm512_permute_ps(hadd1, 0x4E));
    return hadd2;
}

// Initialize test data with pseudo-random values
void init_test_data(int argc, char* argv[]) {
    unsigned int seed = (unsigned int)argc;
    srand(seed);
}

int main(int argc, char* argv[]) {
    init_test_data(argc, argv);
    
    // Initialize vectors with runtime data to prevent constant folding
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    
    // Fill with pseudo-random values based on argc
    for (int i = 0; i < 64; i++) {
        char_data[i] = (char)((i * 13 + argc) & 0xFF);
    }
    for (int i = 0; i < 32; i++) {
        short_data[i] = (short)((i * 17 + argc * 3) & 0xFFFF);
    }
    for (int i = 0; i < 16; i++) {
        int_data[i] = (i * 23 + argc * 5);
    }
    for (int i = 0; i < 8; i++) {
        long_data[i] = (long long)((i * 29 + argc * 7) * 1000LL);
    }
    for (int i = 0; i < 16; i++) {
        float_data[i] = (float)((i * 31 + argc * 11) * 0.1f);
    }
    for (int i = 0; i < 8; i++) {
        double_data[i] = (double)((i * 37 + argc * 13) * 0.01);
    }
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 16));
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 4));
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    // Create runtime-derived masks
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        mask64 |= ((i + argc) % 3 == 0) ? (1ULL << i) : 0;
    }
    for (int i = 0; i < 32; i++) {
        mask32 |= ((i + argc) % 2 == 0) ? (1U << i) : 0;
    }
    for (int i = 0; i < 16; i++) {
        mask16 |= ((i + argc) % 4 == 0) ? (1U << i) : 0;
    }
    for (int i = 0; i < 8; i++) {
        mask8 |= ((i + argc) % 3 == 0) ? (1U << i) : 0;
    }
    
    // Execute all blend operations in sequence
    __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    #ifdef __AVX512FP16__
    // For half-precision, we need to convert from float
    __m512h v32hf_a = _mm512_cvtps_ph(v16sf_a, _MM_FROUND_CUR_DIRECTION);
    __m512h v32hf_b = _mm512_cvtps_ph(v16sf_b, _MM_FROUND_CUR_DIRECTION);
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    #endif
    
    #ifdef __AVX512BF16__
    // For bfloat16
    __m512bh v32bf_a = _mm512_cvtne2ps_pbh(v16sf_a, v16sf_a);
    __m512bh v32bf_b = _mm512_cvtne2ps_pbh(v16sf_b, v16sf_b);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    #endif
    
    __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum to verify execution
    uint64_t checksum = 0;
    
    // Extract and accumulate results
    alignas(64) uint8_t r64qi[64];
    alignas(64) uint16_t r32hi[32];
    alignas(64) uint32_t r16si[16];
    alignas(64) uint64_t r8di[8];
    alignas(64) float r16sf[16];
    alignas(64) double r8df[8];
    
    _mm512_store_si512(r64qi, result64qi);
    _mm512_store_si512(r32hi, result32hi);
    _mm512_store_si512(r16si, result16si);
    _mm512_store_si512(r8di, result8di);
    _mm512_store_ps(r16sf, result16sf);
    _mm512_store_pd(r8df, result8df);
    
    for (int i = 0; i < 64; i++) checksum += r64qi[i];
    for (int i = 0; i < 32; i++) checksum += r32hi[i];
    for (int i = 0; i < 16; i++) checksum += r16si[i];
    for (int i = 0; i < 8; i++) checksum += r8di[i];
    for (int i = 0; i < 16; i++) checksum += (uint64_t)r16sf[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)r8df[i];
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}
