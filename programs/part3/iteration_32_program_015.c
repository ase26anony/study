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

// Multi-stage pipeline functions
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 mask64, __mmask16 mask16);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_half_to_float(__m512h halfs, __m512 floats, __mmask32 mask32, __mmask16 mask16);

// Data-dependent blending with runtime masks
__attribute__((noinline))
__m512i create_runtime_mask64(int seed) {
    volatile int vseed = seed; // Prevent constant propagation
    __mmask64 mask = 0;
    
    // Create pattern based on runtime value
    for (int i = 0; i < 64; i++) {
        if ((i + vseed) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    return _cvtu64_mask64(mask);
}

__attribute__((noinline))
__mmask32 create_runtime_mask32(int seed) {
    volatile int vseed = seed;
    __mmask32 mask = 0;
    
    for (int i = 0; i < 32; i++) {
        if ((i * vseed) % 5 == 0) {
            mask |= (1U << i);
        }
    }
    return _cvtu32_mask32(mask);
}

__attribute__((noinline))
__mmask16 create_runtime_mask16(int seed) {
    volatile int vseed = seed;
    __mmask16 mask = 0;
    
    for (int i = 0; i < 16; i++) {
        if ((i ^ vseed) % 2 == 0) {
            mask |= (1 << i);
        }
    }
    return _cvtu32_mask16(mask);
}

__attribute__((noinline))
__mmask8 create_runtime_mask8(int seed) {
    volatile int vseed = seed;
    __mmask8 mask = 0;
    
    for (int i = 0; i < 8; i++) {
        if ((i + vseed * 2) % 3 == 0) {
            mask |= (1 << i);
        }
    }
    return _cvtu32_mask8(mask);
}

// Blend function implementations
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion for V64QImode
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile to prevent elimination
    g_v64qi_result = result;
    
    // Additional computation to create data dependency
    return _mm512_add_epi8(result, _mm512_set1_epi8(1));
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Force RTL expansion for V32HImode
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    g_v32hi_result = result;
    
    // Create complex dataflow
    __m512i shifted = _mm512_slli_epi16(result, 1);
    return _mm512_xor_si512(result, shifted);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Force RTL expansion for V32HFmode
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    g_v32hf_result = result;
    
    // Mixed precision operation
    __m512h half_one = _mm512_set1_ph(1.0f);
    return _mm512_add_ph(result, half_one);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Force RTL expansion for V32BFmode
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    g_v32bf_result = result;
    
    // Convert to float and back to create dependencies
    __m512 floats = _mm512_cvtpbh_ps(result);
    __m512 scaled = _mm512_mul_ps(floats, _mm512_set1_ps(2.0f));
    return _mm512_cvtps_pbh(scaled);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Force RTL expansion for V16SImode
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    g_v16si_result = result;
    
    // Conditional operation based on blend result
    __m512i threshold = _mm512_set1_epi32(100);
    __mmask16 cmp_mask = _mm512_cmpgt_epi32_mask(result, threshold);
    return _mm512_mask_add_epi32(result, cmp_mask, result, _mm512_set1_epi32(10));
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Force RTL expansion for V8DImode
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    g_v8di_result = result;
    
    // Create data dependency chain
    __m512i shifted = _mm512_slli_epi64(result, 2);
    return _mm512_or_si512(result, shifted);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Force RTL expansion for V8DFmode
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    g_v8df_result = result;
    
    // Mathematical operation to prevent elimination
    __m512d sqrt_vals = _mm512_sqrt_pd(result);
    return _mm512_add_pd(result, sqrt_vals);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Force RTL expansion for V16SFmode
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    g_v16sf_result = result;
    
    // Complex floating point operation
    __m512 recip = _mm512_rcp14_ps(result);
    return _mm512_fmadd_ps(result, recip, _mm512_set1_ps(1.0f));
}

// Multi-stage pipeline: char -> int blending
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 mask64, __mmask16 mask16) {
    // Stage 1: Blend 64 chars (V64QI)
    __m512i blended_chars = _mm512_mask_blend_epi8(mask64, chars, 
        _mm512_add_epi8(chars, _mm512_set1_epi8(10)));
    
    // Convert blended chars to ints (creates V16SI vectors)
    __m512i chars_as_ints = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(blended_chars));
    
    // Stage 2: Blend 16 ints (V16SI) using result from stage 1
    __m512i blended_ints = _mm512_mask_blend_epi32(mask16, ints, chars_as_ints);
    
    // Horizontal sum for verification
    return _mm512_reduce_add_epi32(blended_ints);
}

// Multi-stage pipeline: half -> float blending
__attribute__((target("avx512f,avx512fp16")))
float pipeline_half_to_float(__m512h halfs, __m512 floats, __mmask32 mask32, __mmask16 mask16) {
    // Stage 1: Blend 32 half-precision (V32HF)
    __m512h blended_halfs = _mm512_mask_blend_ph(mask32, halfs,
        _mm512_add_ph(halfs, _mm512_set1_ph(2.0f)));
    
    // Convert to float (V16SF)
    __m512 halfs_as_floats = _mm512_cvtph_ps(blended_halfs);
    
    // Stage 2: Blend 16 floats (V16SF)
    __m512 blended_floats = _mm512_mask_blend_ps(mask16, floats, halfs_as_floats);
    
    // Horizontal sum
    return _mm512_reduce_add_ps(blended_floats);
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    // Initialize test data with seed-dependent values
    alignas(64) int8_t char_data[64];
    alignas(64) int16_t short_data[32];
    alignas(64) uint16_t half_data[32];
    alignas(64) uint16_t bf16_data[32];
    alignas(64) int32_t int_data[16];
    alignas(64) int64_t long_data[8];
    alignas(64) double double_data[8];
    alignas(64) float float_data[16];
    
    // Fill arrays with seed-dependent pseudo-random values
    for (int i = 0; i < 64; i++) char_data[i] = (i * seed) % 128;
    for (int i = 0; i < 32; i++) short_data[i] = (i * seed * 3) % 32767;
    for (int i = 0; i < 32; i++) half_data[i] = (i * seed + 12345) & 0x7FFF;
    for (int i = 0; i < 32; i++) bf16_data[i] = (i * seed * 7) & 0x7FFF;
    for (int i = 0; i < 16; i++) int_data[i] = i * seed * 11;
    for (int i = 0; i < 8; i++) long_data[i] = (int64_t)i * seed * 100;
    for (int i = 0; i < 8; i++) double_data[i] = (i + seed) * 0.5;
    for (int i = 0; i < 16; i++) float_data[i] = (i * seed) * 0.25f;
    
    // Create runtime masks
    __mmask64 mask64 = create_runtime_mask64(seed);
    __mmask32 mask32 = create_runtime_mask32(seed);
    __mmask16 mask16 = create_runtime_mask16(seed);
    __mmask8 mask8 = create_runtime_mask8(seed);
    
    // Load data into vectors
    __m512i v64qi_a = _mm512_load_si512(char_data);
    __m512i v64qi_b = _mm512_load_si512(char_data + 32);
    
    __m512i v32hi_a = _mm512_load_si512(short_data);
    __m512i v32hi_b = _mm512_load_si512(short_data + 16);
    
    __m512h v32hf_a = _mm512_load_ph(half_data);
    __m512h v32hf_b = _mm512_load_ph(half_data + 16);
    
    __m512bh v32bf_a = _mm512_load_ph(bf16_data);
    __m512bh v32bf_b = _mm512_load_ph(bf16_data + 16);
    
    __m512i v16si_a = _mm512_load_si512(int_data);
    __m512i v16si_b = _mm512_load_si512(int_data + 8);
    
    __m512i v8di_a = _mm512_load_si512(long_data);
    __m512i v8di_b = _mm512_load_si512(long_data + 4);
    
    __m512d v8df_a = _mm512_load_pd(double_data);
    __m512d v8df_b = _mm512_load_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_load_ps(float_data);
    __m512 v16sf_b = _mm512_load_ps(float_data + 8);
    
    // Execute all blend functions
    __m512i r64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h r32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    __m512bh r32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    __m512i r16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r8df = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 r16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Execute multi-stage pipelines
    uint64_t pipeline1_result = pipeline_char_to_int(v64qi_a, v16si_a, mask64, mask16);
    float pipeline2_result = pipeline_half_to_float(v32hf_a, v16sf_a, mask32, mask16);
    
    // Compute checksum from all results
    uint64_t checksum = 0;
    
    // Add contributions from each blended result
    alignas(64) int8_t temp64qi[64];
    alignas(64) int16_t temp32hi[32];
    alignas(64) uint16_t temp32hf[32];
    alignas(64) uint16_t temp32bf[32];
    alignas(64) int32_t temp16si[16];
    alignas(64) int64_t temp8di[8];
    alignas(64) double temp8df[8];
    alignas(64) float temp16sf[16];
    
    _mm512_store_si512(temp64qi, r64qi);
    _mm512_store_si512(temp32hi, r32hi);
    _mm512_store_ph(temp32hf, r32hf);
    _mm512_store_ph(temp32bf, r32bf);
    _mm512_store_si512(temp16si, r16si);
    _mm512_store_si512(temp8di, r8di);
    _mm512_store_pd(temp8df, r8df);
    _mm512_store_ps(temp16sf, r16sf);
    
    for (int i = 0; i < 64; i++) checksum += temp64qi[i];
    for (int i = 0; i < 32; i++) checksum += temp32hi[i];
    for (int i = 0; i < 32; i++) checksum += temp32hf[i];
    for (int i = 0; i < 32; i++) checksum += temp32bf[i];
    for (int i = 0; i < 16; i++) checksum += temp16si[i];
    for (int i = 0; i < 8; i++) checksum += temp8di[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)temp8df[i];
    for (int i = 0; i < 16; i++) checksum += (uint64_t)temp16sf[i];
    
    checksum += pipeline1_result;
    checksum += (uint64_t)pipeline2_result;
    
    printf("Final checksum: %lu\n", checksum);
    printf("All blend operations executed successfully.\n");
    
    return 0;
}
