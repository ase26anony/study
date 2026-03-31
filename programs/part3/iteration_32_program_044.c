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
__m512i compute_v64qi_mask(__m512i a, __m512i b, int seed) {
    __m512i cmp = _mm512_cmpgt_epi8_mask(a, b) ? 
                  _mm512_set1_epi8(seed & 0xFF) : 
                  _mm512_set1_epi8((~seed) & 0xFF);
    
    /* Create runtime-dependent mask */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((seed + i) & 1) << i;
    }
    
    /* Force blend operation */
    return _mm512_mask_blend_epi8(mask, a, cmp);
}

__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Multi-stage pipeline: blend then convert */
    __m512i blended = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store to volatile to prevent elimination */
    v64qi_result = blended;
    
    /* Convert to wider type for next stage */
    __m512i extended = _mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(blended, 0));
    return extended;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    /* Create data-dependent mask from comparison */
    __mmask32 cmp_mask = _mm512_cmpgt_epi16_mask(a, b);
    
    /* Blend with runtime mask */
    __m512i blended = _mm512_mask_blend_epi16(mask | cmp_mask, a, b);
    
    v32hi_result = blended;
    
    /* Prepare for integer to float conversion */
    return _mm512_srai_epi16(blended, 2);
}

#ifdef __AVX512FP16__
__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    /* Mixed precision: convert from integer */
    __m512i int_vec = _mm512_loadu_si512((const __m512i*)&a);
    __m512h half_from_int = _mm512_cvtepi16_ph(int_vec);
    
    /* Blend with half precision */
    __m512h blended = _mm512_mask_blend_ph(mask, a, half_from_int);
    
    v32hf_result = blended;
    
    /* Convert to single precision for next stage */
    __m512 single_prec = _mm512_cvtph_ps(_mm512_extractf32x8_ps(blended, 0));
    return _mm512_castps_ph(single_prec);
}
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    /* Convert from float to bfloat16 */
    __m512 float_vec = _mm512_set1_ps(1.0f);
    __m512bh bf_from_float = _mm512_cvtneps_pbh(float_vec);
    
    /* Blend bfloat16 vectors */
    __m512bh blended = _mm512_mask_blend_ph(mask, a, bf_from_float);
    
    v32bf_result = blended;
    return blended;
}
#endif

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    /* Create complex mask from arithmetic */
    __m512i diff = _mm512_sub_epi32(a, b);
    __mmask16 sign_mask = _mm512_cmplt_epi32_mask(diff, _mm512_setzero_si512());
    
    /* Blend with combined masks */
    __m512i blended = _mm512_mask_blend_epi32(mask ^ sign_mask, a, b);
    
    v16si_result = blended;
    
    /* Convert to double for next stage */
    __m512d double_vec = _mm512_cvtepi32_pd(_mm512_extracti32x8_epi32(blended, 0));
    return _mm512_castpd_si512(double_vec);
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    /* Generate mask from bitwise operations */
    __m512i xor_result = _mm512_xor_si512(a, b);
    __mmask8 nonzero_mask = _mm512_cmpneq_epi64_mask(xor_result, _mm512_setzero_si512());
    
    /* Blend 64-bit integers */
    __m512i blended = _mm512_mask_blend_epi64(mask & nonzero_mask, a, b);
    
    v8di_result = blended;
    return blended;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    /* Conditional blending based on comparison */
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* Blend double precision */
    __m512d blended = _mm512_mask_blend_pd(mask | cmp_mask, a, b);
    
    v8df_result = blended;
    
    /* Convert to single precision */
    __m512 single_prec = _mm512_cvtpd_ps(blended);
    return _mm512_castps_pd(single_prec);
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    /* Complex mask generation */
    __m512 abs_a = _mm512_abs_ps(a);
    __m512 abs_b = _mm512_abs_ps(b);
    __mmask16 abs_mask = _mm512_cmp_ps_mask(abs_a, abs_b, _CMP_GT_OQ);
    
    /* Blend single precision */
    __m512 blended = _mm512_mask_blend_ps(mask ^ abs_mask, a, b);
    
    v16sf_result = blended;
    return blended;
}

/* Main test harness */
int main(int argc, char** argv) {
    /* Use argc for runtime variability */
    unsigned int seed = (unsigned int)argc;
    srand(seed);
    
    /* Initialize test data with runtime values */
    char char_data[64];
    short short_data[32];
    int int_data[16];
    long long long_data[8];
    float float_data[16];
    double double_data[8];
    
    for (int i = 0; i < 64; i++) char_data[i] = (char)(rand() % 256 - 128);
    for (int i = 0; i < 32; i++) short_data[i] = (short)(rand() % 65536 - 32768);
    for (int i = 0; i < 16; i++) int_data[i] = rand() - RAND_MAX/2;
    for (int i = 0; i < 8; i++) long_data[i] = ((long long)rand() << 32) | rand();
    for (int i = 0; i < 16; i++) float_data[i] = (float)rand() / RAND_MAX * 100.0f - 50.0f;
    for (int i = 0; i < 8; i++) double_data[i] = (double)rand() / RAND_MAX * 200.0 - 100.0;
    
    /* Load into vectors */
    __m512i v64qi_a = _mm512_loadu_si512((const __m512i*)char_data);
    __m512i v64qi_b = _mm512_loadu_si512((const __m512i*)(char_data + 32));
    
    __m512i v32hi_a = _mm512_loadu_si512((const __m512i*)short_data);
    __m512i v32hi_b = _mm512_loadu_si512((const __m512i*)(short_data + 16));
    
    __m512i v16si_a = _mm512_loadu_si512((const __m512i*)int_data);
    __m512i v16si_b = _mm512_loadu_si512((const __m512i*)(int_data + 8));
    
    __m512i v8di_a = _mm512_loadu_si512((const __m512i*)long_data);
    __m512i v8di_b = _mm512_loadu_si512((const __m512i*)(long_data + 4));
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_loadu_ps(float_data + 8);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_loadu_pd(double_data + 4);
    
    /* Generate runtime masks */
    __mmask64 mask64 = 0;
    __mmask32 mask32 = 0;
    __mmask16 mask16 = 0;
    __mmask8 mask8 = 0;
    
    for (int i = 0; i < 64; i++) mask64 |= ((rand() + i) & 1ULL) << i;
    for (int i = 0; i < 32; i++) mask32 |= ((rand() + i) & 1ULL) << i;
    for (int i = 0; i < 16; i++) mask16 |= ((rand() + i) & 1ULL) << i;
    for (int i = 0; i < 8; i++) mask8 |= ((rand() + i) & 1ULL) << i;
    
    /* Execute blend pipeline */
    __m512i result;
    
    /* V64QI blend */
    result = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    /* V32HI blend */
    __m512i v32hi_input = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    result = blend_v32hi(v32hi_input, v32hi_b, mask32);
    
    /* V16SI blend */
    __m512i v16si_input = _mm512_slli_epi16(result, 2);
    result = blend_v16si(v16si_input, v16si_b, mask16);
    
    /* V8DI blend */
    result = blend_v8di(v8di_a, v8di_b, mask8);
    
    /* V8DF blend */
    __m512d v8df_input = _mm512_castsi512_pd(result);
    __m512d df_result = blend_v8df(v8df_input, v8df_b, mask8);
    
    /* V16SF blend */
    __m512 v16sf_input = _mm512_castpd_ps(df_result);
    __m512 sf_result = blend_v16sf(v16sf_input, v16sf_b, mask16);
    
#ifdef __AVX512FP16__
    /* V32HF blend if supported */
    __m512h v32hf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); /* 1.0 in half */
    __m512h v32hf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); /* 2.0 in half */
    blend_v32hf(v32hf_a, v32hf_b, mask32);
#endif
    
#ifdef __AVX512BF16__
    /* V32BF blend if supported */
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_set1_epi16(0x3F80)); /* ~1.0 in bfloat16 */
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_set1_epi16(0x4000)); /* ~2.0 in bfloat16 */
    blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
    
    /* Compute checksum from all results */
    uint64_t checksum = 0;
    
    /* Horizontal add for integer results */
    checksum += _mm512_reduce_add_epi64(v64qi_result);
    checksum += _mm512_reduce_add_epi64(v32hi_result);
    checksum += _mm512_reduce_add_epi64(v16si_result);
    checksum += _mm512_reduce_add_epi64(v8di_result);
    
    /* Convert float results to integer for checksum */
    checksum += (uint64_t)_mm512_reduce_add_pd(v8df_result);
    checksum += (uint64_t)_mm512_reduce_add_ps(v16sf_result);
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
