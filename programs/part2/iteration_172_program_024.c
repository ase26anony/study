/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC i386-expand.cc
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
static uint8_t  g_u8_data[64] __attribute__((aligned(64)));
static uint16_t g_u16_data[32] __attribute__((aligned(64)));
static uint32_t g_u32_data[16] __attribute__((aligned(64)));
static uint64_t g_u64_data[8] __attribute__((aligned(64)));
static float    g_f32_data[16] __attribute__((aligned(64)));
static double   g_f64_data[8] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 g_f16_data[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16   g_bf16_data[32] __attribute__((aligned(64)));
#endif

/* Initialize test data with distinct patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        g_u8_data[i] = (uint8_t)(i * 3 + 1);
    }
    for (int i = 0; i < 32; i++) {
        g_u16_data[i] = (uint16_t)(i * 5 + 2);
    }
    for (int i = 0; i < 16; i++) {
        g_u32_data[i] = (uint32_t)(i * 7 + 3);
        g_f32_data[i] = (float)(i * 0.5f + 1.0f);
    }
    for (int i = 0; i < 8; i++) {
        g_u64_data[i] = (uint64_t)(i * 11ULL + 4);
        g_f64_data[i] = (double)(i * 0.25 + 2.0);
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        g_f16_data[i] = (_Float16)(i * 0.125f + 0.5f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        /* Simple pattern for bfloat16 */
        uint16_t val = (uint16_t)((i + 1) << 7);
        memcpy(&g_bf16_data[i], &val, sizeof(__bf16));
    }
#endif
}

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_u8_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u8_data + 32));
    
    /* Create dynamic mask based on data values (prevents constant folding) */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)), 
                                           _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_u8_data, result);
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_u16_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u16_data + 16));
    
    /* Dynamic mask based on parity of values */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a, _mm512_set1_epi16(1)), 
                                            _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_u16_data, result);
}
#endif

/* V16SImode: 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_u32_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u32_data + 8));
    
    /* Dynamic mask using comparison */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_si512(a, _mm512_set1_epi32(1)), 
                                            _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_u32_data, result);
}

/* V8DImode: 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_u64_data);
    __m512i b = _mm512_load_si512((const __m512i*)(g_u64_data + 4));
    
    /* Dynamic mask */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_si512(a, _mm512_set1_epi64(1)), 
                                           _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_u64_data, result);
}

/* V8DFmode: 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(g_f64_data);
    __m512d b = _mm512_load_pd(g_f64_data + 4);
    
    /* Dynamic mask using comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(g_f64_data, result);
}

/* V16SFmode: 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(g_f32_data);
    __m512 b = _mm512_load_ps(g_f32_data + 8);
    
    /* Dynamic mask */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(g_f32_data, result);
}

#ifdef __AVX512FP16__
/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(g_f16_data);
    __m512h b = _mm512_load_ph(g_f16_data + 16);
    
    /* Dynamic mask - compare for equality with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(0.5f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(g_f16_data, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void) {
    /* Load bfloat16 data */
    __m512bh a = _mm512_load_si512((const __m512i*)g_bf16_data);
    __m512bh b = _mm512_load_si512((const __m512i*)(g_bf16_data + 16));
    
    /* Create a simple dynamic mask */
    __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
    
    /* Blend using the mask - this should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_bf16_data, (__m512i)result);
}
#endif
#endif /* __AVX512F__ */

/* Main test driver */
int main(void) {
    /* Initialize test data */
    init_test_data();
    
    /* Run all blend tests */
#ifdef __AVX512F__
#ifdef __AVX512BW__
    test_v64qi_blend();
    test_v32hi_blend();
#endif
    
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
    
#ifdef __AVX512FP16__
    test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    test_v32bf_blend();
#endif
    
    /* Compute checksum to ensure all blends executed */
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += g_u8_data[i];
    }
    for (int i = 0; i < 32; i++) {
        checksum += g_u16_data[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += g_u32_data[i];
        checksum += (uint64_t)g_f32_data[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += g_u64_data[i];
        checksum += (uint64_t)g_f64_data[i];
    }
    
    printf("Blend test checksum: %lu\n", checksum);
    return 0;
#else
    printf("AVX-512 not supported on this compiler/platform\n");
    return 1;
#endif
}

#ifdef __cplusplus
}
#endif
