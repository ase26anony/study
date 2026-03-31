/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage.c -o avx512_blend_coverage
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes with target attributes */
#ifdef __AVX512F__
#ifdef __AVX512BW__
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void);
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void);
#endif

#ifdef __AVX512FP16__
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void);
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void);
#endif

__attribute__((target("avx512f")))
static void test_v16si_blend(void);
__attribute__((target("avx512f")))
static void test_v8di_blend(void);
__attribute__((target("avx512f")))
static void test_v8df_blend(void);
__attribute__((target("avx512f")))
static void test_v16sf_blend(void);
#endif

/* Global arrays to prevent constant propagation */
static uint8_t  g_u8_data[64]  __attribute__((aligned(64)));
static uint16_t g_u16_data[32] __attribute__((aligned(64)));
static int32_t  g_i32_data[16] __attribute__((aligned(64)));
static int64_t  g_i64_data[8]  __attribute__((aligned(64)));
static float    g_f32_data[16] __attribute__((aligned(64)));
static double   g_f64_data[8]  __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 g_f16_data[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16   g_bf16_data[32] __attribute__((aligned(64)));
#endif

/* Initialize test data with non-zero, non-constant patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        g_u8_data[i] = (uint8_t)((i * 37 + 123) & 0xFF);
    }
    
    for (int i = 0; i < 32; i++) {
        g_u16_data[i] = (uint16_t)((i * 73 + 456) & 0xFFFF);
    }
    
    for (int i = 0; i < 16; i++) {
        g_i32_data[i] = (int32_t)(i * 97 - 789);
        g_f32_data[i] = (float)(i * 1.5f - 3.14159f);
    }
    
    for (int i = 0; i < 8; i++) {
        g_i64_data[i] = (int64_t)(i * 131 - 1000);
        g_f64_data[i] = (double)(i * 2.71828 - 1.41421);
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        g_f16_data[i] = (_Float16)((i * 0.5f - 2.0f));
    }
#endif

#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        /* Simple conversion from float to bfloat16 */
        float f = (float)(i * 0.25f - 1.0f);
        uint32_t u32;
        memcpy(&u32, &f, sizeof(float));
        g_bf16_data[i] = (__bf16)(u32 >> 16);
    }
#endif
}

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* Test V64QImode blend - covers gen_avx512bw_blendmv64qi */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_u8_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u8_data + 32));
    
    /* Create dynamic mask based on data values */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)),
                                           _mm512_setzero_si512());
    
    /* Blend based on dynamic mask */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_u8_data, result);
}

/* Test V32HImode blend - covers gen_avx512bw_blendmv32hi */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_u16_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u16_data + 16));
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a, _mm512_set1_epi16(1)),
                                            _mm512_setzero_si512());
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_u16_data, result);
}
#endif

#ifdef __AVX512FP16__
/* Test V32HFmode blend - covers gen_avx512bw_blendmv32hf */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(g_f16_data);
    __m512h b = _mm512_load_ph(g_f16_data + 16);
    
    /* Create mask by comparing with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(0.0f), _CMP_GT_OQ);
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(g_f16_data, result);
}
#endif

#ifdef __AVX512BF16__
/* Test V32BFmode blend - covers gen_avx512bw_blendmv32bf */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void) {
    __m512bh a = _mm512_load_bf16(g_bf16_data);
    __m512bh b = _mm512_load_bf16(g_bf16_data + 16);
    
    /* Use a simple alternating pattern mask */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_bf16(g_bf16_data, result);
}
#endif

/* Test V16SImode blend - covers gen_avx512f_blendmv16si */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_i32_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_i32_data + 8));
    
    /* Dynamic mask based on sign bit */
    __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_setzero_si512());
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_i32_data, result);
}

/* Test V8DImode blend - covers gen_avx512f_blendmv8di */
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_i64_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_i64_data + 4));
    
    /* Mask based on LSB */
    __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_i64_data, result);
}

/* Test V8DFmode blend - covers gen_avx512f_blendmv8df */
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(g_f64_data);
    __m512d b = _mm512_load_pd(g_f64_data + 4);
    
    /* Compare with zero for dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_setzero_pd(), _CMP_GT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(g_f64_data, result);
}

/* Test V16SFmode blend - covers gen_avx512f_blendmv16sf */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(g_f32_data);
    __m512 b = _mm512_load_ps(g_f32_data + 8);
    
    /* Dynamic mask based on absolute value */
    __mmask16 mask = _mm512_cmp_ps_mask(_mm512_abs_ps(a), 
                                       _mm512_set1_ps(1.0f), 
                                       _CMP_GT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(g_f32_data, result);
}
#endif

/* Compute checksum to prevent dead code elimination */
static uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += g_u8_data[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += g_u16_data[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)g_i32_data[i];
        checksum += *(uint32_t*)&g_f32_data[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)g_i64_data[i];
        checksum += *(uint64_t*)&g_f64_data[i];
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        checksum += *(uint16_t*)&g_f16_data[i];
    }
#endif

#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        checksum += *(uint16_t*)&g_bf16_data[i];
    }
#endif
    
    return checksum;
}

int main(void) {
    init_test_data();
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    test_v64qi_blend();
    test_v32hi_blend();
#endif

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
#else
    printf("AVX-512 not supported at compile time\n");
    return 1;
#endif
    
    uint64_t checksum = compute_checksum();
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
