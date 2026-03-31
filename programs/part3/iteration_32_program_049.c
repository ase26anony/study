#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
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

// Data-dependent mask generation functions
__attribute__((noinline))
__mmask64 generate_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((__mmask64)((i + seed) % 3 == 0) << i);
    }
    return mask;
}

__attribute__((noinline))
__mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((__mmask32)((i * seed) % 5 == 0) << i);
    }
    return mask;
}

__attribute__((noinline))
__mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((__mmask16)((i ^ seed) % 2 == 0) << i);
    }
    return mask;
}

__attribute__((noinline))
__mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((__mmask8)((i + seed * 7) % 3 != 0) << i);
    }
    return mask;
}

// Blend implementations - each forces specific RTL pattern
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force V64QImode blend
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional computation to prevent folding
    __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    result = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAA, result, temp);
    
    v64qi_result = result; // Volatile store
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Force V32HImode blend
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Multi-stage pipeline: blend -> arithmetic -> blend
    __m512i shifted = _mm512_slli_epi16(result, 1);
    result = _mm512_mask_blend_epi16(mask >> 1, result, shifted);
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Force V32HFmode blend
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision computation
    __m512h scaled = _mm512_mul_ph(result, _mm512_set1_ph(2.0f));
    result = _mm512_mask_blend_ph(mask ^ 0x55555555, result, scaled);
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Force V32BFmode blend (using bfloat16)
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Type conversion sequence
    __m512i as_int = _mm512_castps_si512(_mm512_castph_ps(result));
    __m512i shifted = _mm512_slli_epi32(as_int, 16);
    __m512bh reconverted = _mm512_castsi512_ph(shifted);
    result = _mm512_mask_blend_ph(mask & 0xAAAAAAAA, result, reconverted);
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Force V16SImode blend
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Conditional blending based on comparison
    __m512i cmp = _mm512_cmpgt_epi32_mask(result, _mm512_set1_epi32(0));
    result = _mm512_mask_blend_epi32(cmp, result, _mm512_abs_epi32(result));
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Force V8DImode blend
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Pipeline: blend -> conversion -> blend
    __m512d as_double = _mm512_cvtepi64_pd(result);
    __m512i back_to_int = _mm512_cvtpd_epi64(as_double);
    result = _mm512_mask_blend_epi64(mask | 0xAA, result, back_to_int);
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Force V8DFmode blend
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Data-dependent blending
    __mmask8 neg_mask = _mm512_cmp_pd_mask(result, _mm512_set1_pd(0.0), _CMP_LT_OQ);
    __m512d abs_vals = _mm512_abs_pd(result);
    result = _mm512_mask_blend_pd(neg_mask, result, abs_vals);
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Force V16SFmode blend
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Complex multi-stage pipeline
    __m512 recip = _mm512_rcp14_ps(result);
    __m512 blended_recip = _mm512_mask_blend_ps(mask ^ 0xFFFF, result, recip);
    __m512 corrected = _mm512_fmadd_ps(blended_recip, result, _mm512_set1_ps(1.0f));
    result = _mm512_mask_blend_ps(mask & 0xAAAA, blended_recip, corrected);
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline that connects different blend types
__attribute__((noinline))
uint64_t pipeline_blend_operations(int seed) {
    uint64_t checksum = 0;
    
    // Initialize vectors with seed-dependent values
    int8_t i8_data[64];
    int16_t i16_data[32];
    int32_t i32_data[16];
    int64_t i64_data[8];
    float f32_data[16];
    double f64_data[8];
    uint16_t f16_data[32];
    uint16_t bf16_data[32];
    
    for (int i = 0; i < 64; i++) i8_data[i] = (i + seed) % 256 - 128;
    for (int i = 0; i < 32; i++) i16_data[i] = (i * seed) % 65536 - 32768;
    for (int i = 0; i < 16; i++) i32_data[i] = (i ^ seed) * 123456789;
    for (int i = 0; i < 8; i++) i64_data[i] = ((int64_t)seed << 32) | i;
    for (int i = 0; i < 16; i++) f32_data[i] = (i - 8) * 0.125f * seed;
    for (int i = 0; i < 8; i++) f64_data[i] = (i - 4) * 0.25 * seed;
    for (int i = 0; i < 32; i++) f16_data[i] = (i % 16) * 0.1f * seed;
    for (int i = 0; i < 32; i++) bf16_data[i] = (i % 8) * 0.2f * seed;
    
    // Stage 1: V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512(i8_data);
    __m512i v64qi_b = _mm512_loadu_si512(i8_data + 32);
    __mmask64 mask64 = generate_mask64(seed);
    __m512i blended_64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Stage 2: V32HI blend (using packed results from stage 1)
    __m512i v32hi_a = _mm512_loadu_si512(i16_data);
    __m512i v32hi_b = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended_64qi, 0));
    __mmask32 mask32 = generate_mask32(seed);
    __m512i blended_32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Stage 3: V32HF blend
    __m512h v32hf_a = _mm512_loadu_ph(f16_data);
    __m512h v32hf_b = _mm512_loadu_ph(f16_data + 16);
    __m512h blended_32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    
    // Stage 4: V32BF blend
    __m512bh v32bf_a = _mm512_loadu_bh(bf16_data);
    __m512bh v32bf_b = _mm512_loadu_bh(bf16_data + 16);
    __m512bh blended_32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    
    // Stage 5: V16SI blend (using results from stage 2)
    __m512i v16si_a = _mm512_loadu_si512(i32_data);
    __m512i v16si_b = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(blended_32hi, 0));
    __mmask16 mask16 = generate_mask16(seed);
    __m512i blended_16si = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Stage 6: V8DI blend
    __m512i v8di_a = _mm512_loadu_si512(i64_data);
    __m512i v8di_b = _mm512_cvtepi32_epi64(_mm512_extracti64x4_epi64(blended_16si, 0));
    __mmask8 mask8 = generate_mask8(seed);
    __m512i blended_8di = blend_v8di(v8di_a, v8di_b, mask8);
    
    // Stage 7: V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(f64_data);
    __m512d v8df_b = _mm512_cvtepi64_pd(blended_8di);
    __m512d blended_8df = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Stage 8: V16SF blend
    __m512 v16sf_a = _mm512_loadu_ps(f32_data);
    __m512 v16sf_b = _mm512_cvtpd_ps(blended_8df);
    __m512 blended_16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum from all results
    int8_t* qi_ptr = (int8_t*)&blended_64qi;
    for (int i = 0; i < 64; i++) checksum += qi_ptr[i];
    
    int16_t* hi_ptr = (int16_t*)&blended_32hi;
    for (int i = 0; i < 32; i++) checksum += hi_ptr[i];
    
    float* sf_ptr = (float*)&blended_16sf;
    for (int i = 0; i < 16; i++) checksum += (uint64_t)sf_ptr[i];
    
    double* df_ptr = (double*)&blended_8df;
    for (int i = 0; i < 8; i++) checksum += (uint64_t)df_ptr[i];
    
    return checksum;
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Testing AVX-512 blend intrinsics with seed: %d\n", seed);
    
    // Run pipeline multiple times with different seeds
    uint64_t total_checksum = 0;
    for (int i = 0; i < 3; i++) {
        total_checksum += pipeline_blend_operations(seed + i);
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    
    // Force use of volatile results
    printf("Volatile results (prevent DCE):\n");
    printf("  v64qi: %016lx\n", ((uint64_t*)&v64qi_result)[0]);
    printf("  v32hi: %016lx\n", ((uint64_t*)&v32hi_result)[0]);
    printf("  v16si: %016lx\n", ((uint64_t*)&v16si_result)[0]);
    printf("  v8di:  %016lx\n", ((uint64_t*)&v8di_result)[0]);
    
    return 0;
}
