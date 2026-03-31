/* AVX-512 Blend Instruction Coverage Test
 * Targets specific uncovered lines in i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes with target attributes */
#ifdef __AVX512BW__
void test_v64qi_blend(void) __attribute__((target("avx512bw")));
void test_v32hi_blend(void) __attribute__((target("avx512bw")));
#endif

#ifdef __AVX512FP16__
void test_v32hf_blend(void) __attribute__((target("avx512fp16")));
#endif

#ifdef __AVX512BF16__
void test_v32bf_blend(void) __attribute__((target("avx512bf16")));
#endif

#ifdef __AVX512F__
void test_v16si_blend(void) __attribute__((target("avx512f")));
void test_v8di_blend(void) __attribute__((target("avx512f")));
void test_v8df_blend(void) __attribute__((target("avx512f")));
void test_v16sf_blend(void) __attribute__((target("avx512f")));
#endif

/* Global arrays to prevent constant propagation */
static uint8_t v64qi_data_a[64] __attribute__((aligned(64)));
static uint8_t v64qi_data_b[64] __attribute__((aligned(64)));
static uint8_t v64qi_result[64] __attribute__((aligned(64)));

static int16_t v32hi_data_a[32] __attribute__((aligned(64)));
static int16_t v32hi_data_b[32] __attribute__((aligned(64)));
static int16_t v32hi_result[32] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 v32hf_data_a[32] __attribute__((aligned(64)));
static _Float16 v32hf_data_b[32] __attribute__((aligned(64)));
static _Float16 v32hf_result[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16 v32bf_data_a[32] __attribute__((aligned(64)));
static __bf16 v32bf_data_b[32] __attribute__((aligned(64)));
static __bf16 v32bf_result[32] __attribute__((aligned(64)));
#endif

static int32_t v16si_data_a[16] __attribute__((aligned(64)));
static int32_t v16si_data_b[16] __attribute__((aligned(64)));
static int32_t v16si_result[16] __attribute__((aligned(64)));

static int64_t v8di_data_a[8] __attribute__((aligned(64)));
static int64_t v8di_data_b[8] __attribute__((aligned(64)));
static int64_t v8di_result[8] __attribute__((aligned(64)));

static double v8df_data_a[8] __attribute__((aligned(64)));
static double v8df_data_b[8] __attribute__((aligned(64)));
static double v8df_result[8] __attribute__((aligned(64)));

static float v16sf_data_a[16] __attribute__((aligned(64)));
static float v16sf_data_b[16] __attribute__((aligned(64)));
static float v16sf_result[16] __attribute__((aligned(64)));

/* Initialize data with pattern to create non-trivial blend masks */
static void init_data(void) {
    for (int i = 0; i < 64; i++) {
        v64qi_data_a[i] = (uint8_t)(i * 3);
        v64qi_data_b[i] = (uint8_t)(i * 5 + 1);
    }
    
    for (int i = 0; i < 32; i++) {
        v32hi_data_a[i] = (int16_t)(i * 7);
        v32hi_data_b[i] = (int16_t)(i * 11 + 3);
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        v32hf_data_a[i] = (_Float16)(i * 0.5f);
        v32hf_data_b[i] = (_Float16)(i * 0.7f + 0.1f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        v32bf_data_a[i] = (__bf16)(i * 0.3f);
        v32bf_data_b[i] = (__bf16)(i * 0.9f + 0.2f);
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        v16si_data_a[i] = i * 13;
        v16si_data_b[i] = i * 17 + 5;
    }
    
    for (int i = 0; i < 8; i++) {
        v8di_data_a[i] = i * 23LL;
        v8di_data_b[i] = i * 29LL + 7;
    }
    
    for (int i = 0; i < 8; i++) {
        v8df_data_a[i] = i * 1.5;
        v8df_data_b[i] = i * 2.5 + 0.5;
    }
    
    for (int i = 0; i < 16; i++) {
        v16sf_data_a[i] = i * 0.75f;
        v16sf_data_b[i] = i * 1.25f + 0.25f;
    }
}

#ifdef __AVX512BW__
/* V64QImode blend - triggers gen_avx512bw_blendmv64qi */
void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512(v64qi_data_a);
    __m512i b = _mm512_load_si512(v64qi_data_b);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)), 
                                           _mm512_setzero_si512());
    
    /* Perform blend - this should expand to gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512(v64qi_result, result);
}

/* V32HImode blend - triggers gen_avx512bw_blendmv32hi */
void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512(v32hi_data_a);
    __m512i b = _mm512_load_si512(v32hi_data_b);
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a, _mm512_set1_epi16(1)), 
                                            _mm512_setzero_si512());
    
    /* This should expand to gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512(v32hi_result, result);
}
#endif

#ifdef __AVX512FP16__
/* V32HFmode blend - triggers gen_avx512bw_blendmv32hf */
void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(v32hf_data_a);
    __m512h b = _mm512_load_ph(v32hf_data_b);
    
    /* Create mask by comparing with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(8.0f), _CMP_LT_OQ);
    
    /* This should expand to gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(v32hf_result, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode blend - triggers gen_avx512bw_blendmv32bf */
void test_v32bf_blend(void) {
    __m512bh a = _mm512_load_bf16(v32bf_data_a);
    __m512bh b = _mm512_load_bf16(v32bf_data_b);
    
    /* Use mask blend intrinsic for bfloat16 */
    __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
    
    /* This should expand to gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_bf16(v32bf_result, result);
}
#endif

#ifdef __AVX512F__
/* V16SImode blend - triggers gen_avx512f_blendmv16si */
void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512(v16si_data_a);
    __m512i b = _mm512_load_si512(v16si_data_b);
    
    /* Dynamic mask using comparison */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_epi32(a, _mm512_set1_epi32(1)), 
                                            _mm512_setzero_si512());
    
    /* This should expand to gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512(v16si_result, result);
}

/* V8DImode blend - triggers gen_avx512f_blendmv8di */
void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512(v8di_data_a);
    __m512i b = _mm512_load_si512(v8di_data_b);
    
    /* Dynamic mask */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_epi64(a, _mm512_set1_epi64(1)), 
                                           _mm512_setzero_si512());
    
    /* This should expand to gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512(v8di_result, result);
}

/* V8DFmode blend - triggers gen_avx512f_blendmv8df */
void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(v8df_data_a);
    __m512d b = _mm512_load_pd(v8df_data_b);
    
    /* Create mask by comparing elements */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* This should expand to gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(v8df_result, result);
}

/* V16SFmode blend - triggers gen_avx512f_blendmv16sf */
void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(v16sf_data_a);
    __m512 b = _mm512_load_ps(v16sf_data_b);
    
    /* Dynamic mask based on comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(6.0f), _CMP_LT_OQ);
    
    /* This should expand to gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(v16sf_result, result);
}
#endif

/* Compute checksum to ensure all blends are executed */
static uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += v64qi_result[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += v32hi_result[i];
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)v32hf_result[i]; /* Treat as bits */
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)v32bf_result[i];
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        checksum += v16si_result[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += v8di_result[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)v8df_result[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)v16sf_result[i];
    }
    
    return checksum;
}

int main(void) {
    /* Initialize test data */
    init_data();
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
    /* Execute all blend tests */
#ifdef __AVX512BW__
    test_v64qi_blend();
    printf("V64QImode blend executed\n");
    
    test_v32hi_blend();
    printf("V32HImode blend executed\n");
#endif

#ifdef __AVX512FP16__
    test_v32hf_blend();
    printf("V32HFmode blend executed\n");
#endif

#ifdef __AVX512BF16__
    test_v32bf_blend();
    printf("V32BFmode blend executed\n");
#endif

#ifdef __AVX512F__
    test_v16si_blend();
    printf("V16SImode blend executed\n");
    
    test_v8di_blend();
    printf("V8DImode blend executed\n");
    
    test_v8df_blend();
    printf("V8DFmode blend executed\n");
    
    test_v16sf_blend();
    printf("V16SFmode blend executed\n");
#endif
    
    /* Compute and print checksum to prevent dead code elimination */
    uint64_t checksum = compute_checksum();
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
