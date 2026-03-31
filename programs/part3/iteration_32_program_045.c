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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional computation to prevent folding
    __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    result = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAA, result, temp);
    
    // Store to volatile to force materialization
    v64qi_result = result;
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create data-dependent mask
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    __mmask32 blend_mask = mask & cmp;
    
    __m512i result = _mm512_mask_blend_epi16(blend_mask, a, b);
    
    // Multi-stage operation
    __m512i scaled = _mm512_slli_epi16(result, 1);
    result = _mm512_mask_blend_epi16(mask | 0x55555555, result, scaled);
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Generate mask from comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    __mmask32 blend_mask = mask ^ cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Mixed precision operation
    __m512h half = _mm512_set1_ph(0.5f);
    result = _mm512_mask_blend_ph(mask, result, _mm512_mul_ph(result, half));
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use bfloat16 blend intrinsic
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Additional blend with modified mask
    __m512bh ones = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
    result = _mm512_mask_blend_ph(mask >> 1, result, ones);
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Complex mask generation
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_movepi32_mask(_mm512_srai_epi32(diff, 31));
    __mmask16 blend_mask = mask | sign_mask;
    
    __m512i result = _mm512_mask_blend_epi32(blend_mask, a, b);
    
    // Chain blends
    __m512i abs_val = _mm512_abs_epi32(result);
    result = _mm512_mask_blend_epi32(blend_mask ^ 0xAAAA, result, abs_val);
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Data-dependent blending
    __mmask8 eq_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 blend_mask = mask & ~eq_mask;
    
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    
    // Multi-operation sequence
    __m512i shifted = _mm512_slli_epi64(result, 2);
    result = _mm512_mask_blend_epi64(mask | 0x55, result, shifted);
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Generate mask from floating comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 blend_mask = mask ^ cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(blend_mask, a, b);
    
    // Mathematical transformation
    __m512d sqrt_val = _mm512_sqrt_pd(_mm512_abs_pd(result));
    result = _mm512_mask_blend_pd(blend_mask, result, sqrt_val);
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Complex floating-point blend
    __mmask16 nan_mask = _mm512_cmp_ps_mask(a, a, _CMP_UNORD_Q);
    __mmask16 blend_mask = mask & ~nan_mask;
    
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    
    // Type conversion sequence
    __m512i int_vec = _mm512_cvtps_epi32(result);
    __m512 float_vec = _mm512_cvtepi32_ps(int_vec);
    result = _mm512_mask_blend_ps(blend_mask | 0x5555, result, float_vec);
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline function
__attribute__((target("avx512f,avx512bw")))
uint64_t vector_pipeline(unsigned int seed) {
    uint64_t checksum = 0;
    
    // Initialize data with seed-dependent values
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t fp16_data[32];
    uint16_t bf16_data[32];
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)((i * seed) & 0xFF);
    for (int i = 0; i < 32; i++) short_data[i] = (short)((i * seed * 3) & 0xFFFF);
    for (int i = 0; i < 16; i++) int_data[i] = i * seed * 7;
    for (int i = 0; i < 8; i++) long_data[i] = (int64_t)i * seed * 11;
    for (int i = 0; i < 16; i++) float_data[i] = (float)(i * seed) / 256.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)(i * seed) / 512.0;
    for (int i = 0; i < 32; i++) fp16_data[i] = (uint16_t)((i * seed) & 0xFFFF);
    for (int i = 0; i < 32; i++) bf16_data[i] = (uint16_t)((i * seed * 5) & 0xFFFF);
    
    // V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((char_data[i] & 1) == (seed & 1)) {
            mask64 |= (1ULL << i);
        }
    }
    __m512i v64qi_res = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // V32HI blend (using packed result from V64QI)
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_packs_epi32(v64qi_res, v64qi_res); // Type conversion
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        mask32 |= ((short_data[i] > 0) << i);
    }
    __m512i v32hi_res = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // V32HF blend
    __m512h v32hf_a = _mm512_loadu_ph(fp16_data);
    __m512h v32hf_b = _mm512_set1_ph(1.0f);
    __m512h v32hf_res = blend_v32hf(v32hf_a, v32hf_b, mask32);
    
    // V32BF blend
    __m512bh v32bf_a = _mm512_loadu_si512(bf16_data);
    __m512bh v32bf_b = _mm512_set1_epi16(0x3F80); // 1.0 in bf16
    __m512bh v32bf_res = blend_v32bf(v32bf_a, v32bf_b, mask32);
    
    // V16SI blend (using packed result from V32HI)
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(v32hi_res, 0));
    __mmask16 mask16 = (__mmask16)(seed & 0xFFFF);
    __m512i v16si_res = blend_v16si(v16si_a, v16si_b, mask16);
    
    // V8DI blend
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_slli_epi64(v8di_a, 1);
    __mmask8 mask8 = (__mmask8)(seed & 0xFF);
    __m512i v8di_res = blend_v8di(v8di_a, v8di_b, mask8);
    
    // V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_set1_pd(2.0);
    __m512d v8df_res = blend_v8df(v8df_a, v8df_b, mask8);
    
    // V16SF blend (with type conversion)
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_cvtepi32_ps(v16si_res);
    __m512 v16sf_res = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Compute checksum from all results
    uint8_t* byte_ptr = (uint8_t*)&v64qi_res;
    for (int i = 0; i < 64; i++) checksum += byte_ptr[i];
    
    byte_ptr = (uint8_t*)&v32hi_res;
    for (int i = 0; i < 64; i++) checksum += byte_ptr[i];
    
    uint16_t* half_ptr = (uint16_t*)&v32hf_res;
    for (int i = 0; i < 32; i++) checksum += half_ptr[i];
    
    half_ptr = (uint16_t*)&v32bf_res;
    for (int i = 0; i < 32; i++) checksum += half_ptr[i];
    
    byte_ptr = (uint8_t*)&v16si_res;
    for (int i = 0; i < 64; i++) checksum += byte_ptr[i];
    
    byte_ptr = (uint8_t*)&v8di_res;
    for (int i = 0; i < 64; i++) checksum += byte_ptr[i];
    
    byte_ptr = (uint8_t*)&v8df_res;
    for (int i = 0; i < 64; i++) checksum += byte_ptr[i];
    
    byte_ptr = (uint8_t*)&v16sf_res;
    for (int i = 0; i < 64; i++) checksum += byte_ptr[i];
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for runtime variability
    unsigned int seed = (unsigned int)argc;
    
    // Run the vector pipeline multiple times with different seeds
    uint64_t total_checksum = 0;
    for (int i = 0; i < 3; i++) {
        total_checksum += vector_pipeline(seed + i);
    }
    
    printf("Total checksum: %lu\n", total_checksum);
    
    // Force use of volatile results
    printf("Volatile results exist: %p %p %p\n", 
           (void*)&v64qi_result, 
           (void*)&v32hi_result,
           (void*)&v16sf_result);
    
    return (int)(total_checksum & 0x7FFFFFFF);
}
