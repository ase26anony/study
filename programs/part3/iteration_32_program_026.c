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

// Data-dependent blend functions with control flow to prevent constant folding

__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    // Force runtime mask usage
    __mmask64 dynamic_mask = mask ^ (__mmask64)((uintptr_t)&mask & 0xFF);
    
    // Use intrinsic with dynamic mask
    __m512i result = _mm512_mask_blend_epi8(dynamic_mask, a, b);
    
    // Store to volatile to prevent elimination
    v64qi_result = result;
    
    // Additional data-dependent computation
    __m512i temp = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    return _mm512_xor_si512(result, temp);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    // Create dynamic mask based on input values
    __mmask32 dynamic_mask = mask;
    for (int i = 0; i < 32; i++) {
        if ((_mm512_extract_epi16(a, i) & 1) == 0) {
            dynamic_mask ^= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi16(dynamic_mask, a, b);
    v32hi_result = result;
    
    // Mixed precision: convert to 32-bit for further operations
    __m512i extended = _mm512_cvtepi16_epi32(_mm512_extracti64x4_epi64(result, 0));
    return _mm512_add_epi32(result, extended);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    // Dynamic mask based on comparison
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    __mmask32 dynamic_mask = mask ^ cmp_mask;
    
    __m512h result = _mm512_mask_blend_ph(dynamic_mask, a, b);
    v32hf_result = result;
    
    // Convert to float for additional computation
    __m512 float_result = _mm512_cvtph_ps(result);
    return _mm512_cvtps_ph(float_result, _MM_FROUND_CUR_DIRECTION);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    // Use bfloat16 blend intrinsic
    __mmask32 dynamic_mask = mask | 0xAAAAAAAA; // Pattern to ensure non-constant
    
    __m512bh result = _mm512_mask_blend_ph(dynamic_mask, a, b);
    v32bf_result = result;
    
    // Convert to float for computation
    __m512 float_vec = _mm512_cvtpbh_ps(_mm512_castsi512_ph(_mm512_castph_si512(result)));
    __m512 scaled = _mm512_mul_ps(float_vec, _mm512_set1_ps(2.0f));
    return _mm512_cvtps_pbh(scaled);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    // Generate mask from comparison
    __mmask16 cmp_mask = _mm512_cmplt_epi32_mask(a, b);
    __mmask16 dynamic_mask = mask ^ cmp_mask;
    
    __m512i result = _mm512_mask_blend_epi32(dynamic_mask, a, b);
    v16si_result = result;
    
    // Multi-stage: use result in another blend
    __m512i temp = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    __mmask16 alt_mask = _mm512_cmpeq_epi32_mask(result, temp);
    return _mm512_mask_blend_epi32(alt_mask, result, temp);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    // Dynamic mask from arithmetic
    __mmask8 dynamic_mask = mask;
    for (int i = 0; i < 8; i++) {
        if ((_mm512_extract_epi64(a, i) % 3) == 0) {
            dynamic_mask ^= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi64(dynamic_mask, a, b);
    v8di_result = result;
    
    // Convert to double for mixed-type pipeline
    __m512d dbl_result = _mm512_cvtepi64_pd(result);
    __m512d scaled = _mm512_mul_pd(dbl_result, _mm512_set1_pd(0.5));
    return _mm512_cvtpd_epi64(scaled);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    // Mask from floating comparison
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 dynamic_mask = mask ^ cmp_mask;
    
    __m512d result = _mm512_mask_blend_pd(dynamic_mask, a, b);
    v8df_result = result;
    
    // Additional computation to prevent folding
    __m512d recip = _mm512_div_pd(_mm512_set1_pd(1.0), result);
    return _mm512_add_pd(result, recip);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    // Complex mask generation
    __mmask16 lt_mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    __mmask16 gt_mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    __mmask16 dynamic_mask = (mask & lt_mask) | (~mask & gt_mask);
    
    __m512 result = _mm512_mask_blend_ps(dynamic_mask, a, b);
    v16sf_result = result;
    
    // Use in transcendental function
    __m512 exp_result = _mm512_exp_ps(result); // Requires SVML or similar
    return _mm512_add_ps(result, exp_result);
}

// Multi-stage pipeline: process data through multiple blend types
__attribute__((target("avx512f,avx512bw")))
uint64_t process_pipeline(uint8_t* char_data, uint16_t* short_data, 
                         int32_t* int_data, int64_t* long_data,
                         float* float_data, double* double_data,
                         __fp16* half_data, __bf16* bf16_data) {
    uint64_t checksum = 0;
    
    // Stage 1: V64QI blend
    __m512i char_vec1 = _mm512_loadu_si512(char_data);
    __m512i char_vec2 = _mm512_loadu_si512(char_data + 64);
    __mmask64 char_mask = 0;
    for (int i = 0; i < 64; i++) {
        if (char_data[i] > 128) {
            char_mask |= (1ULL << i);
        }
    }
    __m512i blended_chars = blend_v64qi(char_vec1, char_vec2, char_mask);
    
    // Stage 2: V32HI blend (using packed chars)
    __m512i short_vec1 = _mm512_loadu_si512(short_data);
    __m512i short_vec2 = _mm512_loadu_si512(short_data + 32);
    __mmask32 short_mask = 0;
    for (int i = 0; i < 32; i++) {
        if (short_data[i] > 32768) {
            short_mask |= (1U << i);
        }
    }
    __m512i blended_shorts = blend_v32hi(short_vec1, short_vec2, short_mask);
    
    // Stage 3: V32HF blend
    __m512h half_vec1 = _mm512_loadu_ph(half_data);
    __m512h half_vec2 = _mm512_loadu_ph(half_data + 32);
    __mmask32 half_mask = 0x55555555; // Alternating pattern
    __m512h blended_half = blend_v32hf(half_vec1, half_vec2, half_mask);
    
    // Stage 4: V32BF blend
    __m512bh bf16_vec1 = _mm512_loadu_ph(bf16_data);
    __m512bh bf16_vec2 = _mm512_loadu_ph(bf16_data + 32);
    __mmask32 bf16_mask = 0xAAAAAAAA; // Alternating pattern (different)
    __m512bh blended_bf16 = blend_v32bf(bf16_vec1, bf16_vec2, bf16_mask);
    
    // Stage 5: V16SI blend
    __m512i int_vec1 = _mm512_loadu_si512(int_data);
    __m512i int_vec2 = _mm512_loadu_si512(int_data + 16);
    __mmask16 int_mask = 0;
    for (int i = 0; i < 16; i++) {
        if (int_data[i] < 0) {
            int_mask |= (1U << i);
        }
    }
    __m512i blended_ints = blend_v16si(int_vec1, int_vec2, int_mask);
    
    // Stage 6: V8DI blend
    __m512i long_vec1 = _mm512_loadu_si512(long_data);
    __m512i long_vec2 = _mm512_loadu_si512(long_data + 8);
    __mmask8 long_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (long_data[i] % 2 == 0) {
            long_mask |= (1U << i);
        }
    }
    __m512i blended_longs = blend_v8di(long_vec1, long_vec2, long_mask);
    
    // Stage 7: V8DF blend
    __m512d double_vec1 = _mm512_loadu_pd(double_data);
    __m512d double_vec2 = _mm512_loadu_pd(double_data + 8);
    __mmask8 double_mask = 0xFF; // All bits set initially
    __m512d blended_doubles = blend_v8df(double_vec1, double_vec2, double_mask);
    
    // Stage 8: V16SF blend
    __m512 float_vec1 = _mm512_loadu_ps(float_data);
    __m512 float_vec2 = _mm512_loadu_ps(float_data + 16);
    __mmask16 float_mask = 0xFFFF; // All bits set
    __m512 blended_floats = blend_v16sf(float_vec1, float_vec2, float_mask);
    
    // Accumulate checksum from all results
    uint8_t* char_ptr = (uint8_t*)&blended_chars;
    for (int i = 0; i < 64; i++) {
        checksum += char_ptr[i];
    }
    
    uint16_t* short_ptr = (uint16_t*)&blended_shorts;
    for (int i = 0; i < 32; i++) {
        checksum += short_ptr[i];
    }
    
    // Horizontal adds for verification
    checksum += _mm512_reduce_add_epi32(blended_ints);
    checksum += _mm512_reduce_add_epi64(blended_longs);
    
    return checksum;
}

int main(int argc, char** argv) {
    // Use argc as seed for pseudo-random but deterministic data
    unsigned int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    // Allocate and initialize test data
    uint8_t char_data[128];
    uint16_t short_data[64];
    int32_t int_data[32];
    int64_t long_data[16];
    float float_data[32];
    double double_data[16];
    __fp16 half_data[64];
    __bf16 bf16_data[64];
    
    // Fill with pseudo-random data
    for (int i = 0; i < 128; i++) {
        char_data[i] = rand() % 256;
    }
    for (int i = 0; i < 64; i++) {
        short_data[i] = rand() % 65536;
        half_data[i] = (__fp16)(rand() / (float)RAND_MAX);
        bf16_data[i] = (__bf16)(rand() / (float)RAND_MAX);
    }
    for (int i = 0; i < 32; i++) {
        int_data[i] = rand() - RAND_MAX/2;
        float_data[i] = (rand() / (float)RAND_MAX) * 100.0f - 50.0f;
    }
    for (int i = 0; i < 16; i++) {
        long_data[i] = ((int64_t)rand() << 32) | rand();
        double_data[i] = (rand() / (double)RAND_MAX) * 200.0 - 100.0;
    }
    
    // Run the pipeline multiple times with different masks
    uint64_t total_checksum = 0;
    for (int iter = 0; iter < 3; iter++) {
        // Modify data slightly each iteration
        for (int i = 0; i < 128; i++) {
            char_data[i] = (char_data[i] + iter) % 256;
        }
        
        total_checksum += process_pipeline(char_data, short_data, int_data,
                                          long_data, float_data, double_data,
                                          half_data, bf16_data);
    }
    
    printf("Final checksum: %lu\n", total_checksum);
    return 0;
}
