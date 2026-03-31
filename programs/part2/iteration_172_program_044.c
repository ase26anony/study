/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction coverage */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

#ifdef __AVX512FP16__
#define HAS_AVX512FP16 1
#else
#define HAS_AVX512FP16 0
#endif

#ifdef __AVX512BF16__
#define HAS_AVX512BF16 1
#else
#define HAS_AVX512BF16 0
#endif

/* Function prototypes with target attributes */
#if HAS_AVX512BW
__attribute__((target("avx512bw")))
static void test_v64qi_blend(void);
__attribute__((target("avx512bw")))
static void test_v32hi_blend(void);
#endif

#if HAS_AVX512FP16
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hf_blend(void);
#endif

#if HAS_AVX512BF16
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bf_blend(void);
#endif

#if HAS_AVX512F
__attribute__((target("avx512f")))
static void test_v16si_blend(void);
__attribute__((target("avx512f")))
static void test_v8di_blend(void);
__attribute__((target("avx512f")))
static void test_v8df_blend(void);
__attribute__((target("avx512f")))
static void test_v16sf_blend(void);
#endif

/* Global arrays to prevent optimization */
static uint8_t g_u8_data1[64] __attribute__((aligned(64)));
static uint8_t g_u8_data2[64] __attribute__((aligned(64)));
static uint8_t g_u8_result[64] __attribute__((aligned(64)));

static int16_t g_i16_data1[32] __attribute__((aligned(64)));
static int16_t g_i16_data2[32] __attribute__((aligned(64)));
static int16_t g_i16_result[32] __attribute__((aligned(64)));

static int32_t g_i32_data1[16] __attribute__((aligned(64)));
static int32_t g_i32_data2[16] __attribute__((aligned(64)));
static int32_t g_i32_result[16] __attribute__((aligned(64)));

static int64_t g_i64_data1[8] __attribute__((aligned(64)));
static int64_t g_i64_data2[8] __attribute__((aligned(64)));
static int64_t g_i64_result[8] __attribute__((aligned(64)));

static float g_f32_data1[16] __attribute__((aligned(64)));
static float g_f32_data2[16] __attribute__((aligned(64)));
static float g_f32_result[16] __attribute__((aligned(64)));

static double g_f64_data1[8] __attribute__((aligned(64)));
static double g_f64_data2[8] __attribute__((aligned(64)));
static double g_f64_result[8] __attribute__((aligned(64)));

#if HAS_AVX512FP16
static _Float16 g_f16_data1[32] __attribute__((aligned(64)));
static _Float16 g_f16_data2[32] __attribute__((aligned(64)));
static _Float16 g_f16_result[32] __attribute__((aligned(64)));
#endif

#if HAS_AVX512BF16
static __bf16 g_bf16_data1[32] __attribute__((aligned(64)));
static __bf16 g_bf16_data2[32] __attribute__((aligned(64)));
static __bf16 g_bf16_result[32] __attribute__((aligned(64)));
#endif

/* Initialize test data with distinct patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        g_u8_data1[i] = (uint8_t)(i * 3 + 1);
        g_u8_data2[i] = (uint8_t)(i * 5 + 2);
    }
    
    for (int i = 0; i < 32; i++) {
        g_i16_data1[i] = (int16_t)(i * 7 - 100);
        g_i16_data2[i] = (int16_t)(i * 11 + 200);
    }
    
    for (int i = 0; i < 16; i++) {
        g_i32_data1[i] = i * 13 - 500;
        g_i32_data2[i] = i * 17 + 1000;
    }
    
    for (int i = 0; i < 8; i++) {
        g_i64_data1[i] = i * 19LL - 10000;
        g_i64_data2[i] = i * 23LL + 20000;
    }
    
    for (int i = 0; i < 16; i++) {
        g_f32_data1[i] = i * 1.5f - 10.0f;
        g_f32_data2[i] = i * 2.5f + 20.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        g_f64_data1[i] = i * 1.25 - 50.0;
        g_f64_data2[i] = i * 3.75 + 100.0;
    }
    
#if HAS_AVX512FP16
    for (int i = 0; i < 32; i++) {
        g_f16_data1[i] = (_Float16)(i * 0.5f - 5.0f);
        g_f16_data2[i] = (_Float16)(i * 1.5f + 10.0f);
    }
#endif
    
#if HAS_AVX512BF16
    for (int i = 0; i < 32; i++) {
        g_bf16_data1[i] = (__bf16)(i * 0.75f - 3.0f);
        g_bf16_data2[i] = (__bf16)(i * 2.25f + 15.0f);
    }
#endif
}

/* Test functions for each vector mode */
#if HAS_AVX512BW
__attribute__((target("avx512bw")))
static void test_v64qi_blend(void) {
    /* V64QImode: __m512i blend with 64-byte elements */
    __m512i vec1 = _mm512_load_si512((const __m512i*)g_u8_data1);
    __m512i vec2 = _mm512_load_si512((const __m512i*)g_u8_data2);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(vec1, _mm512_set1_epi8(1)), 
                                           _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, vec1, vec2);
    
    _mm512_store_si512((__m512i*)g_u8_result, result);
}
#endif

#if HAS_AVX512BW
__attribute__((target("avx512bw")))
static void test_v32hi_blend(void) {
    /* V32HImode: __m512i blend with 32 half-word elements */
    __m512i vec1 = _mm512_load_si512((const __m512i*)g_i16_data1);
    __m512i vec2 = _mm512_load_si512((const __m512i*)g_i16_data2);
    
    /* Dynamic mask based on sign bit */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(_mm512_setzero_si512(), vec1);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, vec1, vec2);
    
    _mm512_store_si512((__m512i*)g_i16_result, result);
}
#endif

#if HAS_AVX512FP16
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hf_blend(void) {
    /* V32HFmode: __m512h blend with 32 half-precision float elements */
    __m512h vec1 = _mm512_load_ph(g_f16_data1);
    __m512h vec2 = _mm512_load_ph(g_f16_data2);
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(vec1, _mm512_setzero_ph(), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, vec1, vec2);
    
    _mm512_store_ph(g_f16_result, result);
}
#endif

#if HAS_AVX512BF16
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bf_blend(void) {
    /* V32BFmode: bfloat16 blend with 32 elements */
    __m512bh vec1 = _mm512_load_bf16(g_bf16_data1);
    __m512bh vec2 = _mm512_load_bf16(g_bf16_data2);
    
    /* Create a simple alternating mask pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, vec1, vec2);
    
    _mm512_store_bf16(g_bf16_result, result);
}
#endif

#if HAS_AVX512F
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    /* V16SImode: __m512i blend with 32-bit integer elements */
    __m512i vec1 = _mm512_load_si512((const __m512i*)g_i32_data1);
    __m512i vec2 = _mm512_load_si512((const __m512i*)g_i32_data2);
    
    /* Dynamic mask using equality test */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_epi32(vec1, _mm512_set1_epi32(1)), 
                                            _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, vec1, vec2);
    
    _mm512_store_si512((__m512i*)g_i32_result, result);
}
#endif

#if HAS_AVX512F
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    /* V8DImode: __m512i blend with 64-bit integer elements */
    __m512i vec1 = _mm512_load_si512((const __m512i*)g_i64_data1);
    __m512i vec2 = _mm512_load_si512((const __m512i*)g_i64_data2);
    
    /* Dynamic mask using greater-than comparison */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(vec1, _mm512_set1_epi64(0));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, vec1, vec2);
    
    _mm512_store_si512((__m512i*)g_i64_result, result);
}
#endif

#if HAS_AVX512F
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    /* V8DFmode: __m512d blend with double-precision float elements */
    __m512d vec1 = _mm512_load_pd(g_f64_data1);
    __m512d vec2 = _mm512_load_pd(g_f64_data2);
    
    /* Dynamic mask using less-than comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(vec1, _mm512_setzero_pd(), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, vec1, vec2);
    
    _mm512_store_pd(g_f64_result, result);
}
#endif

#if HAS_AVX512F
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    /* V16SFmode: __m512 blend with single-precision float elements */
    __m512 vec1 = _mm512_load_ps(g_f32_data1);
    __m512 vec2 = _mm512_load_ps(g_f32_data2);
    
    /* Dynamic mask using not-equal comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(vec1, _mm512_setzero_ps(), _CMP_NEQ_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, vec1, vec2);
    
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
        checksum += (uint16_t)g_i16_result[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)g_i32_result[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)g_i64_result[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)(g_f32_result[i] * 1000);
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)(g_f64_result[i] * 1000);
    }
    
#if HAS_AVX512FP16
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)(g_f16_result[i] * 1000);
    }
#endif
    
#if HAS_AVX512BF16
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)(g_bf16_result[i] * 1000);
    }
#endif
    
    return checksum;
}

int main(void) {
    printf("AVX-512 Blend Coverage Test\n");
    
    /* Initialize test data */
    init_test_data();
    
    /* Execute all blend tests */
#if HAS_AVX512BW
    test_v64qi_blend();
    test_v32hi_blend();
#endif

#if HAS_AVX512FP16
    test_v32hf_blend();
#endif

#if HAS_AVX512BF16
    test_v32bf_blend();
#endif

#if HAS_AVX512F
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
#endif
    
    /* Compute and print checksum to ensure all code executes */
    uint64_t checksum = compute_checksum();
    printf("Checksum: %lu\n", (unsigned long)checksum);
    
    /* Print which features were available */
    printf("Features available:\n");
    printf("  AVX512F: %s\n", HAS_AVX512F ? "YES" : "NO");
    printf("  AVX512BW: %s\n", HAS_AVX512BW ? "YES" : "NO");
    printf("  AVX512FP16: %s\n", HAS_AVX512FP16 ? "YES" : "NO");
    printf("  AVX512BF16: %s\n", HAS_AVX512BF16 ? "YES" : "NO");
    
    return 0;
}
