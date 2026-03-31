#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile variables to prevent optimization
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
uint64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 char_mask, __mmask16 int_mask);

__attribute__((target("avx512bw,avx512f,avx512fp16")))
float pipeline_mixed_precision(__m512i ints, __m512 floats, __m512h halves, 
                               __mmask16 int_mask, __mmask16 float_mask, __mmask32 half_mask);

// Implementation of blend functions with data-dependent computations
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional computation to prevent folding
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __m512i masked = _mm512_mask_blend_epi8(cmp, result, _mm512_set1_epi8(0xFF));
    
    // Store to volatile to force materialization
    v64qi_result = masked;
    return masked;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create data-dependent mask based on comparison
    __mmask32 cmp_mask = _mm512_cmplt_epi16_mask(a, b);
    __mmask32 combined_mask = mask ^ cmp_mask;  // XOR to prevent constant folding
    
    __m512i result = _mm512_mask_blend_epi16(combined_mask, a, b);
    
    // Additional operation: saturate to prevent optimization
    __m512i saturated = _mm512_adds_epi16(result, _mm512_set1_epi16(1));
    
    v32hi_result = saturated;
    return saturated;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Generate mask based on comparison result
    __mmask32 cmp_mask = _mm512_cmplt_ph_mask(a, b);
    __mmask32 dynamic_mask = mask & cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(dynamic_mask, a, b);
    
    // Additional computation
    __m512h scaled = _mm512_mul_ph(result, _mm512_set1_ph(1.5f));
    
    v32hf_result = scaled;
    return scaled;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Convert to float for comparison, then back
    __m512 a_f32 = _mm512_cvtneobf16_ps(a);
    __m512 b_f32 = _mm512_cvtneobf16_ps(b);
    
    __mmask16 cmp_mask_f32 = _mm512_cmplt_ps_mask(a_f32, b_f32);
    __mmask32 cmp_mask = _mm512_kunpackw(cmp_mask_f32, cmp_mask_f32);
    
    __mmask32 dynamic_mask = mask | cmp_mask;
    
    __m512bh result = _mm512_mask_blend_ph(dynamic_mask, a, b);
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Create complex mask based on multiple conditions
    __mmask16 lt_mask = _mm512_cmplt_epi32_mask(a, b);
    __mmask16 eq_mask = _mm512_cmpeq_epi32_mask(a, b);
    __mmask16 dynamic_mask = (mask & lt_mask) | (~mask & eq_mask);
    
    __m512i result = _mm512_mask_blend_epi32(dynamic_mask, a, b);
    
    // Additional computation
    __m512i squared = _mm512_mullo_epi32(result, result);
    
    v16si_result = squared;
    return squared;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Generate mask from comparison
    __mmask8 cmp_mask = _mm512_cmplt_epi64_mask(a, b);
    __mmask8 dynamic_mask = mask ^ cmp_mask;
    
    __m512i result = _mm512_mask_blend_epi64(dynamic_mask, a, b);
    
    // Additional operation
    __m512i shifted = _mm512_slli_epi64(result, 1);
    
    v8di_result = shifted;
    return shifted;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Create mask from floating comparison
    __mmask8 cmp_mask = _mm512_cmplt_pd_mask(a, b);
    __mmask8 dynamic_mask = mask & cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(dynamic_mask, a, b);
    
    // Additional computation
    __m512d scaled = _mm512_mul_pd(result, _mm512_set1_pd(2.0));
    
    v8df_result = scaled;
    return scaled;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Generate mask from multiple conditions
    __mmask16 lt_mask = _mm512_cmplt_ps_mask(a, b);
    __mmask16 gt_mask = _mm512_cmpgt_ps_mask(a, b);
    __mmask16 dynamic_mask = (mask & lt_mask) | (~mask & gt_mask);
    
    __m512 result = _mm512_mask_blend_ps(dynamic_mask, a, b);
    
    // Additional computation
    __m512 recip = _mm512_rcp14_ps(result);
    
    v16sf_result = recip;
    return recip;
}

// Multi-stage pipeline: char -> int blending
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 char_mask, __mmask16 int_mask) {
    // Stage 1: Blend 64x char (V64QI)
    __m512i blended_chars = _mm512_mask_blend_epi8(char_mask, chars, 
                                                   _mm512_slli_epi8(chars, 1));
    
    // Convert to 32-bit integers (creates V16SI mode vectors)
    __m512i ints_from_chars = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(blended_chars));
    
    // Stage 2: Blend 16x int (V16SI) - feeds from previous stage
    __m512i blended_ints = _mm512_mask_blend_epi32(int_mask, ints, ints_from_chars);
    
    // Horizontal sum
    return _mm512_reduce_add_epi32(blended_ints);
}

// Mixed precision pipeline
__attribute__((target("avx512bw,avx512f,avx512fp16")))
float pipeline_mixed_precision(__m512i ints, __m512 floats, __m512h halves, 
                               __mmask16 int_mask, __mmask16 float_mask, __mmask32 half_mask) {
    // Stage 1: Blend integers (V16SI)
    __m512i blended_ints = _mm512_mask_blend_epi32(int_mask, ints, 
                                                   _mm512_add_epi32(ints, _mm512_set1_epi32(1)));
    
    // Convert to float (V16SF)
    __m512 ints_as_float = _mm512_cvtepi32_ps(blended_ints);
    
    // Stage 2: Blend floats (V16SF)
    __m512 blended_floats = _mm512_mask_blend_ps(float_mask, floats, ints_as_float);
    
    // Convert to half precision (V32HF)
    __m512h floats_as_half = _mm512_cvtps_ph(blended_floats, _MM_FROUND_TO_NEAREST_INT);
    
    // Stage 3: Blend halves (V32HF)
    __m512h blended_halves = _mm512_mask_blend_ph(half_mask, halves, floats_as_half);
    
    // Convert back to float and reduce
    __m512 final_floats = _mm512_cvtph_ps(blended_halves);
    return _mm512_reduce_add_ps(final_floats);
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic behavior
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Initialize test data arrays
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t half_data[32];  // Half precision storage
    uint16_t bf16_data[32];  // BF16 storage
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)(rand() % 256 - 128);
    for (int i = 0; i < 32; i++) short_data[i] = (short)(rand() % 65536 - 32768);
    for (int i = 0; i < 16; i++) int_data[i] = rand() - RAND_MAX/2;
    for (int i = 0; i < 8; i++) long_data[i] = ((long long)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX * 100.0f - 50.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX * 100.0 - 50.0;
    for (int i = 0; i < 32; i++) half_data[i] = rand() % 65536;
    for (int i = 0; i < 32; i++) bf16_data[i] = rand() % 65536;
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((__m512i*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((__m512i*)(short_data + 16));
    
    __m512h v32hf_a = _mm512_castsi512_ph(_mm512_loadu_si512((__m512i*)half_data));
    __m512h v32hf_b = _mm512_castsi512_ph(_mm512_loadu_si512((__m512i*)(half_data + 16)));
    
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_loadu_si512((__m512i*)bf16_data));
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_loadu_si512((__m512i*)(bf16_data + 16)));
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)(long_data + 4));
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    // Generate runtime masks based on array indices and argc
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((i + argc) % 3 == 0) mask64 |= (1ULL << i);
    }
    for (int i = 0; i < 32; i++) {
        if ((i + argc) % 4 == 0) mask32 |= (1U << i);
    }
    for (int i = 0; i < 16; i++) {
        if ((i + argc) % 5 == 0) mask16 |= (1U << i);
    }
    for (int i = 0; i < 8; i++) {
        if ((i + argc) % 6 == 0) mask8 |= (1U << i);
    }
    
    // Call all blend functions
    __m512i res64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i res32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h res32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    __m512bh res32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
    __m512i res16si = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i res8di = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d res8df = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 res16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Execute pipeline functions
    uint64_t pipeline_sum = pipeline_char_to_int(v64qi_a, v16si_a, mask64, mask16);
    float mixed_sum = pipeline_mixed_precision(v16si_a, v16sf_a, v32hf_a, mask16, mask16, mask32);
    
    // Compute checksum from all results
    uint64_t checksum = 0;
    
    // Add integer results
    int64_t* int64_ptr = (int64_t*)&res64qi;
    for (int i = 0; i < 8; i++) checksum += int64_ptr[i];
    
    int64_ptr = (int64_t*)&res32hi;
    for (int i = 0; i < 8; i++) checksum += int64_ptr[i];
    
    int64_ptr = (int64_t*)&res16si;
    for (int i = 0; i < 8; i++) checksum += int64_ptr[i];
    
    int64_ptr = (int64_t*)&res8di;
    for (int i = 0; i < 8; i++) checksum += int64_ptr[i];
    
    // Add pipeline results
    checksum += pipeline_sum;
    checksum += (uint64_t)mixed_sum;
    
    printf("Checksum: %lu\n", checksum);
    printf("All blend operations executed successfully.\n");
    
    return 0;
}
