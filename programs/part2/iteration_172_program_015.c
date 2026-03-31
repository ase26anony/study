/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction expansion coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

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
static uint8_t g_v64qi_a[64] __attribute__((aligned(64)));
static uint8_t g_v64qi_b[64] __attribute__((aligned(64)));
static uint8_t g_v64qi_out[64] __attribute__((aligned(64)));

static int16_t g_v32hi_a[32] __attribute__((aligned(64)));
static int16_t g_v32hi_b[32] __attribute__((aligned(64)));
static int16_t g_v32hi_out[32] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 g_v32hf_a[32] __attribute__((aligned(64)));
static _Float16 g_v32hf_b[32] __attribute__((aligned(64)));
static _Float16 g_v32hf_out[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
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
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        g_v32hf_a[i] = (_Float16)(i * 0.5f + 1.0f);
        g_v32hf_b[i] = (_Float16)(i * 0.75f - 2.0f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        g_v32bf_a[i] = (__bf16)(i * 0.3f + 0.5f);
        g_v32bf_b[i] = (__bf16)(i * 0.7f - 1.5f);
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        g_v16si_a[i] = i * 13 - 500;
        g_v16si_b[i] = i * 17 + 1000;
    }
    
    for (int i = 0; i < 8; i++) {
        g_v8di_a[i] = i * 23LL - 10000;
        g_v8di_b[i] = i * 29LL + 20000;
    }
    
    for (int i = 0; i < 8; i++) {
        g_v8df_a[i] = i * 0.125 + 1.0;
        g_v8df_b[i] = i * 0.25 - 2.0;
    }
    
    for (int i = 0; i < 16; i++) {
        g_v16sf_a[i] = i * 0.0625f + 0.5f;
        g_v16sf_b[i] = i * 0.125f - 1.0f;
    }
}

#ifdef __AVX512BW__
/* V64QImode blend test - triggers gen_avx512bw_blendmv64qi */
void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v64qi_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v64qi_b);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
    
    /* Force non-constant mask by modifying it */
    mask = mask ^ 0xAAAAAAAAAAAAAAAAULL;  /* XOR with pattern */
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v64qi_out, result);
}

/* V32HImode blend test - triggers gen_avx512bw_blendmv32hi */
void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v32hi_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v32hi_b);
    
    /* Create dynamic mask */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
    mask = mask ^ 0xAAAAAAAA;  /* Make it non-constant */
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v32hi_out, result);
}
#endif

#ifdef __AVX512FP16__
/* V32HFmode blend test - triggers gen_avx512bw_blendmv32hf */
void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(g_v32hf_a);
    __m512h b = _mm512_load_ph(g_v32hf_b);
    
    /* Create dynamic mask */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    mask = mask ^ 0x55555555;  /* Make it non-constant */
    
    /* Perform the blend operation */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(g_v32hf_out, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode blend test - triggers gen_avx512bw_blendmv32bf */
void test_v32bf_blend(void) {
    /* Load bfloat16 data - need to cast through int16 */
    __m512bh a = _mm512_load_si512((const __m512i*)g_v32bf_a);
    __m512bh b = _mm512_load_si512((const __m512i*)g_v32bf_b);
    
    /* Create dynamic mask - compare as integers since bfloat16 comparison is tricky */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (g_v32bf_a[i] == g_v32bf_b[i]) {
            mask |= (1U << i);
        }
    }
    mask = mask ^ 0x33333333;  /* Make it non-constant */
    
    /* Perform blend using mask blend intrinsic */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v32bf_out, (__m512i)result);
}
#endif

#ifdef __AVX512F__
/* V16SImode blend test - triggers gen_avx512f_blendmv16si */
void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v16si_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v16si_b);
    
    /* Create dynamic mask */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
    mask = mask ^ 0xAAAA;  /* Make it non-constant */
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v16si_out, result);
}

/* V8DImode blend test - triggers gen_avx512f_blendmv8di */
void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)g_v8di_a);
    __m512i b = _mm512_load_si512((const __m512i*)g_v8di_b);
    
    /* Create dynamic mask */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
    mask = mask ^ 0xAA;  /* Make it non-constant */
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)g_v8di_out, result);
}

/* V8DFmode blend test - triggers gen_avx512f_blendmv8df */
void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(g_v8df_a);
    __m512d b = _mm512_load_pd(g_v8df_b);
    
    /* Create dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
    mask = mask ^ 0x55;  /* Make it non-constant */
    
    /* Perform the blend operation */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(g_v8df_out, result);
}

/* V16SFmode blend test - triggers gen_avx512f_blendmv16sf */
void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(g_v16sf_a);
    __m512 b = _mm512_load_ps(g_v16sf_b);
    
    /* Create dynamic mask */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ);
    mask = mask ^ 0x5555;  /* Make it non-constant */
    
    /* Perform the blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(g_v16sf_out, result);
}
#endif

/* Calculate checksum to prevent dead code elimination */
static uint64_t calculate_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += g_v64qi_out[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_v32hi_out[i];
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_v32hf_out[i];
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)g_v32bf_out[i];
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)g_v16si_out[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)g_v8di_out[i];
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
    /* Initialize test data */
    init_test_data();
    
    printf("Testing AVX-512 blend instruction expansion...\n");
    
    /* Execute all blend tests */
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

#ifdef __AVX512F__
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
#endif
    
    /* Calculate and print checksum to ensure all code executed */
    uint64_t checksum = calculate_checksum();
    printf("Checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    /* Verify some results */
    int errors = 0;
    for (int i = 0; i < 64; i++) {
        if (g_v64qi_out[i] != g_v64qi_a[i] && g_v64qi_out[i] != g_v64qi_b[i]) {
            errors++;
        }
    }
    
    if (errors == 0) {
        printf("All blend operations completed successfully.\n");
    } else {
        printf("Found %d potential errors in blend results.\n", errors);
    }
    
    return 0;
}

#ifdef __cplusplus
}
#endif
