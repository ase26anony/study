/* avx512_blend_coverage_test.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage_test.c -o avx512_blend_coverage_test
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
static uint16_t g_f16_data[32] __attribute__((aligned(64)));
static uint16_t g_bf16_data[32] __attribute__((aligned(64)));
static int32_t  g_s32_data[16] __attribute__((aligned(64)));
static int64_t  g_s64_data[8]  __attribute__((aligned(64)));
static double   g_f64_data[8]  __attribute__((aligned(64)));
static float    g_f32_data[16] __attribute__((aligned(64)));

static uint8_t  g_u8_out[64]   __attribute__((aligned(64)));
static uint16_t g_u16_out[32]  __attribute__((aligned(64)));
static uint16_t g_f16_out[32]  __attribute__((aligned(64)));
static uint16_t g_bf16_out[32] __attribute__((aligned(64)));
static int32_t  g_s32_out[16]  __attribute__((aligned(64)));
static int64_t  g_s64_out[8]   __attribute__((aligned(64)));
static double   g_f64_out[8]   __attribute__((aligned(64)));
static float    g_f32_out[16]  __attribute__((aligned(64)));

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* V64QImode: 64 x 8-bit integers */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void)
{
    /* Create two different vectors */
    __m512i a = _mm512_loadu_si512((const __m512i*)g_u8_data);
    __m512i b = _mm512_set1_epi8(0xAA);
    
    /* Generate dynamic mask based on data values (prevents constant folding) */
    __mmask64 mask = _mm512_cmpeq_epi8_mask(a, _mm512_setzero_si512());
    
    /* Blend based on dynamic mask - this should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)g_u8_out, result);
}

/* V32HImode: 32 x 16-bit integers */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void)
{
    __m512i a = _mm512_loadu_si512((const __m512i*)g_u16_data);
    __m512i b = _mm512_set1_epi16(0x5555);
    
    /* Dynamic mask: blend where elements are even */
    __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)g_u16_out, result);
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode: 32 x half-precision floats */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void)
{
    __m512h a = _mm512_loadu_ph(g_f16_data);
    __m512h b = _mm512_set1_ph(2.0f);
    
    /* Create dynamic mask by comparing with zero */
    __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_setzero_ph(), _CMP_EQ_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_storeu_ph(g_f16_out, result);
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode: 32 x bfloat16 */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void)
{
    /* Use __m512bh for bfloat16 */
    __m512bh a = _mm512_loadu_bf16(g_bf16_data);
    __m512bh b = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
    
    /* Load as integers to create mask */
    __m512i a_int = _mm512_loadu_si512((const __m512i*)g_bf16_data);
    __mmask32 mask = _mm512_cmpeq_epi16_mask(a_int, _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_storeu_bf16(g_bf16_out, result);
}
#endif /* __AVX512BF16__ */

/* V16SImode: 16 x 32-bit integers */
__attribute__((target("avx512f")))
static void test_v16si_blend(void)
{
    __m512i a = _mm512_loadu_si512((const __m512i*)g_s32_data);
    __m512i b = _mm512_set1_epi32(0xDEADBEEF);
    
    /* Dynamic mask based on sign bit */
    __mmask16 mask = _mm512_cmplt_epi32_mask(a, _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)g_s32_out, result);
}

/* V8DImode: 8 x 64-bit integers */
__attribute__((target("avx512f")))
static void test_v8di_blend(void)
{
    __m512i a = _mm512_loadu_si512((const __m512i*)g_s64_data);
    __m512i b = _mm512_set1_epi64(0xCAFEBABEDEADBEEF);
    
    /* Dynamic mask: blend where low bit is set */
    __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)g_s64_out, result);
}

/* V8DFmode: 8 x double-precision floats */
__attribute__((target("avx512f")))
static void test_v8df_blend(void)
{
    __m512d a = _mm512_loadu_pd(g_f64_data);
    __m512d b = _mm512_set1_pd(3.141592653589793);
    
    /* Dynamic mask: blend where value > 0 */
    __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_setzero_pd(), _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_storeu_pd(g_f64_out, result);
}

/* V16SFmode: 16 x single-precision floats */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void)
{
    __m512 a = _mm512_loadu_ps(g_f32_data);
    __m512 b = _mm512_set1_ps(2.718281828459045f);
    
    /* Dynamic mask: blend where value < 0 */
    __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_setzero_ps(), _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_storeu_ps(g_f32_out, result);
}
#endif /* __AVX512F__ */

/* Initialize test data with non-zero, non-constant values */
static void init_test_data(void)
{
    for (int i = 0; i < 64; i++) {
        g_u8_data[i] = (uint8_t)(i * 3 + 1);
        g_u8_out[i] = 0;
    }
    
    for (int i = 0; i < 32; i++) {
        g_u16_data[i] = (uint16_t)(i * 5 + 2);
        g_f16_data[i] = (uint16_t)((i % 10) * 0x3555); /* Half-float pattern */
        g_bf16_data[i] = (uint16_t)((i % 8) * 0x3F80); /* Bfloat16 pattern */
        g_u16_out[i] = 0;
        g_f16_out[i] = 0;
        g_bf16_out[i] = 0;
    }
    
    for (int i = 0; i < 16; i++) {
        g_s32_data[i] = (int32_t)(i * 7 - 8);
        g_f32_data[i] = (float)(i * 0.5f - 4.0f);
        g_s32_out[i] = 0;
        g_f32_out[i] = 0.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        g_s64_data[i] = (int64_t)(i * 11 - 44);
        g_f64_data[i] = (double)(i * 0.25 - 2.0);
        g_s64_out[i] = 0;
        g_f64_out[i] = 0.0;
    }
}

/* Compute checksum to ensure all blends executed */
static uint64_t compute_checksum(void)
{
    uint64_t sum = 0;
    
    for (int i = 0; i < 64; i++) sum += g_u8_out[i];
    for (int i = 0; i < 32; i++) sum += g_u16_out[i];
    for (int i = 0; i < 32; i++) sum += g_f16_out[i];
    for (int i = 0; i < 32; i++) sum += g_bf16_out[i];
    for (int i = 0; i < 16; i++) sum += (uint32_t)g_s32_out[i];
    for (int i = 0; i < 8; i++) sum += (uint64_t)g_s64_out[i];
    
    /* Use integer representation of floats for checksum */
    for (int i = 0; i < 16; i++) {
        uint32_t val;
        memcpy(&val, &g_f32_out[i], sizeof(val));
        sum += val;
    }
    
    for (int i = 0; i < 8; i++) {
        uint64_t val;
        memcpy(&val, &g_f64_out[i], sizeof(val));
        sum += val;
    }
    
    return sum;
}

int main(void)
{
    init_test_data();
    
#ifdef __AVX512F__
    printf("AVX-512 support detected\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode blend...\n");
    test_v64qi_blend();
    
    printf("Testing V32HImode blend...\n");
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
    
    printf("Testing V16SImode blend...\n");
    test_v16si_blend();
    
    printf("Testing V8DImode blend...\n");
    test_v8di_blend();
    
    printf("Testing V8DFmode blend...\n");
    test_v8df_blend();
    
    printf("Testing V16SFmode blend...\n");
    test_v16sf_blend();
#else
    printf("AVX-512 not supported on this compiler/platform\n");
    return 1;
#endif
    
    uint64_t checksum = compute_checksum();
    printf("Final checksum: 0x%016llx\n", (unsigned long long)checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
