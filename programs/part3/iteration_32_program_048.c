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
__attribute__((noinline, target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    // Compare and create dynamic mask
    __mmask64 mask = _mm512_cmpneq_epi8_mask(a, b);
    
    // Use runtime value to modify mask
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask64 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v64qi(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    __mmask32 mask = _mm512_cmpneq_epi16_mask(a, b);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask32 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v32hi(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_NEQ_OQ);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask32 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v32hf(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    // Convert to float for comparison
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_NEQ_OQ);
    
    // Expand to 32-bit mask for blending
    __mmask32 mask = _mm512_kunpackd(mask16, mask16);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask32 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v32bf(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    __mmask16 mask = _mm512_cmpneq_epi32_mask(a, b);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask16 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v16si(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    __mmask8 mask = _mm512_cmpneq_epi64_mask(a, b);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask8 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v8di(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_NEQ_OQ);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask8 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v8df(a, b, dynamic_mask);
}

__attribute__((noinline, target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_NEQ_OQ);
    volatile int seed = (int)((uintptr_t)&a) & 0xFF;
    __mmask16 dynamic_mask = mask ^ (seed & 1);
    
    return blend_v16sf(a, b, dynamic_mask);
}

// Blend function implementations
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion for V64QImode
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store to volatile global to prevent elimination
    g_v64qi_result = result;
    
    // Additional computation to create data dependency
    return _mm512_add_epi8(result, _mm512_set1_epi8(1));
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    g_v32hi_result = result;
    return _mm512_add_epi16(result, _mm512_set1_epi16(1));
}

__attribute__((noinline, target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    g_v32hf_result = result;
    
    // Convert to float and back to create mode transitions
    __m512 temp = _mm512_cvtph_ps(result);
    temp = _mm512_add_ps(temp, _mm512_set1_ps(1.0f));
    return _mm512_cvtps_ph(temp, _MM_FROUND_CUR_DIRECTION);
}

__attribute__((noinline, target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    g_v32bf_result = result;
    
    // Convert to float, operate, convert back
    __m512 temp = _mm512_cvtpbh_ps(result);
    temp = _mm512_add_ps(temp, _mm512_set1_ps(1.0f));
    return _mm512_cvtne2ps_pbh(temp, temp);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    g_v16si_result = result;
    return _mm512_add_epi32(result, _mm512_set1_epi32(1));
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    g_v8di_result = result;
    return _mm512_add_epi64(result, _mm512_set1_epi64(1));
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    g_v8df_result = result;
    return _mm512_add_pd(result, _mm512_set1_pd(1.0));
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    g_v16sf_result = result;
    return _mm512_add_ps(result, _mm512_set1_ps(1.0f));
}

// Multi-stage pipeline: V64QI -> V16SI conversion and blending
__attribute__((noinline, target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(uint8_t* data) {
    // Load as V64QI
    __m512i v64qi_a = _mm512_loadu_si512(data);
    __m512i v64qi_b = _mm512_loadu_si512(data + 64);
    
    // Blend at V64QI level
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL; // Alternating pattern
    __m512i blended_qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Convert to V16SI (pack bytes to ints)
    __m512i v16si_a = _mm512_cvtepu8_epi32(_mm512_castsi512_si256(blended_qi));
    __m512i v16si_b = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_qi, 1));
    
    // Blend at V16SI level
    __mmask16 mask16 = 0xAAAA; // Alternating pattern
    __m512i blended_si = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Horizontal sum
    return _mm512_reduce_add_epi32(blended_si);
}

// Mixed precision pipeline: integer -> float -> blend
__attribute__((noinline, target("avx512f")))
double pipeline_int_to_float(int32_t* data) {
    // Load as integers
    __m512i v16si = _mm512_loadu_si512(data);
    
    // Convert to float
    __m512 v16sf_a = _mm512_cvtepi32_ps(v16si);
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    
    // Blend floats
    __mmask16 mask16 = 0x5555; // Alternating pattern, different from previous
    __m512 blended_sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Convert to double and blend
    __m512d v8df_a = _mm512_cvtps_pd(_mm512_castps512_ps256(blended_sf));
    __m512d v8df_b = _mm512_set1_pd(200.0);
    
    __mmask8 mask8 = 0xAA; // Different pattern
    __m512d blended_df = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Horizontal sum
    return _mm512_reduce_add_pd(blended_df);
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random initialization
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Allocate and initialize test data
    uint8_t data_8bit[128];
    uint16_t data_16bit[64];
    uint32_t data_32bit[32];
    uint64_t data_64bit[16];
    float data_float[32];
    double data_double[16];
    
    for (int i = 0; i < 128; i++) data_8bit[i] = rand() & 0xFF;
    for (int i = 0; i < 64; i++) data_16bit[i] = rand() & 0xFFFF;
    for (int i = 0; i < 32; i++) data_32bit[i] = rand();
    for (int i = 0; i < 16; i++) data_64bit[i] = ((uint64_t)rand() << 32) | rand();
    for (int i = 0; i < 32; i++) data_float[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 16; i++) data_double[i] = (double)rand() / RAND_MAX;
    
    int64_t checksum = 0;
    
    // Test V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512(data_8bit);
    __m512i v64qi_b = _mm512_loadu_si512(data_8bit + 64);
    __m512i result_qi = compute_v64qi_mask(v64qi_a, v64qi_b);
    
    // Test V32HI blend
    __m512i v32hi_a = _mm512_loadu_si512(data_16bit);
    __m512i v32hi_b = _mm512_loadu_si512(data_16bit + 32);
    __m512i result_hi = compute_v32hi_mask(v32hi_a, v32hi_b);
    
    // Test V16SI blend
    __m512i v16si_a = _mm512_loadu_si512(data_32bit);
    __m512i v16si_b = _mm512_loadu_si512(data_32bit + 16);
    __m512i result_si = compute_v16si_mask(v16si_a, v16si_b);
    
    // Test V8DI blend
    __m512i v8di_a = _mm512_loadu_si512(data_64bit);
    __m512i v8di_b = _mm512_loadu_si512(data_64bit + 8);
    __m512i result_di = compute_v8di_mask(v8di_a, v8di_b);
    
    // Test V16SF blend
    __m512 v16sf_a = _mm512_loadu_ps(data_float);
    __m512 v16sf_b = _mm512_loadu_ps(data_float + 16);
    __m512 result_sf = compute_v16sf_mask(v16sf_a, v16sf_b);
    
    // Test V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(data_double);
    __m512d v8df_b = _mm512_loadu_pd(data_double + 8);
    __m512d result_df = compute_v8df_mask(v8df_a, v8df_b);
    
    // Test pipeline operations
    checksum += pipeline_v64qi_to_v16si(data_8bit);
    checksum += (int64_t)pipeline_int_to_float((int32_t*)data_32bit);
    
    // Accumulate results into checksum
    checksum += _mm512_reduce_add_epi8(result_qi);
    checksum += _mm512_reduce_add_epi16(result_hi);
    checksum += _mm512_reduce_add_epi32(result_si);
    checksum += _mm512_reduce_add_epi64(result_di);
    checksum += (int64_t)_mm512_reduce_add_ps(result_sf);
    checksum += (int64_t)_mm512_reduce_add_pd(result_df);
    
    printf("Checksum: %ld\n", checksum);
    
    // Force use of half-precision types if supported
#ifdef __AVX512FP16__
    _Float16 data_half[64];
    for (int i = 0; i < 64; i++) data_half[i] = (_Float16)((float)rand() / RAND_MAX);
    
    __m512h v32hf_a = _mm512_loadu_ph(data_half);
    __m512h v32hf_b = _mm512_loadu_ph(data_half + 32);
    __m512h result_hf = compute_v32hf_mask(v32hf_a, v32hf_b);
    
    // Use result to prevent elimination
    volatile __m512h temp_hf = result_hf;
#endif
    
#ifdef __AVX512BF16__
    __bf16 data_bf16[64];
    for (int i = 0; i < 64; i++) {
        uint16_t val = rand() & 0xFFFF;
        data_bf16[i] = *(reinterpret_cast<__bf16*>(&val));
    }
    
    __m512bh v32bf_a = _mm512_loadu_si512(data_bf16);
    __m512bh v32bf_b = _mm512_loadu_si512(data_bf16 + 32);
    __m512bh result_bf = compute_v32bf_mask(v32bf_a, v32bf_b);
    
    volatile __m512bh temp_bf = result_bf;
#endif
    
    return 0;
}
