/* AVX-512 blend instruction coverage test for GCC i386-expand.cc */
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

/* Global arrays to prevent constant propagation */
static uint8_t g_v64qi_a[64] __attribute__((aligned(64)));
static uint8_t g_v64qi_b[64] __attribute__((aligned(64)));
static uint8_t g_v64qi_out[64] __attribute__((aligned(64)));

static int16_t g_v32hi_a[32] __attribute__((aligned(64)));
static int16_t g_v32hi_b[32] __attribute__((aligned(64)));
static int16_t g_v32hi_out[32] __attribute__((aligned(64)));

#if HAS_AVX512FP16
static _Float16 g_v32hf_a[32] __attribute__((aligned(64)));
static _Float16 g_v32hf_b[32] __attribute__((aligned(64)));
static _Float16 g_v32hf_out[32] __attribute__((aligned(64)));
#endif

#if HAS_AVX512BF16
static __bf16 g_v32bf_a[32] __attribute__((aligned(64)));
static __bf16 g_v32bf_b[32] __attribute__((aligned(64)));
static __bf16 g_v32bf_out[32] __attribute__((aligned(64)));
#endif

static int32_t g_v16si_a[16] __attribute__((aligned(64)));
static int32_t g_v16si_b[16] __attribute__((aligned(64)));
static int32_t g_v16si_out[16] __attribute__((aligned(64)));

static int64_t g_v8di_a[8] __attribute__((aligned(64)));
static int64_t g_v8di_b[8] __attribute__((aligned(64)));
static int64_t g_v8di_out[8] __attribute__((aligned(64)));

static double g_v8df_a[8] __attribute__((aligned(64)));
static double g_v8df_b[8] __attribute__((aligned(64)));
static double g_v8df_out[8] __attribute__((aligned(64)));

static float g_v16sf_a[16] __attribute__((aligned(64)));
static float g_v16sf_b[16] __attribute__((aligned(64)));
static float g_v16sf_out[16] __attribute__((aligned(64)));

/* Initialize test data with non-zero, non-constant patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        g_v64qi_a[i] = (uint8_t)(i * 3 + 1);
        g_v64qi_b[i] = (uint8_t)(i * 5 + 2);
    }
    
    for (int i = 0; i < 32; i++) {
        g_v32hi_a[i] = (int16_t)(i * 7 - 100);
        g_v32hi_b[i] = (int16_t)(i * 11 + 200);
    }
    
#if HAS_AVX512FP16
    for (int i = 0; i < 32; i++) {
        g_v32hf_a[i] = (_Float16)(i * 0.5f + 1.0f);
        g_v32hf_b[i] = (_Float16)(i * 0.7f - 2.0f);
    }
#endif
    
#if HAS_AVX512BF16
    for (int i = 0; i < 32; i++) {
        g_v32bf_a[i] = (__bf16)(i * 0.3f + 0.5f);
        g_v32bf_b[i] = (__bf16)(i * 0.9f - 1.5f);
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        g_v16si_a[i] = i * 13 - 500;
        g_v16si_b[i] = i * 17 + 1000;
    }
    
    for (int i = 0; i < 8; i++) {
        g_v8di_a[i] = i * 19LL - 10000;
        g_v8di_b[i] = i * 23LL + 20000;
    }
    
    for (int i = 0; i < 8; i++) {
        g_v8df_a[i] = i * 1.1 + 0.5;
        g_v8df_b[i] = i * 1.3 - 1.5;
    }
    
    for (int i = 0; i < 16; i++) {
        g_v16sf_a[i] = i * 0.7f + 0.2f;
        g_v16sf_b[i] = i * 0.9f - 0.4f;
    }
}

#if HAS_AVX512BW
/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v64qi_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v64qi_b);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)),
                                           _mm512_set1_epi8(1));
    
    /* Blend based on dynamic mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v64qi_out, result);
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v32hi_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v32hi_b);
    
    /* Dynamic mask based on sign bit */
    __mmask32 mask = _mm512_cmplt_epi16_mask(a, _mm512_set1_epi16(0));
    
    /* Blend based on dynamic mask - should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v32hi_out, result);
}
#endif

#if HAS_AVX512FP16
/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512fp16,avx512bw")))
static void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph((const __m512h*)g_v32hf_a);
    __m512h b = _mm512_load_ph((const __m512h*)g_v32hf_b);
    
    /* Dynamic mask based on comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    
    /* Blend based on dynamic mask - should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph((__m512h*)g_v32hf_out, result);
}
#endif

#if HAS_AVX512BF16
/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512bf16,avx512bw")))
static void test_v32bf_blend(void) {
    __m512bh a = _mm512_load_si512((const __m512i*)g_v32bf_a);
    __m512bh b = _mm512_load_si512((const __m512i*)g_v32bf_b);
    
    /* Create a simple alternating mask pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    /* Blend based on mask - should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v32bf_out, result);
}
#endif

#if HAS_AVX512F
/* V16SImode: 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v16si_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v16si_b);
    
    /* Dynamic mask based on parity */
    __mmask16 mask = _mm512_test_epi32_mask(a, _mm512_set1_epi32(1));
    
    /* Blend based on dynamic mask - should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v16si_out, result);
}

/* V8DImode: 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v8di_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v8di_b);
    
    /* Dynamic mask based on sign */
    __mmask8 mask = _mm512_movepi64_mask(_mm512_srai_epi64(a, 63));
    
    /* Blend based on dynamic mask - should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v8di_out, result);
}

/* V8DFmode: 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(g_v8df_a);
    __m512d b = _mm512_load_pd(g_v8df_b);
    
    /* Dynamic mask based on comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(0.0), _CMP_GT_OQ);
    
    /* Blend based on dynamic mask - should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(g_v8df_out, result);
}

/* V16SFmode: 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(g_v16sf_a);
    __m512 b = _mm512_load_ps(g_v16sf_b);
    
    /* Dynamic mask based on comparison with zero */
    __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(0.0f), _CMP_LT_OQ);
    
    /* Blend based on dynamic mask - should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(g_v16sf_out, result);
}
#endif

/* Compute checksum to prevent dead code elimination */
static uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += g_v64qi_out[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += g_v32hi_out[i];
    }
    
#if HAS_AVX512FP16
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_v32hf_out[i];
    }
#endif
    
#if HAS_AVX512BF16
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_v32bf_out[i];
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        checksum += g_v16si_out[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += g_v8di_out[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)g_v8df_out[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)g_v16sf_out[i];
    }
    
    return checksum;
}

int main(void) {
    printf("AVX-512 Blend Instruction Coverage Test\n");
    
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
    
    /* Compute and print checksum to ensure all code executed */
    uint64_t checksum = compute_checksum();
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Feature availability report */
    printf("\nFeature availability:\n");
    printf("AVX512F: %s\n", HAS_AVX512F ? "YES" : "NO");
    printf("AVX512BW: %s\n", HAS_AVX512BW ? "YES" : "NO");
    printf("AVX512FP16: %s\n", HAS_AVX512FP16 ? "YES" : "NO");
    printf("AVX512BF16: %s\n", HAS_AVX512BF16 ? "YES" : "NO");
    
    return 0;
}
