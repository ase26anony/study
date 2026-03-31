#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

// Multi-stage pipeline functions
__attribute__((target("avx512bw,avx512f")))
int pipeline_char_to_int(unsigned char* data, int size);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_float_to_half(float* data, int size);

// V64QI blend implementation
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Use runtime-derived mask to prevent constant folding
    volatile __mmask64 vm = mask;
    
    // Data-dependent computation: compare and blend
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __mmask64 blend_mask = vm ^ cmp;  // XOR with input mask
    
    // Force blend operation with non-constant mask
    __m512i result = _mm512_mask_blend_epi8(blend_mask, a, b);
    
    // Store to volatile global to prevent elimination
    g_v64qi_result = result;
    
    return result;
}

// V32HI blend implementation
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    volatile __mmask32 vm = mask;
    
    // Create data-dependent mask
    __m512i sum = _mm512_add_epi16(a, b);
    __mmask32 zero_mask = _mm512_cmpeq_epi16_mask(sum, _mm512_setzero_si512());
    __mmask32 blend_mask = vm & ~zero_mask;
    
    __m512i result = _mm512_mask_blend_epi16(blend_mask, a, b);
    
    // Additional computation to create dataflow
    result = _mm512_add_epi16(result, _mm512_set1_epi16(1));
    g_v32hi_result = result;
    
    return result;
}

// V32HF blend implementation (requires -mavx512fp16)
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    volatile __mmask32 vm = mask;
    
    // Compare and create data-dependent mask
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    __mmask32 blend_mask = vm | cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(blend_mask, a, b);
    
    // Mixed precision operation
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    g_v32hf_result = result;
    
    return result;
}

// V32BF blend implementation (requires -mavx512bf16)
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    volatile __mmask32 vm = mask;
    
    // Convert to float for comparison
    __m512 a_f32 = _mm512_cvtneobf16_ps(a);
    __m512 b_f32 = _mm512_cvtneobf16_ps(b);
    
    __mmask16 cmp_mask_f32 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_LT_OQ);
    
    // Expand to 32-bit mask for bf16 blend
    __mmask32 blend_mask = vm;
    for (int i = 0; i < 16; i++) {
        if (cmp_mask_f32 & (1 << i)) {
            blend_mask |= (3 << (i * 2));  // Set both bf16 elements
        }
    }
    
    __m512bh result = _mm512_mask_blend_ph(blend_mask, a, b);
    g_v32bf_result = result;
    
    return result;
}

// V16SI blend implementation
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    volatile __mmask16 vm = mask;
    
    // Data-dependent mask from arithmetic
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 neg_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    __mmask16 blend_mask = vm ^ neg_mask;
    
    __m512i result = _mm512_mask_blend_epi32(blend_mask, a, b);
    
    // Additional computation
    result = _mm512_mullo_epi32(result, _mm512_set1_epi32(2));
    g_v16si_result = result;
    
    return result;
}

// V8DI blend implementation
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    volatile __mmask8 vm = mask;
    
    // Create mask from comparison
    __mmask8 eq_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 blend_mask = vm & ~eq_mask;
    
    __m512i result = _mm512_mask_blend_epi64(blend_mask, a, b);
    
    // Shift operation to modify result
    result = _mm512_slli_epi64(result, 1);
    g_v8di_result = result;
    
    return result;
}

// V8DF blend implementation
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    volatile __mmask8 vm = mask;
    
    // Data-dependent mask from FP comparison
    __mmask8 lt_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 blend_mask = vm | lt_mask;
    
    __m512d result = _mm512_mask_blend_pd(blend_mask, a, b);
    
    // Mathematical operation
    result = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    g_v8df_result = result;
    
    return result;
}

// V16SF blend implementation
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    volatile __mmask16 vm = mask;
    
    // Complex mask computation
    __mmask16 nan_mask = _mm512_cmp_ps_mask(a, a, _CMP_UNORD_Q);
    __mmask16 inf_mask = _mm512_cmp_ps_mask(_mm512_abs_ps(a), 
                                          _mm512_set1_ps(1e30f), _CMP_GT_OQ);
    __mmask16 blend_mask = vm & ~(nan_mask | inf_mask);
    
    __m512 result = _mm512_mask_blend_ps(blend_mask, a, b);
    
    // Additional FP operation
    result = _mm512_add_ps(result, _mm512_set1_ps(0.5f));
    g_v16sf_result = result;
    
    return result;
}

// Multi-stage pipeline: V64QI -> V16SI
__attribute__((target("avx512bw,avx512f")))
int pipeline_char_to_int(unsigned char* data, int size) {
    int sum = 0;
    
    // Process in chunks of 64 bytes
    for (int i = 0; i + 64 <= size; i += 64) {
        // Load as V64QI
        __m512i chars = _mm512_loadu_si512(data + i);
        
        // Create alternating mask based on index (runtime value)
        __mmask64 char_mask = 0;
        for (int j = 0; j < 64; j++) {
            if ((i + j) % 3 == 0) {
                char_mask |= (1ULL << j);
            }
        }
        
        // Blend V64QI
        __m512i alt_chars = _mm512_set1_epi8(0x7F);
        __m512i blended_chars = _mm512_mask_blend_epi8(char_mask, chars, alt_chars);
        
        // Convert to V16SI (4 chars per int)
        __m512i ints1 = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_chars, 0));
        __m512i ints2 = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_chars, 1));
        
        // Create mask for V16SI blend
        __mmask16 int_mask = 0;
        for (int j = 0; j < 16; j++) {
            if ((i / 4 + j) % 2 == 0) {
                int_mask |= (1 << j);
            }
        }
        
        // Blend V16SI
        __m512i alt_ints = _mm512_set1_epi32(255);
        __m512i blended_ints1 = _mm512_mask_blend_epi32(int_mask, ints1, alt_ints);
        __m512i blended_ints2 = _mm512_mask_blend_epi32(int_mask, ints2, alt_ints);
        
        // Horizontal add
        sum += _mm512_reduce_add_epi32(blended_ints1);
        sum += _mm512_reduce_add_epi32(blended_ints2);
    }
    
    return sum;
}

// Multi-stage pipeline: V16SF -> V32HF
__attribute__((target("avx512f,avx512fp16")))
float pipeline_float_to_half(float* data, int size) {
    float sum = 0.0f;
    
    // Process in chunks of 16 floats
    for (int i = 0; i + 16 <= size; i += 16) {
        // Load as V16SF
        __m512 floats = _mm512_loadu_ps(data + i);
        
        // Create mask based on value sign (runtime dependent)
        __mmask16 float_mask = _mm512_cmp_ps_mask(floats, _mm512_setzero_ps(), _CMP_GT_OQ);
        
        // Blend V16SF
        __m512 alt_floats = _mm512_set1_ps(1.0f);
        __m512 blended_floats = _mm512_mask_blend_ps(float_mask, floats, alt_floats);
        
        // Convert to V32HF
        __m512h halves = _mm512_cvtps_ph(blended_floats, _MM_FROUND_TO_NEAREST_INT);
        
        // Create mask for V32HF blend
        __mmask32 half_mask = 0;
        for (int j = 0; j < 32; j++) {
            if ((i * 2 + j) % 4 == 0) {
                half_mask |= (1 << j);
            }
        }
        
        // Blend V32HF
        __m512h alt_halves = _mm512_set1_ph(0.5f);
        __m512h blended_halves = _mm512_mask_blend_ph(half_mask, halves, alt_halves);
        
        // Convert back to float for accumulation
        __m512 reconverted = _mm512_cvtph_ps(blended_halves);
        sum += _mm512_reduce_add_ps(reconverted);
    }
    
    return sum;
}

int main(int argc, char** argv) {
    // Use argc as pseudo-random seed
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Initialize test data
    unsigned char char_data[512];
    short short_data[512];
    int int_data[512];
    long long long_data[512];
    float float_data[512];
    double double_data[512];
    uint16_t bf16_data[1024];  // bf16 stored as uint16_t
    
    for (int i = 0; i < 512; i++) {
        char_data[i] = rand() % 256;
        short_data[i] = rand() % 65536;
        int_data[i] = rand();
        long_data[i] = ((long long)rand() << 32) | rand();
        float_data[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    for (int i = 0; i < 1024; i++) {
        bf16_data[i] = rand() % 65536;
    }
    
    // Load vectors
    __m512i v64qi_a = _mm512_loadu_si512(char_data);
    __m512i v64qi_b = _mm512_loadu_si512(char_data + 64);
    
    __m512i v32hi_a = _mm512_loadu_si512(short_data);
    __m512i v32hi_b = _mm512_loadu_si512(short_data + 32);
    
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_loadu_si512(int_data + 16);
    
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_loadu_si512(long_data + 8);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 16);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 8);
    
    // For half-precision, we need to convert from float
    __m512h v32hf_a = _mm512_cvtps_ph(_mm512_loadu_ps(float_data), _MM_FROUND_TO_NEAREST_INT);
    __m512h v32hf_b = _mm512_cvtps_ph(_mm512_loadu_ps(float_data + 16), _MM_FROUND_TO_NEAREST_INT);
    
    // For bfloat16
    __m512bh v32bf_a = _mm512_loadu_si512(bf16_data);
    __m512bh v32bf_b = _mm512_loadu_si512(bf16_data + 32);
    
    // Create runtime-derived masks
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) mask64 |= (1ULL << i);
        if (i < 32 && i % 3 == 0) mask32 |= (1 << i);
        if (i < 16 && i % 4 == 0) mask16 |= (1 << i);
        if (i < 8 && i % 5 == 0) mask8 |= (1 << i);
    }
    
    // Execute all blend operations
    __m512i r1 = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r2 = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h r3 = blend_v32hf(v32hf_a, v32hf_b, mask32);
    __m512bh r4 = blend_v32bf(v32bf_a, v32bf_b, mask32);
    __m512i r5 = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r6 = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r7 = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 r8 = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Execute multi-stage pipelines
    int pipeline_sum = pipeline_char_to_int(char_data, 512);
    float pipeline_float_sum = pipeline_float_to_half(float_data, 512);
    
    // Compute checksum
    long long checksum = 0;
    
    // Extract and sum results
    unsigned char r1_bytes[64];
    _mm512_storeu_si512(r1_bytes, r1);
    for (int i = 0; i < 64; i++) checksum += r1_bytes[i];
    
    short r2_shorts[32];
    _mm512_storeu_si512(r2_shorts, r2);
    for (int i = 0; i < 32; i++) checksum += r2_shorts[i];
    
    checksum += pipeline_sum;
    checksum += (long long)pipeline_float_sum;
    
    printf("Checksum: %lld\n", checksum);
    printf("All blend operations executed.\n");
    
    return 0;
}
