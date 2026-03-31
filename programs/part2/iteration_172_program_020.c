/* AVX-512 Blend Instruction Coverage Test
 * Targets specific switch cases in i386-expand.cc for blend instruction expansion
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
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

/* Global arrays to prevent optimization */
static uint8_t v64qi_a[64] __attribute__((aligned(64)));
static uint8_t v64qi_b[64] __attribute__((aligned(64)));
static uint8_t v64qi_result[64] __attribute__((aligned(64)));

static int16_t v32hi_a[32] __attribute__((aligned(64)));
static int16_t v32hi_b[32] __attribute__((aligned(64)));
static int16_t v32hi_result[32] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 v32hf_a[32] __attribute__((aligned(64)));
static _Float16 v32hf_b[32] __attribute__((aligned(64)));
static _Float16 v32hf_result[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16 v32bf_a[32] __attribute__((aligned(64)));
static __bf16 v32bf_b[32] __attribute__((aligned(64)));
static __bf16 v32bf_result[32] __attribute__((aligned(64)));
#endif

static int32_t v16si_a[16] __attribute__((aligned(64)));
static int32_t v16si_b[16] __attribute__((aligned(64)));
static int32_t v16si_result[16] __attribute__((aligned(64)));

static int64_t v8di_a[8] __attribute__((aligned(64)));
static int64_t v8di_b[8] __attribute__((aligned(64)));
static int64_t v8di_result[8] __attribute__((aligned(64)));

static double v8df_a[8] __attribute__((aligned(64)));
static double v8df_b[8] __attribute__((aligned(64)));
static double v8df_result[8] __attribute__((aligned(64)));

static float v16sf_a[16] __attribute__((aligned(64)));
static float v16sf_b[16] __attribute__((aligned(64)));
static float v16sf_result[16] __attribute__((aligned(64)));

/* Initialize test data with non-zero, non-constant patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        v64qi_a[i] = (uint8_t)(i * 3 + 1);
        v64qi_b[i] = (uint8_t)(i * 5 + 2);
    }
    
    for (int i = 0; i < 32; i++) {
        v32hi_a[i] = (int16_t)(i * 7 - 100);
        v32hi_b[i] = (int16_t)(i * 11 + 200);
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        v32hf_a[i] = (_Float16)(i * 1.5f);
        v32hf_b[i] = (_Float16)(i * 2.5f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        v32bf_a[i] = (__bf16)(i * 0.75f);
        v32bf_b[i] = (__bf16)(i * 1.25f);
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        v16si_a[i] = i * 13 - 500;
        v16si_b[i] = i * 17 + 1000;
    }
    
    for (int i = 0; i < 8; i++) {
        v8di_a[i] = i * 19LL - 10000;
        v8di_b[i] = i * 23LL + 20000;
    }
    
    for (int i = 0; i < 8; i++) {
        v8df_a[i] = i * 1.111;
        v8df_b[i] = i * 2.222;
    }
    
    for (int i = 0; i < 16; i++) {
        v16sf_a[i] = i * 0.333f;
        v16sf_b[i] = i * 0.666f;
    }
}

#ifdef __AVX512BW__
/* V64QImode: 64-byte integer blend */
void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)v64qi_a);
    __m512i b = _mm512_load_si512((const __m512i*)v64qi_b);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
    
    /* Force non-constant mask by modifying it */
    mask = mask ^ 0xAAAAAAAAAAAAAAAAULL;  /* XOR with pattern to ensure non-constant */
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512((__m512i*)v64qi_result, result);
}

/* V32HImode: 32 half-word integer blend */
void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)v32hi_a);
    __m512i b = _mm512_load_si512((const __m512i*)v32hi_b);
    
    /* Create dynamic mask */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
    mask = mask ^ 0x55555555;  /* Make mask non-constant */
    
    /* Perform blend */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512((__m512i*)v32hi_result, result);
}
#endif

#ifdef __AVX512FP16__
/* V32HFmode: 32 half-precision float blend */
void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(v32hf_a);
    __m512h b = _mm512_load_ph(v32hf_b);
    
    /* Create mask by comparing with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
    
    /* Make mask non-constant */
    static int counter = 0;
    mask = mask ^ (counter++ & 0x1);
    
    /* Perform blend */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(v32hf_result, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode: 32 bfloat16 blend */
void test_v32bf_blend(void) {
    __m512bh a = _mm512_load_bf16(v32bf_a);
    __m512bh b = _mm512_load_bf16(v32bf_b);
    
    /* Create mask - use integer comparison on reinterpreted data */
    __m512i a_int = _mm512_load_si512((const __m512i*)v32bf_a);
    __m512i b_int = _mm512_load_si512((const __m512i*)v32bf_b);
    __mmask32 mask = _mm512_cmpeq_epi16_mask(a_int, b_int);
    
    /* Make mask non-constant */
    mask = mask ^ 0xAAAAAAAA;
    
    /* Perform blend - use _mm512_mask_blend_epi16 since there's no direct bfloat16 blend */
    __m512i result_int = _mm512_mask_blend_epi16(mask, a_int, b_int);
    _mm512_store_si512((__m512i*)v32bf_result, result_int);
}
#endif

#ifdef __AVX512F__
/* V16SImode: 16 single-word integer blend */
void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)v16si_a);
    __m512i b = _mm512_load_si512((const __m512i*)v16si_b);
    
    /* Create dynamic mask */
    __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
    mask = mask ^ 0xAAAA;  /* Make non-constant */
    
    /* Perform blend */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512((__m512i*)v16si_result, result);
}

/* V8DImode: 8 double-word integer blend */
void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512((const __m512i*)v8di_a);
    __m512i b = _mm512_load_si512((const __m512i*)v8di_b);
    
    /* Create dynamic mask */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
    mask = mask ^ 0xAA;  /* Make non-constant */
    
    /* Perform blend */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512((__m512i*)v8di_result, result);
}

/* V8DFmode: 8 double-precision float blend */
void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(v8df_a);
    __m512d b = _mm512_load_pd(v8df_b);
    
    /* Create dynamic mask */
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    mask = mask ^ 0x55;  /* Make non-constant */
    
    /* Perform blend */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(v8df_result, result);
}

/* V16SFmode: 16 single-precision float blend */
void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(v16sf_a);
    __m512 b = _mm512_load_ps(v16sf_b);
    
    /* Create dynamic mask */
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    mask = mask ^ 0x5555;  /* Make non-constant */
    
    /* Perform blend */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(v16sf_result, result);
}
#endif

/* Calculate checksum to prevent dead code elimination */
static uint64_t calculate_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += v64qi_result[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)v32hi_result[i];
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
        checksum += (uint32_t)v16si_result[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)v8di_result[i];
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
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
