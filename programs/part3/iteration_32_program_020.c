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
int64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 char_mask, __mmask16 int_mask);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_half_to_float(__m512h halfs, __m512 floats, __mmask32 half_mask, __mmask16 float_mask);

// Data-dependent computation with loops
__attribute__((target("avx512f")))
void compute_blend_chain(int iterations, volatile double* output);

// Implementation of blend functions with runtime-derived masks
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force RTL expansion by using intrinsic with runtime mask
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Additional computation to prevent folding
    result = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    
    // Store to volatile to force materialization
    v64qi_result = result;
    
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Data-dependent operation
    result = _mm512_mullo_epi16(result, _mm512_set1_epi16(2));
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision operation
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Note: __bf16 uses same intrinsic as __m512h
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Complex dataflow
    result = _mm512_add_epi32(result, _mm512_slli_epi32(result, 1));
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    result = _mm512_xor_epi64(result, _mm512_set1_epi64(0xAAAAAAAAAAAAAAAA));
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Type conversion sequence
    result = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Mixed operations
    result = _mm512_add_ps(result, _mm512_sqrt_ps(result));
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline: char -> int
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_char_to_int(__m512i chars, __m512i ints, __mmask64 char_mask, __mmask16 int_mask) {
    // First blend at V64QI mode
    __m512i blended_chars = _mm512_mask_blend_epi8(char_mask, chars, 
                                                  _mm512_slli_epi8(chars, 1));
    
    // Convert to wider type (promotes to V16SI)
    __m512i extended = _mm512_cvtepi8_epi32(_mm512_castsi512_si256(blended_chars));
    
    // Second blend at V16SI mode
    __m512i blended_ints = _mm512_mask_blend_epi32(int_mask, extended, ints);
    
    // Horizontal sum to create dependency
    return _mm512_reduce_add_epi32(blended_ints);
}

// Multi-stage pipeline: half -> float
__attribute__((target("avx512f,avx512fp16")))
float pipeline_half_to_float(__m512h halfs, __m512 floats, __mmask32 half_mask, __mmask16 float_mask) {
    // Blend at V32HF mode
    __m512h blended_halfs = _mm512_mask_blend_ph(half_mask, halfs, 
                                                _mm512_add_ph(halfs, _mm512_set1_ph(1.0f)));
    
    // Convert to float (V16SF)
    __m512 converted = _mm512_cvtph_ps(blended_halfs);
    
    // Blend at V16SF mode
    __m512 blended_floats = _mm512_mask_blend_ps(float_mask, converted, floats);
    
    // Horizontal sum
    return _mm512_reduce_add_ps(blended_floats);
}

// Data-dependent computation with loop
__attribute__((target("avx512f")))
void compute_blend_chain(int iterations, volatile double* output) {
    __m512d accum = _mm512_setzero_pd();
    __m512d vec1 = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d vec2 = _mm512_setr_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    
    for (int i = 0; i < iterations; i++) {
        // Runtime-derived mask prevents constant folding
        __mmask8 mask = (i % 3) ? 0xFF : 0xAA;
        
        // Blend at V8DF mode inside loop
        __m512d blended = _mm512_mask_blend_pd(mask, vec1, vec2);
        
        // Data-dependent update
        accum = _mm512_add_pd(accum, blended);
        
        // Modify vectors for next iteration
        vec1 = _mm512_add_pd(vec1, _mm512_set1_pd(0.1));
        vec2 = _mm512_sub_pd(vec2, _mm512_set1_pd(0.1));
    }
    
    // Store result through volatile pointer
    _mm512_storeu_pd((double*)output, accum);
}

int main(int argc, char** argv) {
    // Use argc as pseudo-random seed
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    // Initialize test data arrays
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    uint16_t half_data[32];  // Half-precision storage
    uint16_t bf16_data[32];  // Brain float storage
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)(rand() % 256 - 128);
    for (int i = 0; i < 32; i++) short_data[i] = (short)(rand() % 65536 - 32768);
    for (int i = 0; i < 16; i++) int_data[i] = rand() - RAND_MAX/2;
    for (int i = 0; i < 8; i++) long_data[i] = ((long long)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX * 100.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX * 100.0;
    for (int i = 0; i < 32; i++) half_data[i] = 0x3C00; // 1.0 in half-precision
    for (int i = 0; i < 32; i++) bf16_data[i] = 0x3F80; // 1.0 in bfloat16
    
    // Load into vectors
    __m512i v64qi_a = _mm512_loadu_si512(char_data);
    __m512i v64qi_b = _mm512_loadu_si512(char_data + 32);
    
    __m512i v32hi_a = _mm512_loadu_si512(short_data);
    __m512i v32hi_b = _mm512_loadu_si512(short_data + 16);
    
    __m512h v32hf_a = _mm512_castsi512_ph(_mm512_loadu_si512(half_data));
    __m512h v32hf_b = _mm512_castsi512_ph(_mm512_loadu_si512(half_data + 16));
    
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_loadu_si512(bf16_data));
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_loadu_si512(bf16_data + 16));
    
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_loadu_si512(int_data + 8);
    
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_loadu_si512(long_data + 4);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    // Generate runtime-derived masks
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) mask64 |= (1ULL << i);
    }
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 2 == 0) mask32 |= (1U << i);
    }
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 4 == 0) mask16 |= (1U << i);
    }
    for (int i = 0; i < 8; i++) {
        if ((i + seed) % 3 == 0) mask8 |= (1U << i);
    }
    
    // Call all blend functions
    __m512i r1 = blend_v64qi(v64qi_a, v64qi_b, mask64);
    __m512i r2 = blend_v32hi(v32hi_a, v32hi_b, mask32);
    __m512h r3 = blend_v32hf(v32hf_a, v32hf_b, mask32);
    __m512bh r4 = blend_v32bf(v32bf_a, v32bf_b, mask32);
    __m512i r5 = blend_v16si(v16si_a, v16si_b, mask16);
    __m512i r6 = blend_v8di(v8di_a, v8di_b, mask8);
    __m512d r7 = blend_v8df(v8df_a, v8df_b, mask8);
    __m512 r8 = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Multi-stage pipelines
    int64_t pipeline1_result = pipeline_char_to_int(v64qi_a, v16si_a, mask64, mask16);
    float pipeline2_result = pipeline_half_to_float(v32hf_a, v16sf_a, mask32, mask16);
    
    // Data-dependent computation with loop
    volatile double loop_output[8];
    compute_blend_chain(100, loop_output);
    
    // Compute checksum from all results
    int64_t checksum = 0;
    
    // Extract and sum elements from each result
    char* cp = (char*)&r1;
    for (int i = 0; i < 64; i++) checksum += cp[i];
    
    short* sp = (short*)&r2;
    for (int i = 0; i < 32; i++) checksum += sp[i];
    
    // Skip half-precision direct access (type punning issues)
    
    int* ip = (int*)&r5;
    for (int i = 0; i < 16; i++) checksum += ip[i];
    
    long long* lp = (long long*)&r6;
    for (int i = 0; i < 8; i++) checksum += lp[i];
    
    checksum += pipeline1_result;
    checksum += (int64_t)pipeline2_result;
    
    for (int i = 0; i < 8; i++) checksum += (int64_t)loop_output[i];
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
