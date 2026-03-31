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

/* Function 1: V64QI blend using _mm512_mask_blend_epi8 */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion by using runtime mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store to volatile to prevent elimination */
    v64qi_result = result;
    
    /* Additional computation to create data dependency */
    __m512i shuffled = _mm512_shuffle_epi8(result, _mm512_set1_epi8(0x07));
    return _mm512_add_epi8(result, shuffled);
}

/* Function 2: V32HI blend using _mm512_mask_blend_epi16 */
__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    v32hi_result = result;
    
    /* Create complex dataflow */
    __m512i shifted = _mm512_slli_epi16(result, 3);
    return _mm512_xor_si512(result, shifted);
}

/* Function 3: V32HF blend using _mm512_mask_blend_ph */
__attribute__((target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    v32hf_result = result;
    
    /* FP16 arithmetic to prevent constant folding */
    __m512h scaled = _mm512_mul_ph(result, _mm512_set1_ph(1.5f));
    return _mm512_add_ph(result, scaled);
}

/* Function 4: V32BF blend using _mm512_mask_blend_ph with bf16 */
__attribute__((target("avx512bf16,avx512f")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* Convert to __m512h for blending, then back */
    __m512h a_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(), 
                                     _mm512_castsi512_ps(_mm512_castbh_si512(a)));
    __m512h b_h = _mm512_cvtne2ps_pbh(_mm512_setzero_ps(),
                                     _mm512_castsi512_ps(_mm512_castbh_si512(b)));
    
    __m512h blended_h = _mm512_mask_blend_ph(mask, a_h, b_h);
    
    /* Convert back to bf16 */
    __m512bh result = _mm512_cvtneps_pbh(_mm512_castph_ps(blended_h));
    v32bf_result = result;
    
    return result;
}

/* Function 5: V16SI blend using _mm512_mask_blend_epi32 */
__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    v16si_result = result;
    
    /* Multi-stage computation */
    __m512i abs_val = _mm512_abs_epi32(result);
    __m512i negated = _mm512_sub_epi32(_mm512_setzero_si512(), result);
    return _mm512_mask_blend_epi32(mask, abs_val, negated);
}

/* Function 6: V8DI blend using _mm512_mask_blend_epi64 */
__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    v8di_result = result;
    
    /* Chain blends for complex dataflow */
    __m512i rotated = _mm512_rorv_epi64(result, _mm512_set1_epi64(13));
    return _mm512_mask_blend_epi64(mask ^ 0xFF, result, rotated);
}

/* Function 7: V8DF blend using _mm512_mask_blend_pd */
__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    v8df_result = result;
    
    /* Floating-point arithmetic chain */
    __m512d squared = _mm512_mul_pd(result, result);
    __m512d sqrt_val = _mm512_sqrt_pd(_mm512_add_pd(result, _mm512_set1_pd(1.0)));
    return _mm512_mask_blend_pd(mask, squared, sqrt_val);
}

/* Function 8: V16SF blend using _mm512_mask_blend_ps */
__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    v16sf_result = result;
    
    /* Trigonometric computation to prevent folding */
    __m512 sin_val = _mm512_sin_ps(result);
    __m512 cos_val = _mm512_cos_ps(result);
    return _mm512_mask_blend_ps(mask, sin_val, cos_val);
}

/* Multi-stage pipeline: V64QI -> V16SI conversion and blending */
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(uint8_t* data) {
    /* Load as V64QI */
    __m512i v64qi = _mm512_loadu_si512((__m512i*)data);
    
    /* Create dynamic mask based on data values */
    __mmask64 mask64 = 0;
    for (int i = 0; i < 64; i++) {
        if (data[i] > 128) mask64 |= (1ULL << i);
    }
    
    /* First blend in V64QI mode */
    __m512i blended_qi = blend_v64qi(v64qi, 
                                    _mm512_set1_epi8(0x7F), 
                                    mask64);
    
    /* Convert to V16SI for second blend stage */
    __m512i v16si_low = _mm512_cvtepu8_epi32(_mm512_castsi512_si256(blended_qi));
    __m512i v16si_high = _mm512_cvtepu8_epi32(_mm512_extracti64x4_epi64(blended_qi, 1));
    
    /* Create mask for V16SI blend */
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if (((int32_t*)&blended_qi)[i] > 0) mask16 |= (1 << i);
    }
    
    /* Second blend in V16SI mode */
    __m512i blended_si = blend_v16si(v16si_low, v16si_high, mask16);
    
    /* Horizontal sum for verification */
    return _mm512_reduce_add_epi32(blended_si);
}

/* Mixed precision pipeline: V32HF -> V16SF */
__attribute__((target("avx512fp16,avx512f")))
float pipeline_v32hf_to_v16sf(uint16_t* hdata, float* fdata) {
    /* Load half-precision */
    __m512h v32hf = _mm512_loadu_ph(hdata);
    
    /* Create mask from data pattern */
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        if ((hdata[i] & 0x8000) == 0) mask32 |= (1U << i);  /* Positive values */
    }
    
    /* Blend in V32HF mode */
    __m512h blended_hf = blend_v32hf(v32hf, 
                                    _mm512_set1_ph(0.0f), 
                                    mask32);
    
    /* Convert to single precision */
    __m512 v16sf_low = _mm512_cvtph_ps(_mm512_castph_si256(blended_hf));
    __m512 v16sf_high = _mm512_cvtph_ps(_mm512_extractf32x8_ps(_mm512_castph_ps(blended_hf), 1));
    
    /* Load float data for second blend */
    __m512 v16sf_a = _mm512_loadu_ps(fdata);
    __m512 v16sf_b = _mm512_loadu_ps(fdata + 16);
    
    /* Create mask for V16SF blend */
    __mmask16 mask16 = 0;
    for (int i = 0; i < 16; i++) {
        if (fdata[i] > fdata[i + 16]) mask16 |= (1 << i);
    }
    
    /* Blend in V16SF mode */
    __m512 blended_sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    /* Horizontal sum */
    return _mm512_reduce_add_ps(blended_sf);
}

int main(int argc, char** argv) {
    /* Use argc as seed for pseudo-random but deterministic data */
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    /* Initialize test data arrays */
    uint8_t data_8bit[64];
    uint16_t data_16bit[32];
    uint16_t data_half[32];  /* FP16 */
    uint16_t data_bf16[32];  /* BF16 */
    int32_t data_32bit[16];
    int64_t data_64bit[8];
    double data_double[8];
    float data_float[32];
    
    for (int i = 0; i < 64; i++) data_8bit[i] = rand() % 256;
    for (int i = 0; i < 32; i++) data_16bit[i] = rand() % 65536;
    for (int i = 0; i < 32; i++) data_half[i] = rand() % 65536;  /* Random FP16 pattern */
    for (int i = 0; i < 32; i++) data_bf16[i] = rand() % 65536;  /* Random BF16 pattern */
    for (int i = 0; i < 16; i++) data_32bit[i] = rand();
    for (int i = 0; i < 8; i++) data_64bit[i] = ((int64_t)rand() << 32) | rand();
    for (int i = 0; i < 8; i++) data_double[i] = (double)rand() / RAND_MAX;
    for (int i = 0; i < 32; i++) data_float[i] = (float)rand() / RAND_MAX;
    
    /* Create dynamic masks from runtime data */
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) mask64 |= (1ULL << i);
    }
    for (int i = 0; i < 32; i++) {
        if (i % 2 == 0) mask32 |= (1U << i);
    }
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) mask16 |= (1 << i);
    }
    for (int i = 0; i < 8; i++) {
        if (i % 3 == 0) mask8 |= (1 << i);
    }
    
    /* Call all blend functions with runtime data */
    __m512i v64qi_a = _mm512_loadu_si512((__m512i*)data_8bit);
    __m512i v64qi_b = _mm512_set1_epi8(0xAA);
    blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    __m512i v32hi_a = _mm512_loadu_si512((__m512i*)data_16bit);
    __m512i v32hi_b = _mm512_set1_epi16(0x5555);
    blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    __m512h v32hf_a = _mm512_loadu_ph(data_half);
    __m512h v32hf_b = _mm512_set1_ph(1.0f);
    blend_v32hf(v32hf_a, v32hf_b, mask32);
    
    __m512bh v32bf_a = _mm512_loadu_bh(data_bf16);
    __m512bh v32bf_b = _mm512_set1_bh(0);
    blend_v32bf(v32bf_a, v32bf_b, mask32);
    
    __m512i v16si_a = _mm512_loadu_si512((__m512i*)data_32bit);
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    blend_v16si(v16si_a, v16si_b, mask16);
    
    __m512i v8di_a = _mm512_loadu_si512((__m512i*)data_64bit);
    __m512i v8di_b = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAA);
    blend_v8di(v8di_a, v8di_b, mask8);
    
    __m512d v8df_a = _mm512_loadu_pd(data_double);
    __m512d v8df_b = _mm512_set1_pd(3.1415926535);
    blend_v8df(v8df_a, v8df_b, mask8);
    
    __m512 v16sf_a = _mm512_loadu_ps(data_float);
    __m512 v16sf_b = _mm512_set1_ps(2.7182818284f);
    blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    /* Execute multi-stage pipelines */
    int64_t pipeline_result1 = pipeline_v64qi_to_v16si(data_8bit);
    float pipeline_result2 = pipeline_v32hf_to_v16sf(data_half, data_float);
    
    /* Compute checksum from all volatile results */
    uint64_t checksum = 0;
    uint8_t* ptr;
    
    ptr = (uint8_t*)&v64qi_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v32hi_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v32hf_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    ptr = (uint8_t*)&v32bf_result;
    for (int i = 0; i < 64; i++) checksum += ptr[i];
    
    printf("Final checksum: %lu\n", checksum);
    printf("Pipeline results: %ld, %f\n", pipeline_result1, pipeline_result2);
    
    return 0;
}
