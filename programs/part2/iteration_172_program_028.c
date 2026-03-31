/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction expansion coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Check for required ISA support */
#ifndef __AVX512F__
#error "AVX-512F is required for this test"
#endif

/* Function prototypes with target attributes */
#ifdef __cplusplus
extern "C" {
#endif

/* Integer blend functions requiring AVX512BW */
__attribute__((target("avx512f,avx512bw")))
void test_v64qi_blend(void);

__attribute__((target("avx512f,avx512bw")))
void test_v32hi_blend(void);

/* FP16 blend functions */
__attribute__((target("avx512f,avx512bw,avx512fp16")))
void test_v32hf_blend(void);

/* BF16 blend functions */
__attribute__((target("avx512f,avx512bw,avx512bf16")))
void test_v32bf_blend(void);

/* AVX512F integer blends */
__attribute__((target("avx512f")))
void test_v16si_blend(void);

__attribute__((target("avx512f")))
void test_v8di_blend(void);

/* AVX512F floating point blends */
__attribute__((target("avx512f")))
void test_v8df_blend(void);

__attribute__((target("avx512f")))
void test_v16sf_blend(void);

#ifdef __cplusplus
}
#endif

/* Global arrays to prevent optimization */
static uint8_t   v64qi_data_a[64] __attribute__((aligned(64)));
static uint8_t   v64qi_data_b[64] __attribute__((aligned(64)));
static uint8_t   v64qi_result[64] __attribute__((aligned(64)));

static uint16_t  v32hi_data_a[32] __attribute__((aligned(64)));
static uint16_t  v32hi_data_b[32] __attribute__((aligned(64)));
static uint16_t  v32hi_result[32] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16  v32hf_data_a[32] __attribute__((aligned(64)));
static _Float16  v32hf_data_b[32] __attribute__((aligned(64)));
static _Float16  v32hf_result[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16    v32bf_data_a[32] __attribute__((aligned(64)));
static __bf16    v32bf_data_b[32] __attribute__((aligned(64)));
static __bf16    v32bf_result[32] __attribute__((aligned(64)));
#endif

static int32_t   v16si_data_a[16] __attribute__((aligned(64)));
static int32_t   v16si_data_b[16] __attribute__((aligned(64)));
static int32_t   v16si_result[16] __attribute__((aligned(64)));

static int64_t   v8di_data_a[8] __attribute__((aligned(64)));
static int64_t   v8di_data_b[8] __attribute__((aligned(64)));
static int64_t   v8di_result[8] __attribute__((aligned(64)));

static double    v8df_data_a[8] __attribute__((aligned(64)));
static double    v8df_data_b[8] __attribute__((aligned(64)));
static double    v8df_result[8] __attribute__((aligned(64)));

static float     v16sf_data_a[16] __attribute__((aligned(64)));
static float     v16sf_data_b[16] __attribute__((aligned(64)));
static float     v16sf_result[16] __attribute__((aligned(64)));

/* Initialize test data with distinct patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        v64qi_data_a[i] = i;
        v64qi_data_b[i] = 64 - i;
    }
    
    for (int i = 0; i < 32; i++) {
        v32hi_data_a[i] = i * 2;
        v32hi_data_b[i] = 1000 - i * 3;
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        v32hf_data_a[i] = i * 1.5f;
        v32hf_data_b[i] = 50.0f - i * 0.75f;
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        v32bf_data_a[i] = i * 0.5f;
        v32bf_data_b[i] = 25.0f - i * 0.25f;
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        v16si_data_a[i] = i * 100;
        v16si_data_b[i] = 5000 - i * 200;
    }
    
    for (int i = 0; i < 8; i++) {
        v8di_data_a[i] = i * 1000LL;
        v8di_data_b[i] = 10000LL - i * 500LL;
    }
    
    for (int i = 0; i < 8; i++) {
        v8df_data_a[i] = i * 1.25;
        v8df_data_b[i] = 10.0 - i * 0.75;
    }
    
    for (int i = 0; i < 16; i++) {
        v16sf_data_a[i] = i * 0.75f;
        v16sf_data_b[i] = 20.0f - i * 0.5f;
    }
}

/* V64QImode blend test */
__attribute__((target("avx512f,avx512bw")))
void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512(v64qi_data_a);
    __m512i b = _mm512_load_si512(v64qi_data_b);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)), 
                                           _mm512_set1_epi8(0));
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512(v64qi_result, result);
}

/* V32HImode blend test */
__attribute__((target("avx512f,avx512bw")))
void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512(v32hi_data_a);
    __m512i b = _mm512_load_si512(v32hi_data_b);
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a, _mm512_set1_epi16(1)), 
                                            _mm512_set1_epi16(0));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512(v32hi_result, result);
}

/* V32HFmode blend test */
#ifdef __AVX512FP16__
__attribute__((target("avx512f,avx512bw,avx512fp16")))
void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(v32hf_data_a);
    __m512h b = _mm512_load_ph(v32hf_data_b);
    
    /* Create mask by comparing with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(16.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(v32hf_result, result);
}
#endif

/* V32BFmode blend test */
#ifdef __AVX512BF16__
__attribute__((target("avx512f,avx512bw,avx512bf16")))
void test_v32bf_blend(void) {
    __m512bh a = _mm512_load_si512(v32bf_data_a);
    __m512bh b = _mm512_load_si512(v32bf_data_b);
    
    /* Convert to float for comparison, then back to mask */
    __m512 a_f32 = _mm512_cvtpbh_ps(a);
    __m512 b_f32 = _mm512_cvtpbh_ps(b);
    
    __mmask16 mask32 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_LT_OQ);
    
    /* Expand 16-bit mask to 32-bit mask for blend */
    __mmask32 mask = _cvtu32_mask32(_cvtmask16_u32(mask32));
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_si512(v32bf_result, result);
}
#endif

/* V16SImode blend test */
__attribute__((target("avx512f")))
void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512(v16si_data_a);
    __m512i b = _mm512_load_si512(v16si_data_b);
    
    /* Dynamic mask based on sign bit */
    __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_set1_epi32(800));
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512(v16si_result, result);
}

/* V8DImode blend test */
__attribute__((target("avx512f")))
void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512(v8di_data_a);
    __m512i b = _mm512_load_si512(v8di_data_b);
    
    /* Mask based on comparison */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(3000));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512(v8di_result, result);
}

/* V8DFmode blend test */
__attribute__((target("avx512f")))
void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(v8df_data_a);
    __m512d b = _mm512_load_pd(v8df_data_b);
    
    /* Dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(5.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(v8df_result, result);
}

/* V16SFmode blend test */
__attribute__((target("avx512f")))
void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(v16sf_data_a);
    __m512 b = _mm512_load_ps(v16sf_data_b);
    
    /* Create mask by checking if elements are within range */
    __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(6.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(v16sf_result, result);
}

/* Calculate checksum to prevent dead code elimination */
static uint64_t calculate_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += v64qi_result[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += v32hi_result[i];
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)v32hf_result[i];
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
    printf("AVX-512 Blend Instruction Expansion Coverage Test\n");
    
    init_test_data();
    
    /* Execute all blend tests */
    test_v64qi_blend();
    test_v32hi_blend();
    
#ifdef __AVX512FP16__
    test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    test_v32bf_blend();
#endif
    
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
    
    /* Calculate and print checksum to ensure all code executes */
    uint64_t checksum = calculate_checksum();
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    return 0;
}
