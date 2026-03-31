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

/* Data-dependent computation functions */
__attribute__((target("avx512bw")))
__m512i compute_v64qi_mask(__m512i a, __m512i b) {
    /* Generate mask based on comparison */
    return _mm512_cmpgt_epi8_mask(a, b);
}

__attribute__((target("avx512bw")))
__m512i compute_v32hi_mask(__m512i a, __m512i b) {
    /* Generate mask based on comparison */
    return _mm512_cmpgt_epi16_mask(a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h compute_v32hf_mask(__m512h a, __m512h b) {
    /* Generate mask based on comparison */
    return _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh compute_v32bf_mask(__m512bh a, __m512bh b) {
    /* Convert to float for comparison */
    __m512 a_f = _mm512_cvtpbh_ps(a);
    __m512 b_f = _mm512_cvtpbh_ps(b);
    return _mm512_cmp_ps_mask(a_f, b_f, _CMP_GT_OQ);
}

__attribute__((target("avx512f")))
__m512i compute_v16si_mask(__m512i a, __m512i b) {
    return _mm512_cmpgt_epi32_mask(a, b);
}

__attribute__((target("avx512f")))
__m512i compute_v8di_mask(__m512i a, __m512i b) {
    return _mm512_cmpgt_epi64_mask(a, b);
}

__attribute__((target("avx512f")))
__m512d compute_v8df_mask(__m512d a, __m512d b) {
    return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
}

__attribute__((target("avx512f")))
__m512 compute_v16sf_mask(__m512 a, __m512 b) {
    return _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
}

/* Blend implementations - each triggers a specific case */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Force RTL expansion for V64QImode */
    return _mm512_mask_blend_epi8(mask, a, b);
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    /* Force RTL expansion for V32HImode */
    return _mm512_mask_blend_epi16(mask, a, b);
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    /* Force RTL expansion for V32HFmode */
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* Force RTL expansion for V32BFmode */
    return _mm512_mask_blend_ph(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    /* Force RTL expansion for V16SImode */
    return _mm512_mask_blend_epi32(mask, a, b);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    /* Force RTL expansion for V8DImode */
    return _mm512_mask_blend_epi64(mask, a, b);
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    /* Force RTL expansion for V8DFmode */
    return _mm512_mask_blend_pd(mask, a, b);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    /* Force RTL expansion for V16SFmode */
    return _mm512_mask_blend_ps(mask, a, b);
}

/* Multi-stage pipeline: V64QI -> V16SI conversion and blending */
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(uint8_t* data1, uint8_t* data2, int argc) {
    __m512i a = _mm512_loadu_si512((__m512i*)data1);
    __m512i b = _mm512_loadu_si512((__m512i*)data2);
    
    /* Generate runtime-dependent mask */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((uint64_t)((data1[i] + argc) > data2[i]) << i);
    }
    
    /* First blend: V64QI */
    __m512i blended_qi = blend_v64qi(a, b, mask);
    v64qi_result = blended_qi;
    
    /* Convert to V16SI for second blend stage */
    __m512i a_si = _mm512_cvtepu8_epi32(_mm512_castsi512_si256(blended_qi));
    __m512i b_si = _mm512_slli_epi32(a_si, 1);
    
    /* Generate mask for V16SI blend */
    __mmask16 mask_si = compute_v16si_mask(a_si, b_si);
    
    /* Second blend: V16SI */
    __m512i blended_si = blend_v16si(a_si, b_si, mask_si);
    v16si_result = blended_si;
    
    /* Horizontal sum for checksum */
    return _mm512_reduce_add_epi32(blended_si);
}

/* Mixed precision pipeline: V32HF -> V32BF -> V16SF */
__attribute__((target("avx512bw,avx512fp16,avx512bf16,avx512f")))
float pipeline_mixed_precision(uint16_t* hdata1, uint16_t* hdata2, 
                               __bf16* bfdata1, __bf16* bfdata2,
                               float* fdata1, float* fdata2, int argc) {
    /* V32HF blend */
    __m512h a_hf = _mm512_loadu_ph(hdata1);
    __m512h b_hf = _mm512_loadu_ph(hdata2);
    __mmask32 mask_hf = compute_v32hf_mask(a_hf, b_hf);
    __m512h blended_hf = blend_v32hf(a_hf, b_hf, mask_hf);
    v32hf_result = blended_hf;
    
    /* V32BF blend */
    __m512bh a_bf = _mm512_loadu_si512((__m512bh*)bfdata1);
    __m512bh b_bf = _mm512_loadu_si512((__m512bh*)bfdata2);
    __mmask32 mask_bf = compute_v32bf_mask(a_bf, b_bf);
    __m512bh blended_bf = blend_v32bf(a_bf, b_bf, mask_bf);
    v32bf_result = blended_bf;
    
    /* Convert BF16 to FP32 and blend */
    __m512 a_f1 = _mm512_cvtpbh_ps(blended_bf);
    __m512 b_f1 = _mm512_loadu_ps(fdata1);
    __mmask16 mask_f1 = compute_v16sf_mask(a_f1, b_f1);
    __m512 blended_f1 = blend_v16sf(a_f1, b_f1, mask_f1);
    
    /* V16SF blend with different data */
    __m512 a_f2 = _mm512_loadu_ps(fdata1);
    __m512 b_f2 = _mm512_loadu_ps(fdata2);
    __mmask16 mask_f2 = compute_v16sf_mask(a_f2, b_f2);
    __m512 blended_f2 = blend_v16sf(a_f2, b_f2, mask_f2);
    v16sf_result = blended_f2;
    
    /* Horizontal sum */
    return _mm512_reduce_add_ps(_mm512_add_ps(blended_f1, blended_f2));
}

/* V8DI and V8DF pipeline */
__attribute__((target("avx512f")))
double pipeline_v8di_v8df(int64_t* idata1, int64_t* idata2,
                          double* ddata1, double* ddata2, int argc) {
    /* V8DI blend */
    __m512i a_di = _mm512_loadu_si512((__m512i*)idata1);
    __m512i b_di = _mm512_loadu_si512((__m512i*)idata2);
    __mmask8 mask_di = compute_v8di_mask(a_di, b_di);
    __m512i blended_di = blend_v8di(a_di, b_di, mask_di);
    v8di_result = blended_di;
    
    /* V8DF blend */
    __m512d a_df = _mm512_loadu_pd(ddata1);
    __m512d b_df = _mm512_loadu_pd(ddata2);
    __mmask8 mask_df = compute_v8df_mask(a_df, b_df);
    __m512d blended_df = blend_v8df(a_df, b_df, mask_df);
    v8df_result = blended_df;
    
    /* Convert V8DI to V8DF and add */
    __m512d di_as_df = _mm512_cvtepi64_pd(blended_di);
    return _mm512_reduce_add_pd(_mm512_add_pd(blended_df, di_as_df));
}

/* V32HI standalone test */
__attribute__((target("avx512bw")))
int32_t test_v32hi(int16_t* data1, int16_t* data2, int argc) {
    __m512i a = _mm512_loadu_si512((__m512i*)data1);
    __m512i b = _mm512_loadu_si512((__m512i*)data2);
    
    /* Runtime-dependent mask */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((uint32_t)((data1[i] + argc) > data2[i]) << i);
    }
    
    __m512i blended = blend_v32hi(a, b, mask);
    v32hi_result = blended;
    
    /* Horizontal sum */
    return _mm512_reduce_add_epi16(blended);
}

int main(int argc, char** argv) {
    /* Initialize with argc-dependent seed */
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    /* Allocate and initialize test data */
    uint8_t data8_1[64], data8_2[64];
    int16_t data16_1[32], data16_2[32];
    uint16_t data16u_1[32], data16u_2[32];
    __bf16 bfdata1[32], bfdata2[32];
    int32_t data32_1[16], data32_2[16];
    int64_t data64_1[8], data64_2[8];
    float dataf_1[16], dataf_2[16];
    double datad_1[8], datad_2[8];
    
    for (int i = 0; i < 64; i++) {
        data8_1[i] = rand() % 256;
        data8_2[i] = rand() % 256;
    }
    for (int i = 0; i < 32; i++) {
        data16_1[i] = rand() % 65536 - 32768;
        data16_2[i] = rand() % 65536 - 32768;
        data16u_1[i] = rand() % 65536;
        data16u_2[i] = rand() % 65536;
        bfdata1[i] = (__bf16)(rand() / (float)RAND_MAX);
        bfdata2[i] = (__bf16)(rand() / (float)RAND_MAX);
    }
    for (int i = 0; i < 16; i++) {
        data32_1[i] = rand();
        data32_2[i] = rand();
        dataf_1[i] = rand() / (float)RAND_MAX;
        dataf_2[i] = rand() / (float)RAND_MAX;
    }
    for (int i = 0; i < 8; i++) {
        data64_1[i] = ((int64_t)rand() << 32) | rand();
        data64_2[i] = ((int64_t)rand() << 32) | rand();
        datad_1[i] = rand() / (double)RAND_MAX;
        datad_2[i] = rand() / (double)RAND_MAX;
    }
    
    /* Execute all blend pipelines */
    int64_t sum1 = pipeline_v64qi_to_v16si(data8_1, data8_2, argc);
    int32_t sum2 = test_v32hi(data16_1, data16_2, argc);
    float sum3 = pipeline_mixed_precision(data16u_1, data16u_2,
                                         bfdata1, bfdata2,
                                         dataf_1, dataf_2, argc);
    double sum4 = pipeline_v8di_v8df(data64_1, data64_2,
                                    datad_1, datad_2, argc);
    
    /* Print checksums to prevent dead code elimination */
    printf("Checksums: %ld %d %f %f\n", 
           sum1, sum2, sum3, sum4);
    
    /* Force volatile stores to be observable */
    asm volatile("" : : "m"(v64qi_result), "m"(v32hi_result),
                     "m"(v32hf_result), "m"(v32bf_result),
                     "m"(v16si_result), "m"(v8di_result),
                     "m"(v8df_result), "m"(v16sf_result));
    
    return 0;
}
