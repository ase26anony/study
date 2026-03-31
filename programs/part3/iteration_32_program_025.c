#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile globals to prevent optimization */
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512h v32hf_result;
volatile __m512bh v32bf_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512d v8df_result;
volatile __m512 v16sf_result;

/* Function prototypes with target attributes */
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

/* Multi-stage pipeline functions */
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_v64qi_to_v16si(uint8_t* data1, uint8_t* data2, int iterations);

__attribute__((target("avx512f,avx512fp16")))
float pipeline_v32hf_to_v16sf(_Float16* data1, _Float16* data2, int iterations);

/* Individual blend implementations */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion by using runtime mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Data-dependent computation to prevent folding */
    __m512i cmp = _mm512_cmpeq_epi8_mask(a, b);
    __m512i masked = _mm512_mask_blend_epi8(cmp, result, a);
    
    /* Store to volatile to force materialization */
    v64qi_result = masked;
    return masked;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    /* Create conditional mask from data comparison */
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(a, b);
    __mmask32 final_mask = mask ^ cmp_mask;  /* XOR to prevent constant folding */
    
    __m512i result = _mm512_mask_blend_epi16(final_mask, a, b);
    
    /* Additional operation to create dataflow */
    __m512i shifted = _mm512_slli_epi16(result, 1);
    result = _mm512_mask_blend_epi16(final_mask, result, shifted);
    
    v32hi_result = result;
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    /* Half-precision blend with type conversion stress */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Compare and create new mask */
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    __mmask32 alt_mask = mask & ~cmp_mask;
    
    /* Second blend with different mask */
    __m512h alt_result = _mm512_mask_blend_ph(alt_mask, result, a);
    
    /* Convert to float and back to stress mode handling */
    __m512 float_vec = _mm512_cvtph_ps(alt_result);
    __m512h final = _mm512_cvtps_ph(float_vec, _MM_FROUND_CUR_DIRECTION);
    
    v32hf_result = final;
    return final;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* Brain float16 blend */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Convert to float for computation */
    __m512 float_a = _mm512_cvtpbh_ps(a);
    __m512 float_b = _mm512_cvtpbh_ps(b);
    
    /* Create mask from float comparison */
    __mmask16 float_mask = _mm512_cmp_ps_mask(float_a, float_b, _CMP_LT_OQ);
    __mmask32 extended_mask = _mm512_kunpackw(float_mask, float_mask);
    
    /* Blend again with computed mask */
    __m512bh final = _mm512_mask_blend_ph(extended_mask & mask, result, a);
    
    v32bf_result = final;
    return final;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    /* Integer 32-bit blend with arithmetic */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Create alternating mask pattern */
    __m512i add_result = _mm512_add_epi32(a, b);
    __mmask16 alt_mask = _mm512_cmplt_epi32_mask(a, b);
    
    /* Conditional blend based on comparison */
    result = _mm512_mask_blend_epi32(alt_mask, result, add_result);
    
    v16si_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    /* 64-bit integer blend */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Create mask from high bits */
    __m512i shifted = _mm512_srli_epi64(a, 63);
    __mmask8 high_bit_mask = _mm512_cmpeq_epi64_mask(shifted, _mm512_set1_epi64(1));
    
    /* Blend with high-bit mask */
    result = _mm512_mask_blend_epi64(high_bit_mask, result, b);
    
    v8di_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    /* Double precision blend */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Create mask from comparison */
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    __mmask8 final_mask = mask | cmp_mask;
    
    /* Blend with computed mask */
    result = _mm512_mask_blend_pd(final_mask, result, _mm512_add_pd(a, b));
    
    v8df_result = result;
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    /* Single precision blend with conversions */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Convert to integer and back to stress mode handling */
    __m512i int_vec = _mm512_cvtps_epi32(result);
    __m512 float_again = _mm512_cvtepi32_ps(int_vec);
    
    /* Create new mask from converted values */
    __mmask16 new_mask = _mm512_cmp_ps_mask(float_again, a, _CMP_NEQ_UQ);
    result = _mm512_mask_blend_ps(new_mask, result, float_again);
    
    v16sf_result = result;
    return result;
}

/* Multi-stage pipeline: V64QI -> packed -> V16SI */
__attribute__((target("avx512bw,avx512f")))
uint64_t pipeline_v64qi_to_v16si(uint8_t* data1, uint8_t* data2, int iterations) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Load byte data */
        __m512i vec1 = _mm512_loadu_si512(data1 + i * 64);
        __m512i vec2 = _mm512_loadu_si512(data2 + i * 64);
        
        /* Create runtime-dependent mask */
        __mmask64 mask = 0;
        for (int j = 0; j < 64; j++) {
            if ((data1[i * 64 + j] ^ data2[i * 64 + j]) & 1) {
                mask |= (1ULL << j);
            }
        }
        
        /* V64QI blend */
        __m512i blended_bytes = blend_v64qi(vec1, vec2, mask);
        
        /* Pack bytes to 16-bit words */
        __m512i packed = _mm512_maddubs_epi16(blended_bytes, _mm512_set1_epi8(1));
        
        /* Pack words to 32-bit dwords */
        __m512i dwords = _mm512_madd_epi16(packed, _mm512_set1_epi16(1));
        
        /* Create mask for V16SI blend */
        __mmask16 si_mask = 0;
        for (int j = 0; j < 16; j++) {
            int32_t val1 = ((int32_t*)&dwords)[j];
            int32_t val2 = ((int32_t*)data1)[j % 16];
            if (val1 < val2) {
                si_mask |= (1 << j);
            }
        }
        
        /* V16SI blend */
        __m512i alt_dwords = _mm512_loadu_si512((__m512i*)(data1));
        __m512i blended_dwords = blend_v16si(dwords, alt_dwords, si_mask);
        
        /* Accumulate checksum */
        for (int j = 0; j < 16; j++) {
            checksum += ((int32_t*)&blended_dwords)[j];
        }
    }
    
    return checksum;
}

/* Half-precision to single-precision pipeline */
__attribute__((target("avx512f,avx512fp16")))
float pipeline_v32hf_to_v16sf(_Float16* data1, _Float16* data2, int iterations) {
    float checksum = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        /* Load half-precision data */
        __m512h vec1 = _mm512_loadu_ph(data1 + i * 32);
        __m512h vec2 = _mm512_loadu_ph(data2 + i * 32);
        
        /* Create mask from data */
        __mmask32 mask = 0;
        for (int j = 0; j < 32; j++) {
            if (data1[i * 32 + j] < data2[i * 32 + j]) {
                mask |= (1U << j);
            }
        }
        
        /* V32HF blend */
        __m512h blended_half = blend_v32hf(vec1, vec2, mask);
        
        /* Convert to single precision */
        __m512 singles = _mm512_cvtph_ps(blended_half);
        
        /* Create comparison mask */
        __m512 alt_singles = _mm512_set1_ps(0.5f);
        __mmask16 sf_mask = _mm512_cmp_ps_mask(singles, alt_singles, _CMP_GT_OQ);
        
        /* V16SF blend */
        __m512 blended_singles = blend_v16sf(singles, alt_singles, sf_mask);
        
        /* Horizontal add */
        checksum += _mm512_reduce_add_ps(blended_singles);
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    /* Use argc as seed for pseudo-random but reproducible data */
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    /* Allocate and initialize test data */
    uint8_t byte_data1[1024];
    uint8_t byte_data2[1024];
    _Float16 half_data1[1024];
    _Float16 half_data2[1024];
    
    for (int i = 0; i < 1024; i++) {
        byte_data1[i] = rand() % 256;
        byte_data2[i] = rand() % 256;
        half_data1[i] = (float)(rand() % 1000) / 1000.0f;
        half_data2[i] = (float)(rand() % 1000) / 1000.0f;
    }
    
    uint64_t int_checksum = 0;
    float float_checksum = 0.0f;
    
    /* Execute all blend functions with runtime masks */
    
    /* V64QI */
    __m512i v64qi_a = _mm512_loadu_si512(byte_data1);
    __m512i v64qi_b = _mm512_loadu_si512(byte_data2);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if (byte_data1[i] > byte_data2[i]) {
            mask64 |= (1ULL << i);
        }
    }
    blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    /* V32HI */
    __m512i v32hi_a = _mm512_loadu_si512(byte_data1);
    __m512i v32hi_b = _mm512_loadu_si512(byte_data1 + 64);
    __mmask32 mask32 = 0xAAAAAAAA;  /* Alternating pattern */
    blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    /* V32HF */
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_loadu_ph(half_data1);
    __m512h v32hf_b = _mm512_loadu_ph(half_data2);
    __mmask32 hf_mask = 0x55555555;  /* Alternating pattern */
    blend_v32hf(v32hf_a, v32hf_b, hf_mask);
    #endif
    
    /* V32BF */
    #ifdef __AVX512BF16__
    __m512bh v32bf_a = _mm512_loadu_ph((__m512bh*)half_data1);
    __m512bh v32bf_b = _mm512_loadu_ph((__m512bh*)half_data2);
    __mmask32 bf_mask = 0x33333333;  /* Different pattern */
    blend_v32bf(v32bf_a, v32bf_b, bf_mask);
    #endif
    
    /* V16SI */
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)byte_data1);
    __m512i v16si_b = _mm512_loadu_si512((__m512i*)byte_data2);
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if (((int32_t*)byte_data1)[i] != ((int32_t*)byte_data2)[i]) {
            mask16 |= (1 << i);
        }
    }
    blend_v16si(v16si_a, v16si_b, mask16);
    
    /* V8DI */
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)byte_data1);
    __m512i v8di_b = _mm512_loadu_si512((__m512i*)byte_data2);
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        if (((int64_t*)byte_data1)[i] < ((int64_t*)byte_data2)[i]) {
            mask8 |= (1 << i);
        }
    }
    blend_v8di(v8di_a, v8di_b, mask8);
    
    /* V8DF */
    double double_data1[8], double_data2[8];
    for (int i = 0; i < 8; i++) {
        double_data1[i] = (double)rand() / RAND_MAX;
        double_data2[i] = (double)rand() / RAND_MAX;
    }
    __m512d v8df_a = _mm512_loadu_pd(double_data1);
    __m512d v8df_b = _mm512_loadu_pd(double_data2);
    __mmask8 df_mask = 0xAA;  /* Alternating pattern */
    blend_v8df(v8df_a, v8df_b, df_mask);
    
    /* V16SF */
    float float_data1[16], float_data2[16];
    for (int i = 0; i < 16; i++) {
        float_data1[i] = (float)rand() / RAND_MAX;
        float_data2[i] = (float)rand() / RAND_MAX;
    }
    __m512 v16sf_a = _mm512_loadu_ps(float_data1);
    __m512 v16sf_b = _mm512_loadu_ps(float_data2);
    __mmask16 sf_mask = 0xAAAA;  /* Alternating pattern */
    blend_v16sf(v16sf_a, v16sf_b, sf_mask);
    
    /* Execute pipeline functions */
    int_checksum = pipeline_v64qi_to_v16si(byte_data1, byte_data2, 2);
    
    #ifdef __AVX512FP16__
    float_checksum = pipeline_v32hf_to_v16sf(half_data1, half_data2, 2);
    #endif
    
    printf("Integer checksum: %lu\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    return 0;
}
