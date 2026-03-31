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
__attribute__((target("avx512f,avx512bw,avx512fp16")))
static void test_v32hf_blend(void);
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512f,avx512bw,avx512bf16")))
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
static uint8_t  v64qi_data_a[64] __attribute__((aligned(64)));
static uint8_t  v64qi_data_b[64] __attribute__((aligned(64)));
static uint8_t  v64qi_output[64] __attribute__((aligned(64)));

static uint16_t v32hi_data_a[32] __attribute__((aligned(64)));
static uint16_t v32hi_data_b[32] __attribute__((aligned(64)));
static uint16_t v32hi_output[32] __attribute__((aligned(64)));

#ifdef __AVX512FP16__
static _Float16 v32hf_data_a[32] __attribute__((aligned(64)));
static _Float16 v32hf_data_b[32] __attribute__((aligned(64)));
static _Float16 v32hf_output[32] __attribute__((aligned(64)));
#endif

#ifdef __AVX512BF16__
static __bf16   v32bf_data_a[32] __attribute__((aligned(64)));
static __bf16   v32bf_data_b[32] __attribute__((aligned(64)));
static __bf16   v32bf_output[32] __attribute__((aligned(64)));
#endif

static int32_t  v16si_data_a[16] __attribute__((aligned(64)));
static int32_t  v16si_data_b[16] __attribute__((aligned(64)));
static int32_t  v16si_output[16] __attribute__((aligned(64)));

static int64_t  v8di_data_a[8] __attribute__((aligned(64)));
static int64_t  v8di_data_b[8] __attribute__((aligned(64)));
static int64_t  v8di_output[8] __attribute__((aligned(64)));

static double   v8df_data_a[8] __attribute__((aligned(64)));
static double   v8df_data_b[8] __attribute__((aligned(64)));
static double   v8df_output[8] __attribute__((aligned(64)));

static float    v16sf_data_a[16] __attribute__((aligned(64)));
static float    v16sf_data_b[16] __attribute__((aligned(64)));
static float    v16sf_output[16] __attribute__((aligned(64)));

/* Initialize test data with non-zero, non-constant patterns */
static void init_test_data(void) {
    for (int i = 0; i < 64; i++) {
        v64qi_data_a[i] = (uint8_t)(i * 3 + 1);
        v64qi_data_b[i] = (uint8_t)(i * 5 + 2);
    }
    
    for (int i = 0; i < 32; i++) {
        v32hi_data_a[i] = (uint16_t)(i * 7 + 3);
        v32hi_data_b[i] = (uint16_t)(i * 11 + 5);
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        v32hf_data_a[i] = (_Float16)(i * 0.5f + 1.0f);
        v32hf_data_b[i] = (_Float16)(i * 0.7f + 2.0f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        v32bf_data_a[i] = (__bf16)(i * 0.3f + 1.5f);
        v32bf_data_b[i] = (__bf16)(i * 0.9f + 2.5f);
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        v16si_data_a[i] = i * 13 + 7;
        v16si_data_b[i] = i * 17 + 11;
    }
    
    for (int i = 0; i < 8; i++) {
        v8di_data_a[i] = i * 19 + 13;
        v8di_data_b[i] = i * 23 + 17;
    }
    
    for (int i = 0; i < 8; i++) {
        v8df_data_a[i] = i * 0.25 + 1.0;
        v8df_data_b[i] = i * 0.35 + 2.0;
    }
    
    for (int i = 0; i < 16; i++) {
        v16sf_data_a[i] = i * 0.15f + 1.5f;
        v16sf_data_b[i] = i * 0.25f + 2.5f;
    }
}

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void) {
    __m512i a = _mm512_load_si512(v64qi_data_a);
    __m512i b = _mm512_load_si512(v64qi_data_b);
    
    /* Create dynamic mask based on data comparison */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(_mm512_and_si512(a, _mm512_set1_epi8(1)), 
                                           _mm512_set1_epi8(1));
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_store_si512(v64qi_output, result);
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void) {
    __m512i a = _mm512_load_si512(v32hi_data_a);
    __m512i b = _mm512_load_si512(v32hi_data_b);
    
    /* Dynamic mask based on LSB of elements */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(_mm512_and_si512(a, _mm512_set1_epi16(1)), 
                                            _mm512_set1_epi16(1));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_store_si512(v32hi_output, result);
}
#endif

#ifdef __AVX512FP16__
/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512f,avx512bw,avx512fp16")))
static void test_v32hf_blend(void) {
    __m512h a = _mm512_load_ph(v32hf_data_a);
    __m512h b = _mm512_load_ph(v32hf_data_b);
    
    /* Create mask by comparing with threshold */
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(16.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_ph(v32hf_output, result);
}
#endif

#ifdef __AVX512BF16__
/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512f,avx512bw,avx512bf16")))
static void test_v32bf_blend(void) {
    __m512bh a = _mm512_load_bf16(v32bf_data_a);
    __m512bh b = _mm512_load_bf16(v32bf_data_b);
    
    /* Use same intrinsic as FP16 but with bfloat16 types */
    __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern for dynamic behavior */
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_store_bf16(v32bf_output, result);
}
#endif

/* V16SImode: 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    __m512i a = _mm512_load_si512(v16si_data_a);
    __m512i b = _mm512_load_si512(v16si_data_b);
    
    /* Dynamic mask based on sign bit */
    __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_set1_epi32(0));
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_store_si512(v16si_output, result);
}

/* V8DImode: 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    __m512i a = _mm512_load_si512(v8di_data_a);
    __m512i b = _mm512_load_si512(v8di_data_b);
    
    /* Dynamic mask based on comparison */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(10));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_store_si512(v8di_output, result);
}

/* V8DFmode: 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    __m512d a = _mm512_load_pd(v8df_data_a);
    __m512d b = _mm512_load_pd(v8df_data_b);
    
    /* Dynamic mask based on value range */
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(2.0), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_store_pd(v8df_output, result);
}

/* V16SFmode: 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    __m512 a = _mm512_load_ps(v16sf_data_a);
    __m512 b = _mm512_load_ps(v16sf_data_b);
    
    /* Dynamic mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_store_ps(v16sf_output, result);
}
#endif

/* Compute checksum to prevent dead code elimination */
static uint64_t compute_checksum(void) {
    uint64_t checksum = 0;
    
    for (int i = 0; i < 64; i++) {
        checksum += v64qi_output[i];
    }
    
    for (int i = 0; i < 32; i++) {
        checksum += v32hi_output[i];
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)v32hf_output[i];
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        checksum += (uint16_t)v32bf_output[i];
    }
#endif
    
    for (int i = 0; i < 16; i++) {
        checksum += v16si_output[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += v8di_output[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)v8df_output[i];
    }
    
    for (int i = 0; i < 16; i++) {
        checksum += (uint32_t)v16sf_output[i];
    }
    
    return checksum;
}

int main(void) {
    printf("Testing AVX-512 blend instruction expansion coverage...\n");
    
    init_test_data();
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Testing V64QImode and V32HImode blends...\n");
    test_v64qi_blend();
    test_v32hi_blend();
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode blend...\n");
    test_v32hf_blend();
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode blend...\n");
    test_v32bf_blend();
#endif

    printf("Testing V16SImode, V8DImode, V8DFmode, V16SFmode blends...\n");
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
    
    uint64_t checksum = compute_checksum();
    printf("Checksum: %lu\n", checksum);
    printf("All blend operations completed.\n");
    
    return 0;
#else
    printf("AVX-512 not supported on this platform.\n");
    return 1;
#endif
}

#ifdef __cplusplus
}
#endif
