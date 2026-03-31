/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction expansion coverage */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Define fallback macros for missing intrinsics in older compilers */
#ifndef __AVX512FP16__
#define __AVX512FP16__ 0
#endif

#ifndef __AVX512BF16__
#define __AVX512BF16__ 0
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
static uint8_t g_u8_data1[64] __attribute__((aligned(64)));
static uint8_t g_u8_data2[64] __attribute__((aligned(64)));
static uint8_t g_u8_result[64] __attribute__((aligned(64)));

static uint16_t g_u16_data1[32] __attribute__((aligned(64)));
static uint16_t g_u16_data2[32] __attribute__((aligned(64)));
static uint16_t g_u16_result[32] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 g_f16_data1[32] __attribute__((aligned(64)));
static _Float16 g_f16_data2[32] __attribute__((aligned(64)));
static _Float16 g_f16_result[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16 g_bf16_data1[32] __attribute__((aligned(64)));
static __bf16 g_bf16_data2[32] __attribute__((aligned(64)));
static __bf16 g_bf16_result[32] __attribute__((aligned(64)));
#endif

static int32_t g_i32_data1[16] __attribute__((aligned(64)));
static int32_t g_i32_data2[16] __attribute__((aligned(64)));
static int32_t g_i32_result[16] __attribute__((aligned(64)));

static int64_t g_i64_data1[8] __attribute__((aligned(64)));
static int64_t g_i64_data2[8] __attribute__((aligned(64)));
static int64_t g_i64_result[8] __attribute__((aligned(64)));

static double g_f64_data1[8] __attribute__((aligned(64)));
static double g_f64_data2[8] __attribute__((aligned(64)));
static double g_f64_result[8] __attribute__((aligned(64)));

static float g_f32_data1[16] __attribute__((aligned(64)));
static float g_f32_data2[16] __attribute__((aligned(64)));
static float g_f32_result[16] __attribute__((aligned(64)));

/* Initialize test data with non-constant patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        g_u8_data1[i] = (uint8_t)(i * 3 + 1);
        g_u8_data2[i] = (uint8_t)(i * 5 + 2);
    }
    
    for (int i = 0; i < 32; i++) {
        g_u16_data1[i] = (uint16_t)(i * 7 + 3);
        g_u16_data2[i] = (uint16_t)(i * 11 + 5);
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        g_f16_data1[i] = (_Float16)(i * 0.1f + 1.0f);
        g_f16_data2[i] = (_Float16)(i * 0.2f + 2.0f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        /* Simple pattern for bfloat16 */
        g_bf16_data1[i] = (__bf16)(i + 1);
        g_bf16_data2[i] = (__bf16)(i + 100);
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        g_i32_data1[i] = i * 13 + 7;
        g_i32_data2[i] = i * 17 + 11;
    }
    
    for (int i = 0; i < 8; i++) {
        g_i64_data1[i] = i * 19 + 13;
        g_i64_data2[i] = i * 23 + 17;
    }
    
    for (int i = 0; i < 8; i++) {
        g_f64_data1[i] = i * 0.3 + 1.5;
        g_f64_data2[i] = i * 0.7 + 2.5;
    }
    
    for (int i = 0; i < 16; i++) {
        g_f32_data1[i] = i * 0.1f + 1.0f;
        g_f32_data2[i] = i * 0.3f + 2.0f;
    }
}

#ifdef __AVX512BW__
/* V64QImode blend test */
void test_v64qi_blend(void) {
    __m512i v1 = _mm512_load_si512((const __m512i*)g_u8_data1);
    __m512i v2 = _mm512_load_si512((const __m512i*)g_u8_data2);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(v1, _mm512_set1_epi8(1)), 
                                           _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)g_u8_result, result);
}

/* V32HImode blend test */
void test_v32hi_blend(void) {
    __m512i v1 = _mm512_load_si512((const __m512i*)g_u16_data1);
    __m512i v2 = _mm512_load_si512((const __m512i*)g_u16_data2);
    
    /* Dynamic mask: select elements where LSB is 0 */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(v1, _mm512_set1_epi16(1)), 
                                            _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)g_u16_result, result);
}
#endif

#ifdef __AVX512FP16__
/* V32HFmode blend test */
void test_v32hf_blend(void) {
    __m512h v1 = _mm512_load_ph(g_f16_data1);
    __m512h v2 = _mm512_load_ph(g_f16_data2);
    
    /* Create mask by comparing with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, _mm512_set1_ph(8.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(g_f16_result, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode blend test */
void test_v32bf_blend(void) {
    __m512bh v1 = _mm512_load_bf16(g_bf16_data1);
    __m512bh v2 = _mm512_load_bf16(g_bf16_data2);
    
    /* Create mask using integer comparison on reinterpreted data */
    __m512i vi1 = _mm512_castsi512_si512((__m512i)v1);
    __m512i vi2 = _mm512_castsi512_si512((__m512i)v2);
    
    __mmask32 mask = _mm512_cmplt_epi16_mask(vi1, vi2);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_bf16(g_bf16_result, result);
}
#endif

#ifdef __AVX512F__
/* V16SImode blend test */
void test_v16si_blend(void) {
    __m512i v1 = _mm512_load_si512((const __m512i*)g_i32_data1);
    __m512i v2 = _mm512_load_si512((const __m512i*)g_i32_data2);
    
    /* Dynamic mask based on sign bit */
    __mmask16 mask = _mm512_cmplt_epi32_mask(v1, _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)g_i32_result, result);
}

/* V8DImode blend test */
void test_v8di_blend(void) {
    __m512i v1 = _mm512_load_si512((const __m512i*)g_i64_data1);
    __m512i v2 = _mm512_load_si512((const __m512i*)g_i64_data2);
    
    /* Dynamic mask: select where value < 50 */
    __mmask8 mask = _mm512_cmplt_epi64_mask(v1, _mm512_set1_epi64(50));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)g_i64_result, result);
}

/* V8DFmode blend test */
void test_v8df_blend(void) {
    __m512d v1 = _mm512_load_pd(g_f64_data1);
    __m512d v2 = _mm512_load_pd(g_f64_data2);
    
    /* Dynamic mask: select where value > 2.0 */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(2.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(g_f64_result, result);
}

/* V16SFmode blend test */
void test_v16sf_blend(void) {
    __m512 v1 = _mm512_load_ps(g_f32_data1);
    __m512 v2 = _mm512_load_ps(g_f32_data2);
    
    /* Dynamic mask: select where value < 3.0f */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(3.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(g_f32_result, result);
}
#endif

/* Compute checksum to prevent dead code elimination */
static uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += g_u8_result[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += g_u16_result[i];
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        /* Convert half to integer for checksum */
        checksum += (uint16_t)g_f16_result[i];
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_bf16_result[i];
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        checksum += g_i32_result[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += g_i64_result[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)g_f64_result[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)g_f32_result[i];
    }
    
    return checksum;
}

int main(void) {
    init_test_data();
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
#ifdef __AVX512BW__
    test_v64qi_blend();
    printf("  V64QImode blend tested\n");
    
    test_v32hi_blend();
    printf("  V32HImode blend tested\n");
#endif
    
#ifdef __AVX512FP16__
    test_v32hf_blend();
    printf("  V32HFmode blend tested\n");
#endif
    
#ifdef __AVX512BF16__
    test_v32bf_blend();
    printf("  V32BFmode blend tested\n");
#endif
    
#ifdef __AVX512F__
    test_v16si_blend();
    printf("  V16SImode blend tested\n");
    
    test_v8di_blend();
    printf("  V8DImode blend tested\n");
    
    test_v8df_blend();
    printf("  V8DFmode blend tested\n");
    
    test_v16sf_blend();
    printf("  V16SFmode blend tested\n");
#endif
    
    uint64_t checksum = compute_checksum();
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
