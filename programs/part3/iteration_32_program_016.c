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

// Function to generate runtime-dependent mask
static inline __mmask64 get_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((__mmask64)((seed + i) % 3 == 0) << i);
    }
    return mask;
}

static inline __mmask32 get_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((__mmask32)((seed + i) % 5 == 0) << i);
    }
    return mask;
}

static inline __mmask16 get_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((__mmask16)((seed + i) % 7 == 0) << i);
    }
    return mask;
}

static inline __mmask8 get_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((__mmask8)((seed + i) % 11 == 0) << i);
    }
    return mask;
}

// Separate functions for each blend intrinsic to force RTL expansion
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, int seed) {
    __mmask64 mask = get_mask64(seed);
    // Force RTL expansion for V64QImode
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    v64qi_result = result;  // Volatile store prevents elimination
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, int seed) {
    __mmask32 mask = get_mask32(seed);
    // Force RTL expansion for V32HImode
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, int seed) {
    __mmask32 mask = get_mask32(seed);
    // Force RTL expansion for V32HFmode
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bf16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, int seed) {
    __mmask32 mask = get_mask32(seed);
    // Force RTL expansion for V32BFmode
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, int seed) {
    __mmask16 mask = get_mask16(seed);
    // Force RTL expansion for V16SImode
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, int seed) {
    __mmask8 mask = get_mask8(seed);
    // Force RTL expansion for V8DImode
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, int seed) {
    __mmask8 mask = get_mask8(seed);
    // Force RTL expansion for V8DFmode
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, int seed) {
    __mmask16 mask = get_mask16(seed);
    // Force RTL expansion for V16SFmode
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline: process data through different blend types
__attribute__((target("avx512f,avx512bw")))
uint64_t process_pipeline(int seed) {
    uint64_t checksum = 0;
    
    // Stage 1: V64QI blend
    int8_t data64qi_a[64], data64qi_b[64];
    for (int i = 0; i < 64; i++) {
        data64qi_a[i] = (int8_t)((seed + i) % 256);
        data64qi_b[i] = (int8_t)((seed + i * 3) % 256);
    }
    __m512i v64qi_a = _mm512_loadu_si512(data64qi_a);
    __m512i v64qi_b = _mm512_loadu_si512(data64qi_b);
    __m512i blended_qi = blend_v64qi(v64qi_a, v64qi_b, seed);
    
    // Stage 2: Convert V64QI to V32HI and blend
    __m512i v32hi_a = _mm512_srai_epi16(blended_qi, 4);  // Create different data
    __m512i v32hi_b = _mm512_add_epi16(v32hi_a, _mm512_set1_epi16(1));
    __m512i blended_hi = blend_v32hi(v32hi_a, v32hi_b, seed + 1);
    
    // Stage 3: Convert V32HI to V16SI and blend
    __m512i v16si_a = _mm512_srai_epi32(blended_hi, 8);
    __m512i v16si_b = _mm512_add_epi32(v16si_a, _mm512_set1_epi32(2));
    __m512i blended_si = blend_v16si(v16si_a, v16si_b, seed + 2);
    
    // Stage 4: Convert V16SI to V8DI and blend
    __m512i v8di_a = _mm512_srai_epi64(blended_si, 16);
    __m512i v8di_b = _mm512_add_epi64(v8di_a, _mm512_set1_epi64(3));
    __m512i blended_di = blend_v8di(v8di_a, v8di_b, seed + 3);
    
    // Stage 5: Integer to float conversion and blend
    __m512 v16sf_a = _mm512_cvtepi32_ps(_mm512_extracti32x8_epi32(blended_si, 0));
    __m512 v16sf_b = _mm512_cvtepi32_ps(_mm512_extracti32x8_epi32(blended_si, 1));
    __m512 blended_sf = blend_v16sf(v16sf_a, v16sf_b, seed + 4);
    
    // Stage 6: Float to double conversion and blend
    __m512d v8df_a = _mm512_cvtps_pd(_mm512_extractf32x8_ps(blended_sf, 0));
    __m512d v8df_b = _mm512_cvtps_pd(_mm512_extractf32x8_ps(blended_sf, 1));
    __m512d blended_df = blend_v8df(v8df_a, v8df_b, seed + 5);
    
    // Accumulate checksum from all results
    int8_t* qi_ptr = (int8_t*)&blended_qi;
    for (int i = 0; i < 64; i++) checksum += qi_ptr[i];
    
    int16_t* hi_ptr = (int16_t*)&blended_hi;
    for (int i = 0; i < 32; i++) checksum += hi_ptr[i];
    
    int32_t* si_ptr = (int32_t*)&blended_si;
    for (int i = 0; i < 16; i++) checksum += si_ptr[i];
    
    int64_t* di_ptr = (int64_t*)&blended_di;
    for (int i = 0; i < 8; i++) checksum += di_ptr[i];
    
    float* sf_ptr = (float*)&blended_sf;
    for (int i = 0; i < 16; i++) checksum += (uint64_t)sf_ptr[i];
    
    double* df_ptr = (double*)&blended_df;
    for (int i = 0; i < 8; i++) checksum += (uint64_t)df_ptr[i];
    
    return checksum;
}

// Half-precision specific pipeline
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16,avx512bw")))
uint64_t process_half_precision(int seed) {
    uint64_t checksum = 0;
    
    // Generate half-precision data
    _Float16 data32hf_a[32], data32hf_b[32];
    for (int i = 0; i < 32; i++) {
        data32hf_a[i] = (_Float16)((seed + i) % 100) / 10.0f;
        data32hf_b[i] = (_Float16)((seed + i * 2) % 100) / 10.0f;
    }
    
    __m512h v32hf_a = _mm512_loadu_ph(data32hf_a);
    __m512h v32hf_b = _mm512_loadu_ph(data32hf_b);
    
    // Blend half-precision
    __m512h blended_hf = blend_v32hf(v32hf_a, v32hf_b, seed + 10);
    
    // Store to volatile
    _Float16* hf_ptr = (_Float16*)&blended_hf;
    for (int i = 0; i < 32; i++) checksum += (uint64_t)hf_ptr[i];
    
    return checksum;
}
#endif

// Bfloat16 specific pipeline
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16,avx512bw")))
uint64_t process_bfloat16(int seed) {
    uint64_t checksum = 0;
    
    // Generate bfloat16 data
    __bf16 data32bf_a[32], data32bf_b[32];
    for (int i = 0; i < 32; i++) {
        uint16_t val = (seed + i) % 65535;
        data32bf_a[i] = *(__bf16*)&val;
        val = (seed + i * 3) % 65535;
        data32bf_b[i] = *(__bf16*)&val;
    }
    
    __m512bh v32bf_a = _mm512_loadu_si512(data32bf_a);
    __m512bh v32bf_b = _mm512_loadu_si512(data32bf_b);
    
    // Blend bfloat16
    __m512bh blended_bf = blend_v32bf(v32bf_a, v32bf_b, seed + 20);
    
    // Store to volatile
    __bf16* bf_ptr = (__bf16*)&blended_bf;
    for (int i = 0; i < 32; i++) checksum += (*(uint16_t*)&bf_ptr[i]);
    
    return checksum;
}
#endif

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics with seed: %d\n", seed);
    
    uint64_t total_checksum = 0;
    
    // Execute main pipeline
    total_checksum += process_pipeline(seed);
    
    // Execute half-precision pipeline if available
#ifdef __AVX512FP16__
    total_checksum += process_half_precision(seed);
#endif
    
    // Execute bfloat16 pipeline if available
#ifdef __AVX512BF16__
    total_checksum += process_bfloat16(seed);
#endif
    
    // Additional data-dependent blending with comparisons
    {
        // Create vectors with runtime-dependent data
        float data_a[16], data_b[16];
        for (int i = 0; i < 16; i++) {
            data_a[i] = (float)((seed + i) % 100) / 10.0f;
            data_b[i] = (float)((seed + i * 2) % 100) / 10.0f;
        }
        
        __m512 vec_a = _mm512_loadu_ps(data_a);
        __m512 vec_b = _mm512_loadu_ps(data_b);
        
        // Generate mask from comparison (data-dependent)
        __mmask16 cmp_mask = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_GT_OQ);
        
        // Use the comparison mask for blending
        __m512 blended = _mm512_mask_blend_ps(cmp_mask, vec_a, vec_b);
        
        // Force materialization
        float result[16];
        _mm512_storeu_ps(result, blended);
        
        for (int i = 0; i < 16; i++) {
            total_checksum += (uint64_t)result[i];
        }
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}
