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

// Prevent inlining to force separate RTL expansion
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use runtime-derived mask to prevent constant folding
    __mmask64 dynamic_mask = mask ^ 0xAAAAAAAAAAAAAAAAULL;
    return _mm512_mask_blend_epi8(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Complex mask computation to prevent folding
    __mmask32 dynamic_mask = (mask * 1103515245 + 12345) & 0xFFFFFFFF;
    return _mm512_mask_blend_epi16(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Data-dependent mask from vector comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    __mmask32 dynamic_mask = mask & cmp_mask;
    return _mm512_mask_blend_ph(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512bf16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use _Float16 for computation, convert to bfloat16
    __m512h fa = _mm512_cvtpbh_ps(a);
    __m512h fb = _mm512_cvtpbh_ps(b);
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(fa, fb, _CMP_GT_OQ);
    __mmask32 dynamic_mask = mask | cmp_mask;
    return _mm512_mask_blend_ph(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Generate mask from vector data
    __m512i cmp = _mm512_cmpgt_epi32_mask(a, b);
    __mmask16 dynamic_mask = mask ^ cmp;
    return _mm512_mask_blend_epi32(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Complex mask with arithmetic
    __mmask8 dynamic_mask = (mask * 7 + 3) & 0xFF;
    return _mm512_mask_blend_epi64(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Mask from floating comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
    __mmask8 dynamic_mask = mask & ~cmp_mask;
    return _mm512_mask_blend_pd(dynamic_mask, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Multi-stage mask computation
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, b, _CMP_LE_OQ);
    __mmask16 dynamic_mask = (mask << 1) | cmp_mask;
    return _mm512_mask_blend_ps(dynamic_mask, a, b);
}

// Multi-stage pipeline: V64QI -> V16SI -> V8DF
__attribute__((noinline, target("avx512f,avx512bw")))
double pipeline_blend(int seed) {
    // Stage 1: V64QI blending
    char data1[64], data2[64];
    for (int i = 0; i < 64; i++) {
        data1[i] = (char)((i * seed) & 0xFF);
        data2[i] = (char)((i * seed + 1) & 0xFF);
    }
    
    __m512i v64qi_a = _mm512_loadu_si512(data1);
    __m512i v64qi_b = _mm512_loadu_si512(data2);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((i * seed) % 3 == 0) mask64 |= (1ULL << i);
    }
    
    __m512i v64qi_res = blend_v64qi(v64qi_a, v64qi_b, mask64);
    v64qi_result = v64qi_res;  // Volatile store
    
    // Stage 2: Convert to V16SI and blend
    __m512i v16si_a = _mm512_cvtepi8_epi32(_mm512_extracti64x4_epi64(v64qi_res, 0));
    __m512i v16si_b = _mm512_cvtepi8_epi32(_mm512_extracti64x4_epi64(v64qi_res, 1));
    
    __mmask16 mask16 = (seed * 1103515245 + 12345) & 0xFFFF;
    __m512i v16si_res = blend_v16si(v16si_a, v16si_b, mask16);
    v16si_result = v16si_res;  // Volatile store
    
    // Stage 3: Convert to V8DF and blend
    __m512d v8df_a = _mm512_cvtepi32_pd(_mm512_extracti32x8_epi32(v16si_res, 0));
    __m512d v8df_b = _mm512_cvtepi32_pd(_mm512_extracti32x8_epi32(v16si_res, 1));
    
    __mmask8 mask8 = seed & 0xFF;
    __m512d v8df_res = blend_v8df(v8df_a, v8df_b, mask8);
    v8df_result = v8df_res;  // Volatile store
    
    // Horizontal sum
    return _mm512_reduce_add_pd(v8df_res);
}

int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    // Initialize test data with seed-dependent values
    int8_t i8_data1[64], i8_data2[64];
    int16_t i16_data1[32], i16_data2[32];
    _Float16 f16_data1[32], f16_data2[32];
    __bf16 bf16_data1[32], bf16_data2[32];
    int32_t i32_data1[16], i32_data2[16];
    int64_t i64_data1[8], i64_data2[8];
    double f64_data1[8], f64_data2[8];
    float f32_data1[16], f32_data2[16];
    
    for (int i = 0; i < 64; i++) {
        i8_data1[i] = (i * seed) & 0x7F;
        i8_data2[i] = (i * seed + 64) & 0x7F;
        if (i < 32) {
            i16_data1[i] = (i * seed * 257) & 0x7FFF;
            i16_data2[i] = (i * seed * 257 + 32) & 0x7FFF;
            f16_data1[i] = (_Float16)(i * seed * 0.1f);
            f16_data2[i] = (_Float16)(i * seed * 0.1f + 0.5f);
            bf16_data1[i] = (__bf16)(i * seed * 0.1f);
            bf16_data2[i] = (__bf16)(i * seed * 0.1f + 0.5f);
        }
        if (i < 16) {
            i32_data1[i] = i * seed * 65537;
            i32_data2[i] = i * seed * 65537 + 16;
            f32_data1[i] = i * seed * 0.01f;
            f32_data2[i] = i * seed * 0.01f + 0.25f;
        }
        if (i < 8) {
            i64_data1[i] = (int64_t)i * seed * 4294967296LL;
            i64_data2[i] = (int64_t)i * seed * 4294967296LL + 8;
            f64_data1[i] = i * seed * 0.001;
            f64_data2[i] = i * seed * 0.001 + 0.125;
        }
    }
    
    double checksum = 0.0;
    
    // Test V64QI
    __m512i v64qi_a = _mm512_loadu_si512(i8_data1);
    __m512i v64qi_b = _mm512_loadu_si512(i8_data2);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 2 == 0) mask64 |= (1ULL << i);
    }
    __m512i v64qi_res = blend_v64qi(v64qi_a, v64qi_b, mask64);
    v64qi_result = v64qi_res;
    
    // Test V32HI
    __m512i v32hi_a = _mm512_loadu_si512(i16_data1);
    __m512i v32hi_b = _mm512_loadu_si512(i16_data2);
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 3 == 0) mask32 |= (1U << i);
    }
    __m512i v32hi_res = blend_v32hi(v32hi_a, v32hi_b, mask32);
    v32hi_result = v32hi_res;
    
    // Test V32HF
    __m512h v32hf_a = _mm512_loadu_ph(f16_data1);
    __m512h v32hf_b = _mm512_loadu_ph(f16_data2);
    __mmask32 mask32_hf = (seed * 1664525 + 1013904223) & 0xFFFFFFFF;
    __m512h v32hf_res = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
    v32hf_result = v32hf_res;
    
    // Test V32BF
    __m512bh v32bf_a = _mm512_loadu_si512(bf16_data1);
    __m512bh v32bf_b = _mm512_loadu_si512(bf16_data2);
    __mmask32 mask32_bf = ~mask32_hf;
    __m512bh v32bf_res = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
    v32bf_result = v32bf_res;
    
    // Test V16SI
    __m512i v16si_a = _mm512_loadu_si512(i32_data1);
    __m512i v16si_b = _mm512_loadu_si512(i32_data2);
    __mmask16 mask16 = seed & 0xFFFF;
    __m512i v16si_res = blend_v16si(v16si_a, v16si_b, mask16);
    v16si_result = v16si_res;
    
    // Test V8DI
    __m512i v8di_a = _mm512_loadu_si512(i64_data1);
    __m512i v8di_b = _mm512_loadu_si512(i64_data2);
    __mmask8 mask8 = seed & 0xFF;
    __m512i v8di_res = blend_v8di(v8di_a, v8di_b, mask8);
    v8di_result = v8di_res;
    
    // Test V8DF
    __m512d v8df_a = _mm512_loadu_pd(f64_data1);
    __m512d v8df_b = _mm512_loadu_pd(f64_data2);
    __mmask8 mask8_df = (mask8 * 13) & 0xFF;
    __m512d v8df_res = blend_v8df(v8df_a, v8df_b, mask8_df);
    v8df_result = v8df_res;
    
    // Test V16SF
    __m512 v16sf_a = _mm512_loadu_ps(f32_data1);
    __m512 v16sf_b = _mm512_loadu_ps(f32_data2);
    __mmask16 mask16_sf = (mask16 * 17) & 0xFFFF;
    __m512 v16sf_res = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
    v16sf_result = v16sf_res;
    
    // Run pipeline
    checksum += pipeline_blend(seed);
    
    // Compute final checksum from all results
    int64_t* i64p = (int64_t*)&v64qi_res;
    for (int i = 0; i < 8; i++) checksum += i64p[i];
    
    i64p = (int64_t*)&v32hi_res;
    for (int i = 0; i < 8; i++) checksum += i64p[i];
    
    printf("Checksum: %f\n", checksum);
    return (int)checksum & 1;
}
