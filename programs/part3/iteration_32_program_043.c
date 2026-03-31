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

// Multi-stage pipeline functions
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 char_mask, __mmask16 int_mask);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_half_to_float(__m512h halfs, __m512 floats, __mmask32 half_mask, __mmask16 float_mask);

// Implementation of blend functions
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Data-dependent computation: conditional increment based on blend result
    __mmask64 nonzero = _mm512_cmpneq_epi8_mask(result, _mm512_setzero_si512());
    result = _mm512_mask_add_epi8(result, nonzero, result, _mm512_set1_epi8(1));
    
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Use computed mask from vector comparison
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Additional dataflow: scale blended values
    __mmask32 gt_mask = _mm512_cmpgt_epi16_mask(result, _mm512_set1_epi16(100));
    result = _mm512_mask_mullo_epi16(result, gt_mask, result, _mm512_set1_epi16(2));
    
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Half-precision blend
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision: convert to float, operate, convert back
    __m512 temp_f = _mm512_cvtph_ps(result);
    __m512 scaled = _mm512_mul_ps(temp_f, _mm512_set1_ps(1.5f));
    result = _mm512_cvtps_ph(scaled, _MM_FROUND_CUR_DIRECTION);
    
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Brain float16 blend
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float for computation
    __m512 temp_f = _mm512_cvtpbh_ps(result);
    __m512 adjusted = _mm512_add_ps(temp_f, _mm512_set1_ps(0.1f));
    
    // Store to volatile to force materialization
    v32bf_result = result;
    
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // 32-bit integer blend
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Complex dataflow: blend -> multiply -> conditional select
    __m512i squared = _mm512_mullo_epi32(result, result);
    __mmask16 overflow = _mm512_cmpgt_epi32_mask(squared, _mm512_set1_epi32(1000000));
    result = _mm512_mask_blend_epi32(overflow, result, _mm512_set1_epi32(0));
    
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // 64-bit integer blend
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Use in reduction
    __m512i shifted = _mm512_slli_epi64(result, 1);
    result = _mm512_or_epi64(result, shifted);
    
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Double precision blend
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Mathematical operation on blended result
    __m512d abs_result = _mm512_abs_pd(result);
    __m512d log_result = _mm512_log_pd(_mm512_add_pd(abs_result, _mm512_set1_pd(1.0)));
    
    return log_result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Single precision blend
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Trigonometric operation chain
    __m512 sin_result = _mm512_sin_ps(result);
    __m512 cos_result = _mm512_cos_ps(result);
    result = _mm512_add_ps(sin_result, cos_result);
    
    return result;
}

// Multi-stage pipeline: char -> int
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 char_mask, __mmask16 int_mask) {
    // Stage 1: Blend chars (V64QI)
    __m512i blended_chars = _mm512_mask_blend_epi8(char_mask, chars, 
                                                  _mm512_add_epi8(chars, _mm512_set1_epi8(10)));
    
    // Convert to 32-bit integers (creates V16SI)
    __m512i ints_from_chars = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(blended_chars));
    
    // Stage 2: Blend ints (V16SI)
    __m512i blended_ints = _mm512_mask_blend_epi32(int_mask, ints, ints_from_chars);
    
    // Horizontal reduction
    return _mm512_reduce_add_epi32(blended_ints);
}

// Multi-stage pipeline: half -> float
__attribute__((target("avx512f,avx512fp16")))
float pipeline_half_to_float(__m512h halfs, __m512 floats, __mmask32 half_mask, __mmask16 float_mask) {
    // Stage 1: Blend half precision (V32HF)
    __m512h blended_halfs = _mm512_mask_blend_ph(half_mask, halfs, 
                                                _mm512_castps_ph(_mm512_set1_ps(2.0f)));
    
    // Convert to float (V16SF)
    __m512 floats_from_halfs = _mm512_cvtph_ps(blended_halfs);
    
    // Stage 2: Blend floats (V16SF)
    __m512 blended_floats = _mm512_mask_blend_ps(float_mask, floats, floats_from_halfs);
    
    // Horizontal reduction
    return _mm512_reduce_add_ps(blended_floats);
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic behavior
    unsigned int seed = (unsigned int)argc;
    
    // Initialize test data arrays
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t half_data[32];
    uint16_t bfloat_data[32];
    
    // Fill with pseudo-random values based on seed
    for (int i = 0; i < 64; i++) char_data[i] = (char)((seed + i * 13) % 256 - 128);
    for (int i = 0; i < 32; i++) short_data[i] = (short)((seed + i * 17) % 65536 - 32768);
    for (int i = 0; i < 16; i++) int_data[i] = (int)((seed + i * 19) * 1103515245);
    for (int i = 0; i < 8; i++) long_data[i] = (long long)((seed + i * 23) * 1103515245);
    for (int i = 0; i < 16; i++) float_data[i] = (float)((seed + i * 29) % 100) / 10.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)((seed + i * 31) % 100) / 10.0;
    
    // Initialize half-precision data
    for (int i = 0; i < 32; i++) {
        float temp = (float)((seed + i * 37) % 100) / 10.0f;
        half_data[i] = _cvtss_sh(temp, 0);
        bfloat_data[i] = _cvtsh_bf16(_cvtss_sh(temp, 0));
    }
    
    // Load into vectors
    __m512i v_char1 = _mm512_loadu_si512(char_data);
    __m512i v_char2 = _mm512_loadu_si512(char_data + 32);
    
    __m512i v_short1 = _mm512_loadu_si512(short_data);
    __m512i v_short2 = _mm512_loadu_si512(short_data + 16);
    
    __m512i v_int1 = _mm512_loadu_si512(int_data);
    __m512i v_int2 = _mm512_loadu_si512(int_data + 8);
    
    __m512i v_long1 = _mm512_loadu_si512(long_data);
    __m512i v_long2 = _mm512_loadu_si512(long_data + 4);
    
    __m512 v_float1 = _mm512_loadu_ps(float_data);
    __m512 v_float2 = _mm512_loadu_ps(float_data + 8);
    
    __m512d v_double1 = _mm512_loadu_pd(double_data);
    __m512d v_double2 = _mm512_loadu_pd(double_data + 4);
    
    __m512h v_half1, v_half2;
    memcpy(&v_half1, half_data, 64);
    memcpy(&v_half2, half_data + 16, 64);
    
    __m512bh v_bfloat1, v_bfloat2;
    memcpy(&v_bfloat1, bfloat_data, 64);
    memcpy(&v_bfloat2, bfloat_data + 16, 64);
    
    // Generate runtime masks (prevents constant folding)
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((seed + i) % 3 == 0) mask64 |= (1ULL << i);
    }
    for (int i = 0; i < 32; i++) {
        if ((seed + i) % 4 == 0) mask32 |= (1U << i);
    }
    for (int i = 0; i < 16; i++) {
        if ((seed + i) % 5 == 0) mask16 |= (1U << i);
    }
    for (int i = 0; i < 8; i++) {
        if ((seed + i) % 6 == 0) mask8 |= (1U << i);
    }
    
    // Call all blend functions
    __m512i blended_chars = blend_v64qi(v_char1, v_char2, mask64);
    __m512i blended_shorts = blend_v32hi(v_short1, v_short2, mask32);
    __m512h blended_halfs = blend_v32hf(v_half1, v_half2, mask32);
    __m512bh blended_bfloats = blend_v32bf(v_bfloat1, v_bfloat2, mask32);
    __m512i blended_ints = blend_v16si(v_int1, v_int2, mask16);
    __m512i blended_longs = blend_v8di(v_long1, v_long2, mask8);
    __m512d blended_doubles = blend_v8df(v_double1, v_double2, mask8);
    __m512 blended_floats = blend_v16sf(v_float1, v_float2, mask16);
    
    // Store to volatile to prevent elimination
    v64qi_result = blended_chars;
    v32hi_result = blended_shorts;
    v32hf_result = blended_halfs;
    v32bf_result = blended_bfloats;
    v16si_result = blended_ints;
    v8di_result = blended_longs;
    v8df_result = blended_doubles;
    v16sf_result = blended_floats;
    
    // Execute pipeline functions
    int64_t char_int_result = pipeline_char_to_int(v_char1, v_int1, mask64, mask16);
    float half_float_result = pipeline_half_to_float(v_half1, v_float1, mask32, mask16);
    
    // Compute checksum
    int64_t checksum = 0;
    checksum += _mm512_reduce_add_epi64(blended_chars);
    checksum += _mm512_reduce_add_epi32(blended_shorts);
    checksum += (int64_t)_mm512_reduce_add_ph(blended_halfs);
    checksum += (int64_t)_mm512_reduce_add_ph(blended_bfloats);
    checksum += _mm512_reduce_add_epi32(blended_ints);
    checksum += _mm512_reduce_add_epi64(blended_longs);
    checksum += (int64_t)_mm512_reduce_add_pd(blended_doubles);
    checksum += (int64_t)_mm512_reduce_add_ps(blended_floats);
    checksum += char_int_result;
    checksum += (int64_t)half_float_result;
    
    printf("Checksum: %ld\n", checksum);
    printf("All AVX-512 blend operations executed.\n");
    
    return 0;
}
