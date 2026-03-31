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

// Helper function to generate runtime-dependent masks
__attribute__((noinline))
unsigned long long generate_mask64(int seed) {
    unsigned long long mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((seed + i) % 3 == 0) ? (1ULL << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
unsigned int generate_mask32(int seed) {
    unsigned int mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((seed + i * 2) % 5 == 0) ? (1U << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
unsigned short generate_mask16(int seed) {
    unsigned short mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((seed + i * 3) % 7 == 0) ? (1U << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
unsigned char generate_mask8(int seed) {
    unsigned char mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((seed + i * 5) % 11 == 0) ? (1U << i) : 0;
    }
    return mask;
}

// V64QI blend function - triggers case E_V64QImode
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use volatile to prevent constant folding
    volatile __mmask64 vm = mask;
    
    // Data-dependent computation with comparison
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __mmask64 combined_mask = vm ^ cmp;
    
    // Force blend operation with runtime mask
    __m512i result = _mm512_mask_blend_epi8(combined_mask, a, b);
    
    // Additional dataflow to prevent optimization
    __m512i shifted = _mm512_slli_epi16(result, 1);
    result = _mm512_add_epi8(result, shifted);
    
    return result;
}

// V32HI blend function - triggers case E_V32HImode
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    volatile __mmask32 vm = mask;
    
    // Create predicate from comparison
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(a, b);
    __mmask32 final_mask = vm | cmp_mask;
    
    // Blend with runtime-dependent mask
    __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
    
    // Multi-stage operation
    __m512i scaled = _mm512_mullo_epi16(result, _mm512_set1_epi16(3));
    result = _mm512_add_epi16(result, scaled);
    
    return result;
}

// V32HF blend function - triggers case E_V32HFmode
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    volatile __mmask32 vm = mask;
    
    // Compare and create blend mask
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    __mmask32 blend_mask = vm & ~cmp_mask;
    
    // Half-precision blend
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Additional computation
    __m512h half = _mm512_set1_ph(0.5f);
    result = _mm512_mul_ph(result, half);
    
    return result;
}

// V32BF blend function - triggers case E_V32BFmode
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    volatile __mmask32 vm = mask;
    
    // For bfloat16, we need to use the same intrinsic as half-precision
    // but with bfloat16 types
    __mmask32 blend_mask = vm ^ 0xAAAAAAAA; // Pattern to ensure mixing
    
    // Blend bfloat16 vectors
    __m512bh result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    return result;
}

// V16SI blend function - triggers case E_V16SImode
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    volatile __mmask16 vm = mask;
    
    // Generate predicate from arithmetic
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    __mmask16 final_mask = vm ^ sign_mask;
    
    // Integer blend
    __m512i result = _mm512_mask_blend_epi32(final_mask, a, b);
    
    // Additional dataflow
    result = _mm512_mullo_epi32(result, _mm512_set1_epi32(2));
    
    return result;
}

// V8DI blend function - triggers case E_V8DImode
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    volatile __mmask8 vm = mask;
    
    // Create mask from comparison
    __mmask8 cmp_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 blend_mask = vm | cmp_mask;
    
    // 64-bit integer blend
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    
    // Shift operation to create more complex dataflow
    result = _mm512_slli_epi64(result, 1);
    
    return result;
}

// V8DF blend function - triggers case E_V8DFmode
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    volatile __mmask8 vm = mask;
    
    // Floating-point comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 final_mask = vm & cmp_mask;
    
    // Double-precision blend
    __m512d result = _mm512_mask_blend_pd(final_mask, a, b);
    
    // Additional computation
    __m512d sqrt_vals = _mm512_sqrt_pd(result);
    result = _mm512_add_pd(result, sqrt_vals);
    
    return result;
}

// V16SF blend function - triggers case E_V16SFmode
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    volatile __mmask16 vm = mask;
    
    // Generate blend mask from comparison
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    __mmask16 blend_mask = vm ^ cmp_mask;
    
    // Single-precision blend
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    
    // Additional floating operations
    __m512 recip = _mm512_rcp14_ps(result);
    result = _mm512_mul_ps(result, recip);
    
    return result;
}

// Multi-stage pipeline: process V64QI then convert to V16SI
__attribute__((noinline))
int pipeline_v64qi_to_v16si(int seed) {
    // Initialize data
    char data_a[64];
    char data_b[64];
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)(seed + i);
        data_b[i] = (char)(seed - i);
    }
    
    // Load as V64QI
    __m512i vec_a = _mm512_loadu_si512(data_a);
    __m512i vec_b = _mm512_loadu_si512(data_b);
    
    // Generate runtime mask
    __mmask64 mask64 = generate_mask64(seed);
    
    // First blend: V64QI
    __m512i blended_qi = blend_v64qi(vec_a, vec_b, mask64);
    
    // Store to volatile global
    g_v64qi_result = blended_qi;
    
    // Convert to V16SI by unpacking
    __m512i extended = _mm512_srai_epi16(_mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_qi, 0)), 4);
    __m512i vec_si_a = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(extended, 0));
    __m512i vec_si_b = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(extended, 1));
    
    // Generate new mask for V16SI
    __mmask16 mask16 = generate_mask16(seed);
    
    // Second blend: V16SI
    __m512i blended_si = blend_v16si(vec_si_a, vec_si_b, mask16);
    
    // Store to volatile global
    g_v16si_result = blended_si;
    
    // Compute checksum
    int sum = 0;
    int temp[16];
    _mm512_storeu_si512(temp, blended_si);
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    
    return sum;
}

// Mixed precision pipeline: float -> half -> bfloat
__attribute__((noinline))
float mixed_precision_pipeline(int seed) {
    // Start with single precision
    float float_data_a[16];
    float float_data_b[16];
    for (int i = 0; i < 16; i++) {
        float_data_a[i] = (seed + i) * 0.1f;
        float_data_b[i] = (seed - i) * 0.2f;
    }
    
    __m512 vec_f_a = _mm512_loadu_ps(float_data_a);
    __m512 vec_f_b = _mm512_loadu_ps(float_data_b);
    
    // Blend single precision
    __mmask16 mask16 = generate_mask16(seed);
    __m512 blended_f = blend_v16sf(vec_f_a, vec_f_b, mask16);
    g_v16sf_result = blended_f;
    
    // Convert to half precision
    __m512h vec_h_a = _mm512_cvtps_ph(blended_f, _MM_FROUND_TO_NEAREST_INT);
    __m512h vec_h_b = _mm512_cvtps_ph(_mm512_set1_ps(1.0f), _MM_FROUND_TO_NEAREST_INT);
    
    // Blend half precision
    __mmask32 mask32 = generate_mask32(seed);
    __m512h blended_h = blend_v32hf(vec_h_a, vec_h_b, mask32);
    g_v32hf_result = blended_h;
    
    // Convert back to float for checksum
    __m512 reconverted = _mm512_cvtph_ps(blended_h);
    
    // Horizontal add for checksum
    float sum = _mm512_reduce_add_ps(reconverted);
    
    return sum;
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc;
    
    // Initialize test data arrays
    unsigned char mask_data[64];
    for (int i = 0; i < 64; i++) {
        mask_data[i] = (unsigned char)((seed + i * 17) % 256);
    }
    
    // Test each blend function individually
    __m512i vec_i64 = _mm512_set1_epi8(seed);
    __m512i vec_i64_alt = _mm512_set1_epi8(seed ^ 0xFF);
    
    // 1. V64QI
    __mmask64 mask64 = generate_mask64(seed);
    __m512i result64qi = blend_v64qi(vec_i64, vec_i64_alt, mask64);
    g_v64qi_result = result64qi;
    
    // 2. V32HI
    __m512i vec_hi_a = _mm512_set1_epi16(seed);
    __m512i vec_hi_b = _mm512_set1_epi16(seed * 2);
    __mmask32 mask32 = generate_mask32(seed);
    __m512i result32hi = blend_v32hi(vec_hi_a, vec_hi_b, mask32);
    g_v32hi_result = result32hi;
    
    // 3. V32HF (if supported)
    #ifdef __AVX512FP16__
    __m512h vec_hf_a = _mm512_set1_ph(seed * 0.01f);
    __m512h vec_hf_b = _mm512_set1_ph(seed * 0.02f);
    __m512h result32hf = blend_v32hf(vec_hf_a, vec_hf_b, mask32);
    g_v32hf_result = result32hf;
    #endif
    
    // 4. V32BF (if supported)
    #ifdef __AVX512BF16__
    __m512bh vec_bf_a = _mm512_set1_bh(seed * 0.01f);
    __m512bh vec_bf_b = _mm512_set1_bh(seed * 0.02f);
    __m512bh result32bf = blend_v32bf(vec_bf_a, vec_bf_b, mask32);
    g_v32bf_result = result32bf;
    #endif
    
    // 5. V16SI
    __m512i vec_si_a = _mm512_set1_epi32(seed);
    __m512i vec_si_b = _mm512_set1_epi32(seed * 3);
    __mmask16 mask16 = generate_mask16(seed);
    __m512i result16si = blend_v16si(vec_si_a, vec_si_b, mask16);
    g_v16si_result = result16si;
    
    // 6. V8DI
    __m512i vec_di_a = _mm512_set1_epi64(seed);
    __m512i vec_di_b = _mm512_set1_epi64(seed * 5);
    __mmask8 mask8 = generate_mask8(seed);
    __m512i result8di = blend_v8di(vec_di_a, vec_di_b, mask8);
    g_v8di_result = result8di;
    
    // 7. V8DF
    __m512d vec_df_a = _mm512_set1_pd(seed * 0.001);
    __m512d vec_df_b = _mm512_set1_pd(seed * 0.002);
    __m512d result8df = blend_v8df(vec_df_a, vec_df_b, mask8);
    g_v8df_result = result8df;
    
    // 8. V16SF
    __m512 vec_sf_a = _mm512_set1_ps(seed * 0.01f);
    __m512 vec_sf_b = _mm512_set1_ps(seed * 0.02f);
    __m512 result16sf = blend_v16sf(vec_sf_a, vec_sf_b, mask16);
    g_v16sf_result = result16sf;
    
    // Execute multi-stage pipelines
    int pipeline_sum = pipeline_v64qi_to_v16si(seed);
    float mixed_sum = mixed_precision_pipeline(seed);
    
    // Compute final checksum
    long long checksum = 0;
    
    // Extract values from all results
    char temp64[64];
    _mm512_storeu_si512(temp64, result64qi);
    for (int i = 0; i < 64; i++) checksum += temp64[i];
    
    short temp32[32];
    _mm512_storeu_si512(temp32, result32hi);
    for (int i = 0; i < 32; i++) checksum += temp32[i];
    
    int temp16[16];
    _mm512_storeu_si512(temp16, result16si);
    for (int i = 0; i < 16; i++) checksum += temp16[i];
    
    long long temp8[8];
    _mm512_storeu_si512(temp8, result8di);
    for (int i = 0; i < 8; i++) checksum += temp8[i];
    
    checksum += pipeline_sum;
    checksum += (long long)mixed_sum;
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
