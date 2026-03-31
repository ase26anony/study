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

// Data-dependent computation with runtime masks
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
    // Complex dataflow with multiple operations
    __m512i temp = _mm512_mask_blend_epi16(mask, a, b);
    
    // Data-dependent computation
    __m512i cmp = _mm512_cmpgt_epi16_mask(a, b);
    temp = _mm512_mask_blend_epi16(cmp, temp, _mm512_set1_epi16(42));
    
    v32hi_result = temp;
    return temp;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Half-precision blend
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Mixed precision operations
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    
    v32hf_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Brain float16 blend
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    // Type conversion stress
    __m512i int_vec = _mm512_castps_si512(_mm512_castph_ps(result));
    result = _mm512_castsi_ph(_mm512_add_epi32(int_vec, _mm512_set1_epi32(1)));
    
    v32bf_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Integer 32-bit blend
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Multi-stage pipeline: use result in another operation
    __m512d double_vec = _mm512_cvtepi32_pd(_mm512_extracti32x8_epi32(result, 0));
    __mmask8 double_mask = mask & 0xFF;
    double_vec = _mm512_mask_blend_pd(double_mask, double_vec, 
                                     _mm512_set1_pd(3.14159));
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Integer 64-bit blend
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Complex predicate computation
    __mmask8 cmp_mask = _mm512_cmpeq_epi64_mask(a, b);
    __mmask8 final_mask = mask ^ cmp_mask;
    result = _mm512_mask_blend_epi64(final_mask, result, 
                                    _mm512_set1_epi64(0xDEADBEEF));
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Double-precision blend
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Data-dependent computation with comparisons
    __mmask8 lt_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    result = _mm512_mask_blend_pd(lt_mask, result, _mm512_mul_pd(a, b));
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Single-precision blend
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Mixed-type operation chain
    __m512i int_result = _mm512_cvtps_epi32(result);
    result = _mm512_cvtepi32_ps(int_result);
    
    // Second blend with modified mask
    __mmask16 alt_mask = ~mask & 0xFFFF;
    result = _mm512_mask_blend_ps(alt_mask, result, _mm512_set1_ps(2.0f));
    
    v16sf_result = result;
    return result;
}

// Multi-stage pipeline function
__attribute__((target("avx512f,avx512bw")))
uint64_t multi_stage_pipeline(uint8_t* char_data, uint16_t* short_data,
                             int* int_data, uint64_t* long_data,
                             float* float_data, double* double_data,
                             int argc) {
    uint64_t checksum = 0;
    
    // Stage 1: V64QI blend
    __m512i v64qi_a = _mm512_loadu_si512(char_data);
    __m512i v64qi_b = _mm512_loadu_si512(char_data + 64);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        mask64 |= ((uint64_t)((char_data[i] + argc) % 2) << i);
    }
    __m512i v64qi_res = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Stage 2: V32HI blend (using packed results from stage 1)
    __m512i v32hi_a = _mm512_loadu_si512(short_data);
    __m512i v32hi_b = _mm512_loadu_si512(short_data + 32);
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        mask32 |= ((uint32_t)((short_data[i] + argc) % 3) << i);
    }
    __m512i v32hi_res = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Stage 3: V16SI blend
    __m512i v16si_a = _mm512_loadu_si512(int_data);
    __m512i v16si_b = _mm512_loadu_si512(int_data + 16);
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        mask16 |= ((uint16_t)((int_data[i] + argc) % 5) << i);
    }
    __m512i v16si_res = blend_v16si(v16si_a, v16si_b, mask16);
    
    // Stage 4: V8DI blend
    __m512i v8di_a = _mm512_loadu_si512(long_data);
    __m512i v8di_b = _mm512_loadu_si512(long_data + 8);
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        mask8 |= ((uint8_t)((long_data[i] + argc) % 7) << i);
    }
    __m512i v8di_res = blend_v8di(v8di_a, v8di_b, mask8);
    
    // Stage 5: V16SF blend
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 16);
    mask16 = 0;
    for (int i = 0; i < 16; i++) {
        mask16 |= ((uint16_t)(((int)float_data[i] + argc) % 11) << i);
    }
    __m512 v16sf_res = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Stage 6: V8DF blend
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 8);
    mask8 = 0;
    for (int i = 0; i < 8; i++) {
        mask8 |= ((uint8_t)(((int)double_data[i] + argc) % 13) << i);
    }
    __m512d v8df_res = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Combine checksums
    uint8_t* v64qi_bytes = (uint8_t*)&v64qi_res;
    for (int i = 0; i < 64; i++) checksum += v64qi_bytes[i];
    
    uint16_t* v32hi_shorts = (uint16_t*)&v32hi_res;
    for (int i = 0; i < 32; i++) checksum += v32hi_shorts[i];
    
    int* v16si_ints = (int*)&v16si_res;
    for (int i = 0; i < 16; i++) checksum += v16si_ints[i];
    
    uint64_t* v8di_longs = (uint64_t*)&v8di_res;
    for (int i = 0; i < 8; i++) checksum += v8di_longs[i];
    
    return checksum;
}

int main(int argc, char** argv) {
    // Initialize test data with pseudo-random values based on argc
    srand(argc);
    
    // Allocate and initialize test arrays
    uint8_t char_data[128];
    uint16_t short_data[64];
    int int_data[32];
    uint64_t long_data[16];
    float float_data[32];
    double double_data[16];
    
    for (int i = 0; i < 128; i++) char_data[i] = rand() % 256;
    for (int i = 0; i < 64; i++) short_data[i] = rand() % 65536;
    for (int i = 0; i < 32; i++) int_data[i] = rand();
    for (int i = 0; i < 16; i++) long_data[i] = ((uint64_t)rand() << 32) | rand();
    for (int i = 0; i < 32; i++) float_data[i] = (float)rand() / RAND_MAX;
    for (int i = 0; i < 16; i++) double_data[i] = (double)rand() / RAND_MAX;
    
    // Execute multi-stage pipeline
    uint64_t checksum = multi_stage_pipeline(char_data, short_data, int_data,
                                            long_data, float_data, double_data,
                                            argc);
    
    // Execute half-precision blends if supported
    #ifdef __AVX512FP16__
    {
        _Float16 half_data[64];
        for (int i = 0; i < 64; i++) half_data[i] = (_Float16)((float)rand() / RAND_MAX);
        
        __m512h v32hf_a = _mm512_loadu_ph(half_data);
        __m512h v32hf_b = _mm512_loadu_ph(half_data + 32);
        __mmask32 mask32 = 0;
        for (int i = 0; i < 32; i++) {
            mask32 |= ((uint32_t)(((int)half_data[i] + argc) % 17) << i);
        }
        blend_v32hf(v32hf_a, v32hf_b, mask32);
    }
    #endif
    
    // Execute bfloat16 blends if supported
    #ifdef __AVX512BF16__
    {
        __bf16 bfloat_data[64];
        for (int i = 0; i < 64; i++) {
            uint16_t val = rand() % 65536;
            memcpy(&bfloat_data[i], &val, sizeof(__bf16));
        }
        
        __m512bh v32bf_a = _mm512_loadu_bh(bfloat_data);
        __m512bh v32bf_b = _mm512_loadu_bh(bfloat_data + 32);
        __mmask32 mask32 = 0;
        for (int i = 0; i < 32; i++) {
            mask32 |= ((uint32_t)(((int)bfloat_data[i] + argc) % 19) << i);
        }
        blend_v32bf(v32bf_a, v32bf_b, mask32);
    }
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    
    // Use volatile results to prevent dead code elimination
    printf("Volatile results prevent optimization\n");
    
    return 0;
}
