#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Volatile globals to prevent optimization
volatile __m512i g_v64qi_result;
volatile __m512i g_v32hi_result;
volatile __m512h g_v32hf_result;
volatile __m512bh g_v32bf_result;
volatile __m512i g_v16si_result;
volatile __m512i g_v8di_result;
volatile __m512d g_v8df_result;
volatile __m512 g_v16sf_result;

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
    // Generate runtime mask by comparing elements
    __mmask64 mask = _mm512_cmpneq_epi8_mask(a, b);
    
    // Additional data-dependent manipulation
    __m512i shifted = _mm512_slli_epi16(a, 1);
    __mmask64 mask2 = _mm512_cmpeq_epi8_mask(shifted, b);
    
    // Combine masks for non-trivial pattern
    return _mm512_mask_blend_epi8(mask ^ mask2, a, b);
}

__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use runtime-derived mask to prevent constant folding
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    g_v64qi_result = result;
    
    // Additional data-dependent operation
    __m512i temp = _mm512_add_epi8(result, a);
    return _mm512_mask_blend_epi8(mask, temp, b);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Generate mask from comparison
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    
    // Blend with runtime mask
    __m512i result = _mm512_mask_blend_epi16(mask | cmp_mask, a, b);
    
    // Store to volatile
    g_v32hi_result = result;
    
    // Multi-stage: convert to 32-bit for next operation
    __m512i extended = _mm512_srai_epi32(_mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(result, 0)), 1);
    return _mm512_inserti64x4(result, _mm512_cvtepi32_epi16(extended), 0);
}

#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Half-precision blend
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store to volatile
    g_v32hf_result = result;
    
    // Mixed precision: convert to float, operate, convert back
    __m512 floats = _mm512_cvtph_ps(result);
    __m512 scaled = _mm512_mul_ps(floats, _mm512_set1_ps(2.0f));
    return _mm512_cvtps_ph(scaled, _MM_FROUND_CUR_DIRECTION);
}
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Brain float16 blend
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Store to volatile
    g_v32bf_result = result;
    
    // Convert to float for computation
    __m512 floats = _mm512_cvtpbh_ps(result);
    __m512 adjusted = _mm512_add_ps(floats, _mm512_set1_ps(1.0f));
    return _mm512_cvtne2ps_pbh(adjusted, adjusted);
}
#endif

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Generate complex mask pattern
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    
    // Blend with combined masks
    __m512i result = _mm512_mask_blend_epi32(mask ^ sign_mask, a, b);
    
    // Store to volatile
    g_v16si_result = result;
    
    // Feed into next stage: extract to 64-bit
    __m512i extended = _mm512_cvtepi32_epi64(_mm512_extracti32x8_epi32(result, 0));
    return _mm512_inserti32x8(result, _mm512_cvtepi64_epi32(extended), 0);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Create data-dependent mask
    __mmask8 parity_mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
    
    // Blend with runtime mask
    __m512i result = _mm512_mask_blend_epi64(mask & parity_mask, a, b);
    
    // Store to volatile
    g_v8di_result = result;
    
    // Convert to double for mixed-type pipeline
    __m512d doubles = _mm512_cvtepi64_pd(result);
    __m512d scaled = _mm512_mul_pd(doubles, _mm512_set1_pd(0.5));
    return _mm512_cvtpd_epi64(scaled);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Generate mask from comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    // Blend with combined masks
    __m512d result = _mm512_mask_blend_pd(mask | cmp_mask, a, b);
    
    // Store to volatile
    g_v8df_result = result;
    
    // Convert to integer and back for type stress
    __m512i ints = _mm512_cvtpd_epi64(result);
    __m512d reconverted = _mm512_cvtepi64_pd(ints);
    return _mm512_mask_blend_pd(mask, result, reconverted);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Complex mask generation
    __m512 abs_a = _mm512_abs_ps(a);
    __m512 abs_b = _mm512_abs_ps(b);
    __mmask16 abs_mask = _mm512_cmp_ps_mask(abs_a, abs_b, _CMP_GT_OQ);
    
    // Blend with runtime mask
    __m512 result = _mm512_mask_blend_ps(mask ^ abs_mask, a, b);
    
    // Store to volatile
    g_v16sf_result = result;
    
    // Convert to integer for mixed-type operation
    __m512i ints = _mm512_cvtps_epi32(result);
    __m512 floats = _mm512_cvtepi32_ps(ints);
    return _mm512_mask_blend_ps(mask, result, floats);
}

// Multi-stage pipeline combining different blend types
__attribute__((target("avx512f,avx512bw")))
float multi_stage_pipeline(int seed) {
    // Initialize with seed-dependent values
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)((i * seed) & 0xFF);
    for (int i = 0; i < 32; i++) short_data[i] = (short)((i * seed * 3) & 0xFFFF);
    for (int i = 0; i < 16; i++) int_data[i] = i * seed * 5;
    for (int i = 0; i < 8; i++) long_data[i] = (long long)i * seed * 7;
    for (int i = 0; i < 16; i++) float_data[i] = (float)(i * seed * 0.1f);
    for (int i = 0; i < 8; i++) double_data[i] = (double)(i * seed * 0.01);
    
    // Stage 1: V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_set1_epi8(seed);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((char_data[i] & 1) == (seed & 1)) {
            mask64 |= (1ULL << i);
        }
    }
    __m512i v64qi_result = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Stage 2: V32HI blend (using packed result from stage 1)
    __m512i v32hi_a = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(v64qi_result, 0));
    __m512i v32hi_b = _mm512_set1_epi16(seed * 2);
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 3 == 0) {
            mask32 |= (1U << i);
        }
    }
    __m512i v32hi_result = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Stage 3: V16SI blend
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_set1_epi32(seed * 3);
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if ((int_data[i] % 5) == (seed % 5)) {
            mask16 |= (1 << i);
        }
    }
    __m512i v16si_result = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Stage 4: V8DI blend
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_set1_epi64(seed * 4);
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        if ((long_data[i] & 3) == (seed & 3)) {
            mask8 |= (1 << i);
        }
    }
    __m512i v8di_result = blend_v8di(v8di_a, v8di_b, mask8);
    
    // Stage 5: V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_set1_pd(seed * 0.5);
    __m512d v8df_result = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Stage 6: V16SF blend
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_set1_ps(seed * 0.25f);
    __m512 v16sf_result = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum from all results
    float checksum = 0.0f;
    
    // Horizontal add for v64qi
    __m256i v64qi_sum = _mm256_sad_epu8(
        _mm512_extracti64x4_epi64(v64qi_result, 0),
        _mm512_extracti64x4_epi64(v64qi_result, 1)
    );
    checksum += (float)_mm256_extract_epi64(v64qi_sum, 0);
    checksum += (float)_mm256_extract_epi64(v64qi_sum, 1);
    checksum += (float)_mm256_extract_epi64(v64qi_sum, 2);
    checksum += (float)_mm256_extract_epi64(v64qi_sum, 3);
    
    // Horizontal add for v32hi
    __m512i v32hi_sum = _mm512_madd_epi16(v32hi_result, _mm512_set1_epi16(1));
    checksum += (float)_mm512_reduce_add_epi32(v32hi_sum);
    
    // Horizontal add for v16si
    checksum += (float)_mm512_reduce_add_epi32(v16si_result);
    
    // Horizontal add for v8di
    checksum += (float)_mm512_reduce_add_epi64(v8di_result);
    
    // Horizontal add for v8df
    checksum += (float)_mm512_reduce_add_pd(v8df_result);
    
    // Horizontal add for v16sf
    checksum += _mm512_reduce_add_ps(v16sf_result);
    
    return checksum;
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics with seed: %d\n", seed);
    
    float checksum = multi_stage_pipeline(seed);
    
    printf("Final checksum: %f\n", checksum);
    
    // Force use of half-precision blends if supported
#ifdef __AVX512FP16__
    {
        __m512h v32hf_a = _mm512_set1_ph(1.0f);
        __m512h v32hf_b = _mm512_set1_ph(2.0f);
        __mmask32 mask32 = 0xAAAAAAAA; // Alternating pattern
        __m512h v32hf_result = blend_v32hf(v32hf_a, v32hf_b, mask32);
        
        // Convert to float for checksum
        __m512 floats = _mm512_cvtph_ps(v32hf_result);
        checksum += _mm512_reduce_add_ps(floats);
    }
#endif
    
#ifdef __AVX512BF16__
    {
        __m512bh v32bf_a = _mm512_set1_bh(1.0f);
        __m512bh v32bf_b = _mm512_set1_bh(2.0f);
        __mmask32 mask32 = 0x55555555; // Alternating pattern (complement)
        __m512bh v32bf_result = blend_v32bf(v32bf_a, v32bf_b, mask32);
        
        // Convert to float for checksum
        __m512 floats = _mm512_cvtpbh_ps(v32bf_result);
        checksum += _mm512_reduce_add_ps(floats);
    }
#endif
    
    printf("Final checksum with FP16/BF16: %f\n", checksum);
    
    return 0;
}
