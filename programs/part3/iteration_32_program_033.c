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

// Data-dependent computation functions
__attribute__((target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    // Generate mask based on comparison
    return _mm512_cmpgt_epi8_mask(a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    // Generate mask based on comparison
    return _mm512_cmpgt_epi16_mask(a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__mmask32 compute_v32hf_mask(__m512h a, __m512h b) {
    // Generate mask based on comparison
    return _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
}

__attribute__((target("avx512bw,avx512bf16")))
__mmask32 compute_v32bf_mask(__m512bh a, __m512bh b) {
    // Convert to float for comparison
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    return _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
}

__attribute__((target("avx512f")))
__mmask16 compute_v16si_mask(__m512i a, __m512i b) {
    return _mm512_cmpgt_epi32_mask(a, b);
}

__attribute__((target("avx512f")))
__mmask8 compute_v8di_mask(__m512i a, __m512i b) {
    return _mm512_cmpgt_epi64_mask(a, b);
}

__attribute__((target("avx512f")))
__mmask8 compute_v8df_mask(__m512d a, __m512d b) {
    return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
}

__attribute__((target("avx512f")))
__mmask16 compute_v16sf_mask(__m512 a, __m512 b) {
    return _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
}

// Blend implementations with runtime-derived masks
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional data-dependent computation
    __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    result = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAA, result, temp);
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Multi-stage operation
    __m512i shifted = _mm512_slli_epi16(result, 1);
    result = _mm512_mask_blend_epi16(mask | 0x55555555, result, shifted);
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Type conversion stress
    __m512 result_f32 = _mm512_cvtph_ps(result);
    __m512h result2 = _mm512_cvtps_ph(result_f32, _MM_FROUND_TO_NEAREST_INT);
    result = _mm512_mask_blend_ph(mask ^ 0xAAAAAAAA, result, result2);
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Convert to float and back
    __m512 result_f32 = _mm512_cvtpbh_ps(result);
    __m512bh result2 = _mm512_cvtneps_pbh(result_f32);
    result = _mm512_mask_blend_ph(mask | 0x55555555, result, result2);
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Feed into different operation
    __m512i multiplied = _mm512_mullo_epi32(result, _mm512_set1_epi32(2));
    result = _mm512_mask_blend_epi32(mask ^ 0xAAAA, result, multiplied);
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Chain operations
    __m512i shifted = _mm512_slli_epi64(result, 2);
    result = _mm512_mask_blend_epi64(mask | 0x55, result, shifted);
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Mixed precision stress
    __m512 result_f32 = _mm512_cvtpd_ps(result);
    __m512d result2 = _mm512_cvtps_pd(_mm512_castps512_ps256(result_f32));
    result = _mm512_mask_blend_pd(mask ^ 0xAA, result, result2);
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Multi-stage pipeline
    __m512 squared = _mm512_mul_ps(result, result);
    result = _mm512_mask_blend_ps(mask | 0x5555, result, squared);
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline: process V64QI then pack to V16SI
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(uint8_t* data, int size) {
    int64_t checksum = 0;
    
    for (int i = 0; i + 64 <= size; i += 64) {
        // Load as V64QI
        __m512i v64qi = _mm512_loadu_si512((__m512i*)(data + i));
        __m512i v64qi2 = _mm512_loadu_si512((__m512i*)(data + i + 32));
        
        // Generate runtime mask
        __mmask64 mask = compute_v64qi_mask(v64qi, v64qi2);
        
        // Blend V64QI
        __m512i blended = blend_v64qi(v64qi, v64qi2, mask);
        
        // Pack to V16SI (4:1 packing)
        __m512i packed = _mm512_cvtepu8_epi32(_mm512_castsi512_si128(blended));
        __m512i packed2 = _mm512_cvtepu8_epi32(_mm512_extracti32x4_epi32(blended, 1));
        
        // Generate mask for V16SI blend
        __mmask16 mask16 = compute_v16si_mask(packed, packed2);
        
        // Blend V16SI
        __m512i blended16si = blend_v16si(packed, packed2, mask16);
        
        // Accumulate checksum
        int32_t* ptr = (int32_t*)&blended16si;
        for (int j = 0; j < 16; j++) {
            checksum += ptr[j];
        }
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic data
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Allocate and initialize test data
    const int DATA_SIZE = 4096;
    uint8_t* char_data = (uint8_t*)aligned_alloc(64, DATA_SIZE);
    uint16_t* short_data = (uint16_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint16_t));
    int32_t* int_data = (int32_t*)aligned_alloc(64, DATA_SIZE * sizeof(int32_t));
    int64_t* long_data = (int64_t*)aligned_alloc(64, DATA_SIZE * sizeof(int64_t));
    float* float_data = (float*)aligned_alloc(64, DATA_SIZE * sizeof(float));
    double* double_data = (double*)aligned_alloc(64, DATA_SIZE * sizeof(double));
    uint16_t* half_data = (uint16_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint16_t));
    uint16_t* bfloat_data = (uint16_t*)aligned_alloc(64, DATA_SIZE * sizeof(uint16_t));
    
    // Initialize with pseudo-random values
    for (int i = 0; i < DATA_SIZE; i++) {
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 65536;
        int_data[i] = rand();
        long_data[i] = ((int64_t)rand() << 32) | rand();
        float_data[i] = (float)rand() / RAND_MAX;
        double_data[i] = (double)rand() / RAND_MAX;
        half_data[i] = rand() % 65536;
        bfloat_data[i] = rand() % 65536;
    }
    
    int64_t total_checksum = 0;
    
    // Execute all blend functions with runtime-derived masks
    
    // 1. V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 64));
    __mmask64 mask64 = compute_v64qi_mask(v64qi_a, v64qi_b);
    __m512i blended_v64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // 2. V32HI blend
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 32));
    __mmask32 mask32 = compute_v32hi_mask(v32hi_a, v32hi_b);
    __m512i blended_v32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // 3. V32HF blend (if supported)
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_loadu_ph(half_data);
    __m512h v32hf_b = _mm512_loadu_ph(half_data + 32);
    __mmask32 mask32_hf = compute_v32hf_mask(v32hf_a, v32hf_b);
    __m512h blended_v32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
    #endif
    
    // 4. V32BF blend (if supported)
    #ifdef __AVX512BF16__
    __m512bh v32bf_a = _mm512_loadu_si512((__m512i*)bfloat_data);
    __m512bh v32bf_b = _mm512_loadu_si512((__m512i*)(bfloat_data + 32));
    __mmask32 mask32_bf = compute_v32bf_mask(v32bf_a, v32bf_b);
    __m512bh blended_v32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
    #endif
    
    // 5. V16SI blend
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 16));
    __mmask16 mask16 = compute_v16si_mask(v16si_a, v16si_b);
    __m512i blended_v16si = blend_v16si(v16si_a, v16si_b, mask16);
    
    // 6. V8DI blend
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 8));
    __mmask8 mask8 = compute_v8di_mask(v8di_a, v8di_b);
    __m512i blended_v8di = blend_v8di(v8di_a, v8di_b, mask8);
    
    // 7. V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 8);
    __mmask8 mask8_df = compute_v8df_mask(v8df_a, v8df_b);
    __m512d blended_v8df = blend_v8df(v8df_a, v8df_b, mask8_df);
    
    // 8. V16SF blend
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 16);
    __mmask16 mask16_sf = compute_v16sf_mask(v16sf_a, v16sf_b);
    __m512 blended_v16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
    
    // Multi-stage pipeline
    total_checksum += pipeline_v64qi_to_v16si(char_data, DATA_SIZE);
    
    // Compute final checksum from all results
    uint8_t* ptr;
    
    ptr = (uint8_t*)&blended_v64qi;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    
    ptr = (uint8_t*)&blended_v32hi;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    
    #ifdef __AVX512FP16__
    ptr = (uint8_t*)&blended_v32hf;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    #endif
    
    #ifdef __AVX512BF16__
    ptr = (uint8_t*)&blended_v32bf;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    #endif
    
    ptr = (uint8_t*)&blended_v16si;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    
    ptr = (uint8_t*)&blended_v8di;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    
    ptr = (uint8_t*)&blended_v8df;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    
    ptr = (uint8_t*)&blended_v16sf;
    for (int i = 0; i < 64; i++) total_checksum += ptr[i];
    
    printf("Checksum: %ld\n", total_checksum);
    
    // Cleanup
    free(char_data);
    free(short_data);
    free(int_data);
    free(long_data);
    free(float_data);
    free(double_data);
    free(half_data);
    free(bfloat_data);
    
    return 0;
}
