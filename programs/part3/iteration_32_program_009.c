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

/* Data-dependent mask generation functions */
__attribute__((noinline))
__mmask64 generate_mask64(int seed) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((seed + i) % 3 == 0) ? (1ULL << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
__mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((seed + i * 2) % 5 == 0) ? (1U << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
__mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((seed + i * 3) % 7 == 0) ? (1 << i) : 0;
    }
    return mask;
}

__attribute__((noinline))
__mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((seed + i * 5) % 11 == 0) ? (1 << i) : 0;
    }
    return mask;
}

/* Blend implementations - each must be in separate function to force RTL expansion */
__attribute__((target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    /* Use runtime mask to prevent constant folding */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Data-dependent computation to prevent elimination */
    __m512i check = _mm512_xor_si512(a, b);
    __mmask64 nonzero = _mm512_cmpneq_epi8_mask(check, _mm512_setzero_si512());
    result = _mm512_mask_add_epi8(result, nonzero, result, _mm512_set1_epi8(1));
    
    return result;
}

__attribute__((target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    /* Create data dependency */
    __m512i diff = _mm512_sub_epi16(a, b);
    __mmask32 pos = _mm512_cmpgt_epi16_mask(diff, _mm512_setzero_si512());
    result = _mm512_mask_sub_epi16(result, pos, result, _mm512_set1_epi16(1));
    
    return result;
}

__attribute__((target("avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Mixed precision operation */
    __m512h half_one = _mm512_set1_ph((_Float16)1.0f);
    __mmask32 gt_mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    result = _mm512_mask_add_ph(result, gt_mask, result, half_one);
    
    return result;
}

__attribute__((target("avx512bw,avx512bf16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Convert to float and back for data dependency */
    __m512 float_vec = _mm512_cvtpbh_ps(result);
    __m512 scaled = _mm512_mul_ps(float_vec, _mm512_set1_ps(1.1f));
    result = _mm512_cvtps_pbh(scaled);
    
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    /* Multi-stage operation */
    __m512i abs_diff = _mm512_abs_epi32(_mm512_sub_epi32(a, b));
    __mmask16 large = _mm512_cmpgt_epi32_mask(abs_diff, _mm512_set1_epi32(100));
    result = _mm512_mask_mullo_epi32(result, large, result, _mm512_set1_epi32(2));
    
    return result;
}

__attribute__((target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    /* Cross-type dependency */
    __m512d float_a = _mm512_cvtepi64_pd(a);
    __m512d float_b = _mm512_cvtepi64_pd(b);
    __m512d float_sum = _mm512_add_pd(float_a, float_b);
    __m512i int_sum = _mm512_cvtpd_epi64(float_sum);
    result = _mm512_add_epi64(result, int_sum);
    
    return result;
}

__attribute__((target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    /* Complex floating point computation */
    __m512d diff = _mm512_sub_pd(a, b);
    __m512d abs_diff = _mm512_max_pd(diff, _mm512_sub_pd(_mm512_setzero_pd(), diff));
    __mmask8 significant = _mm512_cmp_pd_mask(abs_diff, _mm512_set1_pd(0.001), _CMP_GT_OQ);
    result = _mm512_mask_div_pd(result, significant, result, _mm512_set1_pd(2.0));
    
    return result;
}

__attribute__((target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Trigonometric data dependency */
    __m512 sum = _mm512_add_ps(a, b);
    __m512 half_sum = _mm512_mul_ps(sum, _mm512_set1_ps(0.5f));
    __m512 sin_val = _mm512_sin_ps(half_sum);
    result = _mm512_add_ps(result, sin_val);
    
    return result;
}

/* Multi-stage pipeline: V64QI -> V16SI conversion and blending */
__attribute__((target("avx512bw,avx512f")))
int64_t pipeline_v64qi_to_v16si(int seed) {
    /* Stage 1: Blend V64QI */
    char data_a[64], data_b[64];
    for (int i = 0; i < 64; i++) {
        data_a[i] = (char)((seed + i) % 256);
        data_b[i] = (char)((seed + i * 2) % 256);
    }
    
    __m512i vec_a = _mm512_loadu_si512(data_a);
    __m512i vec_b = _mm512_loadu_si512(data_b);
    __mmask64 mask64 = generate_mask64(seed);
    __m512i blended_qi = blend_v64qi(vec_a, vec_b, mask64);
    
    /* Stage 2: Convert to V16SI and blend */
    __m512i extended = _mm512_cvtepi8_epi32(_mm512_castsi512_si128(blended_qi));
    __m512i si_a = _mm512_slli_epi32(extended, 1);
    __m512i si_b = _mm512_srli_epi32(extended, 1);
    __mmask16 mask16 = generate_mask16(seed);
    __m512i blended_si = blend_v16si(si_a, si_b, mask16);
    
    /* Horizontal sum */
    return _mm512_reduce_add_epi32(blended_si);
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    srand(seed);
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
    /* Initialize test data with runtime values */
    int8_t qi_data_a[64], qi_data_b[64];
    int16_t hi_data_a[32], hi_data_b[32];
    int32_t si_data_a[16], si_data_b[16];
    int64_t di_data_a[8], di_data_b[8];
    float sf_data_a[16], sf_data_b[16];
    double df_data_a[8], df_data_b[8];
    
    for (int i = 0; i < 64; i++) {
        qi_data_a[i] = (int8_t)(rand() % 256 - 128);
        qi_data_b[i] = (int8_t)(rand() % 256 - 128);
        if (i < 32) {
            hi_data_a[i] = (int16_t)(rand() % 65536 - 32768);
            hi_data_b[i] = (int16_t)(rand() % 65536 - 32768);
        }
        if (i < 16) {
            si_data_a[i] = rand() - RAND_MAX/2;
            si_data_b[i] = rand() - RAND_MAX/2;
            sf_data_a[i] = (float)rand() / RAND_MAX * 100.0f;
            sf_data_b[i] = (float)rand() / RAND_MAX * 100.0f;
        }
        if (i < 8) {
            di_data_a[i] = ((int64_t)rand() << 32) | rand();
            di_data_b[i] = ((int64_t)rand() << 32) | rand();
            df_data_a[i] = (double)rand() / RAND_MAX * 200.0;
            df_data_b[i] = (double)rand() / RAND_MAX * 200.0;
        }
    }
    
    /* Test each blend type with runtime masks */
    __mmask64 mask64 = generate_mask64(seed);
    __mmask32 mask32 = generate_mask32(seed);
    __mmask16 mask16 = generate_mask16(seed);
    __mmask8 mask8 = generate_mask8(seed);
    
    /* Force all blend operations to execute */
    __m512i v64qi = blend_v64qi(_mm512_loadu_si512(qi_data_a),
                               _mm512_loadu_si512(qi_data_b),
                               mask64);
    v64qi_result = v64qi;
    
    __m512i v32hi = blend_v32hi(_mm512_loadu_si512(hi_data_a),
                               _mm512_loadu_si512(hi_data_b),
                               mask32);
    v32hi_result = v32hi;
    
    /* Half-precision requires conversion */
    _Float16 hf_data_a[32], hf_data_b[32];
    for (int i = 0; i < 32; i++) {
        hf_data_a[i] = (_Float16)(rand() % 100) / 10.0f;
        hf_data_b[i] = (_Float16)(rand() % 100) / 10.0f;
    }
    
    __m512h v32hf = blend_v32hf(_mm512_loadu_ph(hf_data_a),
                               _mm512_loadu_ph(hf_data_b),
                               mask32);
    v32hf_result = v32hf;
    
    /* Brain float */
    __bf16 bf_data_a[32], bf_data_b[32];
    for (int i = 0; i < 32; i++) {
        uint16_t val = rand() % 65536;
        memcpy(&bf_data_a[i], &val, sizeof(__bf16));
        val = rand() % 65536;
        memcpy(&bf_data_b[i], &val, sizeof(__bf16));
    }
    
    __m512bh v32bf = blend_v32bf(_mm512_loadu_bh(bf_data_a),
                                _mm512_loadu_bh(bf_data_b),
                                mask32);
    v32bf_result = v32bf;
    
    __m512i v16si = blend_v16si(_mm512_loadu_si512(si_data_a),
                               _mm512_loadu_si512(si_data_b),
                               mask16);
    v16si_result = v16si;
    
    __m512i v8di = blend_v8di(_mm512_loadu_si512(di_data_a),
                             _mm512_loadu_si512(di_data_b),
                             mask8);
    v8di_result = v8di;
    
    __m512d v8df = blend_v8df(_mm512_loadu_pd(df_data_a),
                             _mm512_loadu_pd(df_data_b),
                             mask8);
    v8df_result = v8df;
    
    __m512 v16sf = blend_v16sf(_mm512_loadu_ps(sf_data_a),
                              _mm512_loadu_ps(sf_data_b),
                              mask16);
    v16sf_result = v16sf;
    
    /* Execute multi-stage pipeline */
    int64_t pipeline_result = pipeline_v64qi_to_v16si(seed);
    
    /* Compute checksum from all results */
    int64_t checksum = 0;
    
    /* Extract elements for checksum - forces materialization */
    int8_t *qi_ptr = (int8_t*)&v64qi;
    for (int i = 0; i < 64; i += 8) checksum += qi_ptr[i];
    
    int16_t *hi_ptr = (int16_t*)&v32hi;
    for (int i = 0; i < 32; i += 4) checksum += hi_ptr[i];
    
    int32_t *si_ptr = (int32_t*)&v16si;
    for (int i = 0; i < 16; i += 2) checksum += si_ptr[i];
    
    int64_t *di_ptr = (int64_t*)&v8di;
    for (int i = 0; i < 8; i++) checksum += di_ptr[i];
    
    checksum += pipeline_result;
    
    printf("Final checksum: %ld\n", checksum);
    printf("All AVX-512 blend intrinsics tested.\n");
    
    return 0;
}
