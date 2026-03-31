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
int pipeline_64qi_to_16si(unsigned int seed);

__attribute__((target("avx512bw,avx512f,avx512fp16")))
float pipeline_mixed_precision(unsigned int seed);

/* Data-dependent computation with loops */
__attribute__((target("avx512bw")))
__m512i conditional_blend_64qi(const char* data1, const char* data2, int len);

/* Implementation of blend_v64qi - triggers E_V64QImode case */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion by using runtime mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store to volatile to prevent elimination */
    v64qi_result = result;
    
    /* Additional computation to create dataflow */
    result = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    
    return result;
}

/* Implementation of blend_v32hi - triggers E_V32HImode case */
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    v32hi_result = result;
    
    /* Create complex dataflow with comparisons */
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    result = _mm512_mask_blend_epi16(cmp_mask, result, _mm512_set1_epi16(-1));
    
    return result;
}

/* Implementation of blend_v32hf - triggers E_V32HFmode case */
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    v32hf_result = result;
    
    /* Mixed precision operations */
    result = _mm512_add_ph(result, _mm512_set1_ph(1.0f));
    
    return result;
}

/* Implementation of blend_v32bf - triggers E_V32BFmode case */
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    v32bf_result = result;
    
    return result;
}

/* Implementation of blend_v16si - triggers E_V16SImode case */
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    v16si_result = result;
    
    /* Type conversion before blending */
    __m512 float_vec = _mm512_cvtepi32_ps(result);
    __m512 float_blend = _mm512_blend_ps(float_vec, _mm512_set1_ps(0.5f), 0xAAAA);
    result = _mm512_cvtps_epi32(float_blend);
    
    return result;
}

/* Implementation of blend_v8di - triggers E_V8DImode case */
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    v8di_result = result;
    
    /* Chain blends with different masks */
    __mmask8 alt_mask = mask ^ 0xFF;
    result = _mm512_mask_blend_epi64(alt_mask, result, _mm512_slli_epi64(result, 1));
    
    return result;
}

/* Implementation of blend_v8df - triggers E_V8DFmode case */
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    v8df_result = result;
    
    /* Data-dependent computation */
    __m512d cmp = _mm512_cmp_pd_mask(a, b, _CMP_LT_OS);
    result = _mm512_mask_blend_pd(cmp, result, _mm512_mul_pd(result, result));
    
    return result;
}

/* Implementation of blend_v16sf - triggers E_V16SFmode case */
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    v16sf_result = result;
    
    /* Complex floating-point pipeline */
    __m512 sqrt_val = _mm512_sqrt_ps(result);
    __mmask16 nan_mask = _mm512_cmp_ps_mask(result, result, _CMP_EQ_OQ);
    result = _mm512_mask_blend_ps(nan_mask, sqrt_val, result);
    
    return result;
}

/* Multi-stage pipeline: 64QI -> 32HI -> 16SI */
__attribute__((target("avx512bw,avx512f")))
int pipeline_64qi_to_16si(unsigned int seed) {
    /* Stage 1: V64QI blending */
    char data1[64], data2[64];
    for (int i = 0; i < 64; i++) {
        data1[i] = (char)((seed + i) % 256);
        data2[i] = (char)((seed * i) % 256);
    }
    
    __m512i vec1 = _mm512_loadu_si512(data1);
    __m512i vec2 = _mm512_loadu_si512(data2);
    
    /* Runtime-derived mask */
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if ((data1[i] + data2[i]) % 2) {
            mask64 |= (1ULL << i);
        }
    }
    
    __m512i blended_64qi = blend_v64qi(vec1, vec2, mask64);
    
    /* Stage 2: Convert to V32HI and blend */
    __m512i vec1_32hi = _mm512_cvtepi8_epi16(_mm512_castsi512_si256(blended_64qi));
    __m512i vec2_32hi = _mm512_srai_epi16(vec1_32hi, 1);
    
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        mask32 |= ((seed >> i) & 1) << i;
    }
    
    __m512i blended_32hi = blend_v32hi(vec1_32hi, vec2_32hi, mask32);
    
    /* Stage 3: Convert to V16SI and blend */
    __m512i vec1_16si = _mm512_cvtepi16_epi32(_mm512_castsi512_si256(blended_32hi));
    __m512i vec2_16si = _mm512_slli_epi32(vec1_16si, 1);
    
    __mmask16 mask16 = (__mmask16)(seed & 0xFFFF);
    
    __m512i blended_16si = blend_v16si(vec1_16si, vec2_16si, mask16);
    
    /* Horizontal sum */
    return _mm512_reduce_add_epi32(blended_16si);
}

/* Mixed precision pipeline */
__attribute__((target("avx512bw,avx512f,avx512fp16")))
float pipeline_mixed_precision(unsigned int seed) {
    /* Start with integer */
    int data[16];
    for (int i = 0; i < 16; i++) {
        data[i] = seed * i;
    }
    
    __m512i int_vec = _mm512_loadu_si512(data);
    
    /* Convert to float and blend */
    __m512 float_vec1 = _mm512_cvtepi32_ps(int_vec);
    __m512 float_vec2 = _mm512_set1_ps(seed * 0.1f);
    
    __mmask16 mask = (__mmask16)((seed * 0xCCCC) & 0xFFFF);
    __m512 blended_sf = blend_v16sf(float_vec1, float_vec2, mask);
    
    /* Convert to double and blend */
    __m512d double_vec1 = _mm512_cvtps_pd(_mm512_castps512_ps256(blended_sf));
    __m512d double_vec2 = _mm512_set1_pd(seed * 0.01);
    
    __mmask8 dbl_mask = (__mmask8)(seed & 0xFF);
    __m512d blended_df = blend_v8df(double_vec1, double_vec2, dbl_mask);
    
    /* Horizontal sum */
    return (float)_mm512_reduce_add_pd(blended_df);
}

/* Data-dependent loop-based blending */
__attribute__((target("avx512bw")))
__m512i conditional_blend_64qi(const char* data1, const char* data2, int len) {
    __m512i result = _mm512_setzero_si512();
    
    for (int i = 0; i < len; i += 64) {
        __m512i vec1 = _mm512_loadu_si512(data1 + i);
        __m512i vec2 = _mm512_loadu_si512(data2 + i);
        
        /* Compute mask based on data comparison */
        __mmask64 mask = _mm512_cmpgt_epi8_mask(vec1, vec2);
        
        /* Force runtime mask modification */
        mask ^= (i & 0x3F);
        
        result = blend_v64qi(vec1, vec2, mask);
    }
    
    return result;
}

int main(int argc, char** argv) {
    unsigned int seed = (unsigned int)(argc > 1 ? atoi(argv[1]) : 12345);
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    printf("Seed: %u\n", seed);
    
    /* Initialize test data */
    char char_data1[64], char_data2[64];
    short short_data1[32], short_data2[32];
    int int_data1[16], int_data2[16];
    long long long_data1[8], long_data2[8];
    float float_data1[16], float_data2[16];
    double double_data1[8], double_data2[8];
    
    for (int i = 0; i < 64; i++) {
        char_data1[i] = (char)((seed + i * 3) % 256 - 128);
        char_data2[i] = (char)((seed * i + 7) % 256 - 128);
    }
    
    for (int i = 0; i < 32; i++) {
        short_data1[i] = (short)((seed * i) % 65536 - 32768);
        short_data2[i] = (short)((seed + i * 5) % 65536 - 32768);
    }
    
    for (int i = 0; i < 16; i++) {
        int_data1[i] = seed * i * i;
        int_data2[i] = seed + i * 100;
        float_data1[i] = (float)(seed * 0.01 * i);
        float_data2[i] = (float)(seed * 0.001 * i * i);
    }
    
    for (int i = 0; i < 8; i++) {
        long_data1[i] = (long long)seed * i * 1000;
        long_data2[i] = (long long)seed * 100 + i;
        double_data1[i] = seed * 0.0001 * i;
        double_data2[i] = seed * 0.00001 * i * i;
    }
    
    int checksum = 0;
    
    /* Test V64QI blend */
    __m512i v64qi_1 = _mm512_loadu_si512(char_data1);
    __m512i v64qi_2 = _mm512_loadu_si512(char_data2);
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        mask64 |= ((char_data1[i] > char_data2[i]) ? 1ULL : 0ULL) << i;
    }
    __m512i res64qi = blend_v64qi(v64qi_1, v64qi_2, mask64);
    
    /* Test V32HI blend */
    __m512i v32hi_1 = _mm512_loadu_si512(short_data1);
    __m512i v32hi_2 = _mm512_loadu_si512(short_data2);
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        mask32 |= ((short_data1[i] % 2) ? 1U : 0U) << i;
    }
    __m512i res32hi = blend_v32hi(v32hi_1, v32hi_2, mask32);
    
    /* Test V16SI blend */
    __m512i v16si_1 = _mm512_loadu_si512(int_data1);
    __m512i v16si_2 = _mm512_loadu_si512(int_data2);
    __mmask16 mask16 = (__mmask16)(seed & 0xFFFF);
    __m512i res16si = blend_v16si(v16si_1, v16si_2, mask16);
    
    /* Test V8DI blend */
    __m512i v8di_1 = _mm512_loadu_si512(long_data1);
    __m512i v8di_2 = _mm512_loadu_si512(long_data2);
    __mmask8 mask8 = (__mmask8)(seed & 0xFF);
    __m512i res8di = blend_v8di(v8di_1, v8di_2, mask8);
    
    /* Test V16SF blend */
    __m512 v16sf_1 = _mm512_loadu_ps(float_data1);
    __m512 v16sf_2 = _mm512_loadu_ps(float_data2);
    __mmask16 mask16f = (__mmask16)((seed * 0xAAAA) & 0xFFFF);
    __m512 res16sf = blend_v16sf(v16sf_1, v16sf_2, mask16f);
    
    /* Test V8DF blend */
    __m512d v8df_1 = _mm512_loadu_pd(double_data1);
    __m512d v8df_2 = _mm512_loadu_pd(double_data2);
    __mmask8 mask8d = (__mmask8)((seed * 0x55) & 0xFF);
    __m512d res8df = blend_v8df(v8df_1, v8df_2, mask8d);
    
    /* Run pipeline tests */
    int pipeline_result = pipeline_64qi_to_16si(seed);
    float mixed_result = pipeline_mixed_precision(seed);
    
    /* Run conditional blend */
    __m512i cond_result = conditional_blend_64qi(char_data1, char_data2, 64);
    
    /* Compute final checksum */
    checksum += _mm512_reduce_add_epi32(res64qi);
    checksum += _mm512_reduce_add_epi32(res32hi);
    checksum += _mm512_reduce_add_epi32(res16si);
    checksum += (int)_mm512_reduce_add_epi64(res8di);
    checksum += (int)_mm512_reduce_add_ps(res16sf);
    checksum += (int)_mm512_reduce_add_pd(res8df);
    checksum += pipeline_result;
    checksum += (int)mixed_result;
    checksum += _mm512_reduce_add_epi32(cond_result);
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
