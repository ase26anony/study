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

// Prevent inlining and force RTL expansion
__attribute__((noinline, target("avx512f,avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use runtime-derived mask to prevent constant folding
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    v64qi_result = result; // Volatile store prevents elimination
    return result;
}

__attribute__((noinline, target("avx512f,avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Data-dependent computation: compare elements to generate dynamic mask
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 dynamic_mask = mask & cmp;
    __m512i result = _mm512_mask_blend_epi16(dynamic_mask, a, b);
    v32hi_result = result;
    return result;
}

#ifdef __AVX512FP16__
__attribute__((noinline, target("avx512f,avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Mixed precision: convert from integer to half-precision
    __m512i int_vec = _mm512_set1_epi16(0x3C00); // 1.0 in half-precision
    __m512h one = _mm512_castsi512_ph(int_vec);
    
    // Create dynamic mask based on comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, one, _CMP_LT_OQ);
    __mmask32 blend_mask = mask ^ cmp_mask; // XOR for non-trivial mask
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    v32hf_result = result;
    return result;
}
#endif

#ifdef __AVX512BF16__
__attribute__((noinline, target("avx512f,avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use bfloat16 blend intrinsic with dynamic mask
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    v32bf_result = result;
    return result;
}
#endif

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Multi-stage: use result from previous blend as input
    __m512i temp = _mm512_add_epi32(a, b);
    __mmask16 dynamic_mask = mask & _mm512_cmplt_epi32_mask(a, temp);
    __m512i result = _mm512_mask_blend_epi32(dynamic_mask, a, b);
    v16si_result = result;
    return result;
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Type conversion before blending
    __m512d a_dbl = _mm512_cvtepi64_pd(a);
    __m512d b_dbl = _mm512_cvtepi64_pd(b);
    __mmask8 cmp_mask = _mm512_cmplt_pd_mask(a_dbl, b_dbl);
    __mmask8 blend_mask = mask | cmp_mask; // OR for non-trivial mask
    
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    v8di_result = result;
    return result;
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Complex data-dependent mask generation
    __m512d threshold = _mm512_set1_pd(0.5);
    __mmask8 lt_mask = _mm512_cmplt_pd_mask(a, threshold);
    __mmask8 gt_mask = _mm512_cmpgt_pd_mask(b, threshold);
    __mmask8 blend_mask = (mask & lt_mask) | (~mask & gt_mask);
    
    __m512d result = _mm512_mask_blend_pd(blend_mask, a, b);
    v8df_result = result;
    return result;
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Mixed operations to create complex dataflow
    __m512 abs_a = _mm512_abs_ps(a);
    __m512 abs_b = _mm512_abs_ps(b);
    __mmask16 abs_cmp = _mm512_cmplt_ps_mask(abs_a, abs_b);
    __mmask16 blend_mask = mask ^ abs_cmp; // XOR for non-trivial pattern
    
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline: blend results feed into next stage
__attribute__((noinline, target("avx512f,avx512bw")))
uint64_t pipeline_blend_operations(int seed) {
    uint64_t checksum = 0;
    
    // Initialize vectors with seed-dependent values
    __m512i v64qi_a = _mm512_set1_epi8(seed);
    __m512i v64qi_b = _mm512_set1_epi8(seed * 2);
    __mmask64 mask64 = (__mmask64)(seed | 0xAAAAAAAAAAAAAAAA);
    
    // Stage 1: V64QI blend
    __m512i blended_64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Convert to V32HI for next stage
    __m512i v32hi_a = _mm512_srai_epi16(blended_64qi, 2);
    __m512i v32hi_b = _mm512_set1_epi16(seed * 3);
    __mmask32 mask32 = (__mmask32)(seed | 0x55555555);
    
    // Stage 2: V32HI blend
    __m512i blended_32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Convert to V16SI for next stage
    __m512i v16si_a = _mm512_srai_epi32(blended_32hi, 4);
    __m512i v16si_b = _mm512_set1_epi32(seed * 5);
    __mmask16 mask16 = (__mmask16)(seed | 0xAAAA);
    
    // Stage 3: V16SI blend
    __m512i blended_16si = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Convert to V8DI for next stage
    __m512i v8di_a = _mm512_srai_epi64(blended_16si, 8);
    __m512i v8di_b = _mm512_set1_epi64(seed * 7);
    __mmask8 mask8 = (__mmask8)(seed | 0xAA);
    
    // Stage 4: V8DI blend
    __m512i blended_8di = blend_v8di(v8di_a, v8di_b, mask8);
    
    // Convert to floating point for final stages
    __m512d v8df_a = _mm512_cvtepi64_pd(blended_8di);
    __m512d v8df_b = _mm512_set1_pd(seed * 0.1);
    
    // Stage 5: V8DF blend
    __m512d blended_8df = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Convert to single precision
    __m512 v16sf_a = _mm512_cvtpd_ps(blended_8df);
    __m512 v16sf_b = _mm512_set1_ps(seed * 0.2f);
    
    // Stage 6: V16SF blend
    __m512 blended_16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Accumulate checksum
    checksum += _mm512_reduce_add_epi64(blended_64qi);
    checksum += _mm512_reduce_add_epi32(blended_32hi);
    checksum += _mm512_reduce_add_epi32(blended_16si);
    checksum += _mm512_reduce_add_epi64(blended_8di);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics with seed: %d\n", seed);
    
    // Initialize test data arrays
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)(seed + i);
    for (int i = 0; i < 32; i++) short_data[i] = (short)(seed * 2 + i);
    for (int i = 0; i < 16; i++) int_data[i] = seed * 3 + i;
    for (int i = 0; i < 8; i++) long_data[i] = seed * 5 + i;
    for (int i = 0; i < 16; i++) float_data[i] = (seed + i) * 0.1f;
    for (int i = 0; i < 8; i++) double_data[i] = (seed + i) * 0.2;
    
    // Load vectors from arrays (prevents constant folding)
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 16));
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_set1_epi64(seed * 11);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_set1_pd(seed * 0.3);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_set1_ps(seed * 0.4f);
    
    // Generate runtime masks (prevents optimization)
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) mask64 |= ((seed + i) % 3 == 0) << i;
    for (int i = 0; i < 32; i++) mask32 |= ((seed + i) % 5 == 0) << i;
    for (int i = 0; i < 16; i++) mask16 |= ((seed + i) % 7 == 0) << i;
    for (int i = 0; i < 8; i++) mask8 |= ((seed + i) % 11 == 0) << i;
    
    // Execute all blend functions
    blend_v64qi(v64qi_a, v64qi_b, mask64);
    blend_v32hi(v32hi_a, v32hi_b, mask32);
    
#ifdef __AVX512FP16__
    // Half-precision requires special initialization
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    blend_v32hf(v32hf_a, v32hf_b, mask32);
#endif
    
#ifdef __AVX512BF16__
    // Bfloat16 initialization
    __m512bh v32bf_a = _mm512_set1_ph(1.0f);
    __m512bh v32bf_b = _mm512_set1_ph(2.0f);
    blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
    
    blend_v16si(v16si_a, v16si_b, mask16);
    blend_v8di(v8di_a, v8di_b, mask8);
    blend_v8df(v8df_a, v8df_b, mask8);
    blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Execute multi-stage pipeline
    uint64_t checksum = pipeline_blend_operations(seed);
    
    printf("Checksum: %lu\n", checksum);
    printf("All blend operations executed.\n");
    
    return 0;
}
