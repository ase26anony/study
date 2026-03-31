/* AVX-512 blend instruction coverage test
 * Targets specific uncovered lines in i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test
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

/* Checksum variables to prevent dead code elimination */
static volatile uint64_t g_checksum = 0;

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* V64QImode: 64-byte integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void)
{
    /* Initialize source data */
    __m512i src1 = _mm512_set1_epi8(0x11);
    __m512i src2 = _mm512_set1_epi8(0x22);
    
    /* Create dynamic mask based on array data */
    __m512i mask_data = _mm512_load_si512((const __m512i*)g_u8_data);
    __mmask64 mask = _mm512_cmpeq_epi8_mask(mask_data, _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, src1, src2);
    
    /* Store and compute checksum to prevent optimization */
    _mm512_store_si512((__m512i*)g_u8_data, result);
    
    /* Compute simple checksum */
    __m512i sum = _mm512_sad_epu8(result, _mm512_setzero_si512());
    g_checksum += _mm512_reduce_add_epi64(sum);
}

/* V32HImode: 32 half-word integer blend */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void)
{
    /* Initialize source data */
    __m512i src1 = _mm512_set1_epi16(0x1111);
    __m512i src2 = _mm512_set1_epi16(0x2222);
    
    /* Create dynamic mask */
    __m512i mask_data = _mm512_load_si512((const __m512i*)g_u16_data);
    __mmask32 mask = _mm512_cmpeq_epi16_mask(mask_data, _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, src1, src2);
    
    _mm512_store_si512((__m512i*)g_u16_data, result);
    
    /* Compute checksum */
    __m512i sum_lo = _mm512_unpacklo_epi16(result, _mm512_setzero_si512());
    __m512i sum_hi = _mm512_unpackhi_epi16(result, _mm512_setzero_si512());
    __m512i sum = _mm512_add_epi32(sum_lo, sum_hi);
    g_checksum += _mm512_reduce_add_epi32(sum);
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode: 32 half-precision float blend */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void)
{
    /* Initialize source data */
    __m512h src1 = _mm512_set1_ph(1.0f);
    __m512h src2 = _mm512_set1_ph(2.0f);
    
    /* Create dynamic mask using comparison */
    __m512h mask_data = _mm512_load_ph(g_f16_data);
    __mmask32 mask = _mm512_cmp_ph_mask(mask_data, _mm512_setzero_ph(), _CMP_EQ_OQ);
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, src1, src2);
    
    _mm512_store_ph(g_f16_data, result);
    
    /* Compute checksum */
    __m256h lo = _mm512_castph512_ph256(result);
    __m256h hi = _mm512_extractf32x8_ps(_mm512_castph_ps(result), 1);
    
    for (int i = 0; i < 16; i++) {
        g_checksum += (uint64_t)((float)lo[i] * 1000);
    }
    for (int i = 0; i < 16; i++) {
        g_checksum += (uint64_t)((float)hi[i] * 1000);
    }
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode: 32 bfloat16 blend */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void)
{
    /* Initialize source data */
    __m512bh src1 = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
    __m512bh src2 = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
    
    /* Load mask data as bfloat16 */
    __m512bh mask_data = _mm512_load_si512(g_bf16_data);
    
    /* Create mask by checking if elements are zero */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (((uint16_t*)g_bf16_data)[i] == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, src1, src2);
    
    _mm512_store_si512(g_bf16_data, result);
    
    /* Compute checksum */
    for (int i = 0; i < 32; i++) {
        g_checksum += ((uint16_t*)g_bf16_data)[i];
    }
}
#endif /* __AVX512BF16__ */

/* V16SImode: 16 single-word integer blend */
__attribute__((target("avx512f")))
static void test_v16si_blend(void)
{
    /* Initialize source data */
    __m512i src1 = _mm512_set1_epi32(0x11111111);
    __m512i src2 = _mm512_set1_epi32(0x22222222);
    
    /* Create dynamic mask */
    __m512i mask_data = _mm512_load_si512((const __m512i*)g_i32_data);
    __mmask16 mask = _mm512_cmpeq_epi32_mask(mask_data, _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, src1, src2);
    
    _mm512_store_si512((__m512i*)g_i32_data, result);
    
    /* Compute checksum */
    g_checksum += _mm512_reduce_add_epi32(result);
}

/* V8DImode: 8 double-word integer blend */
__attribute__((target("avx512f")))
static void test_v8di_blend(void)
{
    /* Initialize source data */
    __m512i src1 = _mm512_set1_epi64(0x1111111111111111ULL);
    __m512i src2 = _mm512_set1_epi64(0x2222222222222222ULL);
    
    /* Create dynamic mask */
    __m512i mask_data = _mm512_load_si512((const __m512i*)g_i64_data);
    __mmask8 mask = _mm512_cmpeq_epi64_mask(mask_data, _mm512_setzero_si512());
    
    /* Perform blend - this should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, src1, src2);
    
    _mm512_store_si512((__m512i*)g_i64_data, result);
    
    /* Compute checksum */
    g_checksum += _mm512_reduce_add_epi64(result);
}

/* V8DFmode: 8 double-precision float blend */
__attribute__((target("avx512f")))
static void test_v8df_blend(void)
{
    /* Initialize source data */
    __m512d src1 = _mm512_set1_pd(1.0);
    __m512d src2 = _mm512_set1_pd(2.0);
    
    /* Create dynamic mask */
    __m512d mask_data = _mm512_load_pd(g_f64_data);
    __mmask8 mask = _mm512_cmp_pd_mask(mask_data, _mm512_setzero_pd(), _CMP_EQ_OQ);
    
    /* Perform blend - this should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, src1, src2);
    
    _mm512_store_pd(g_f64_data, result);
    
    /* Compute checksum */
    __m256d lo = _mm512_castpd512_pd256(result);
    __m256d hi = _mm512_extractf64x4_pd(result, 1);
    
    double sum = 0.0;
    for (int i = 0; i < 4; i++) sum += lo[i];
    for (int i = 0; i < 4; i++) sum += hi[i];
    g_checksum += (uint64_t)(sum * 1000);
}

/* V16SFmode: 16 single-precision float blend */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void)
{
    /* Initialize source data */
    __m512 src1 = _mm512_set1_ps(1.0f);
    __m512 src2 = _mm512_set1_ps(2.0f);
    
    /* Create dynamic mask */
    __m512 mask_data = _mm512_load_ps(g_f32_data);
    __mmask16 mask = _mm512_cmp_ps_mask(mask_data, _mm512_setzero_ps(), _CMP_EQ_OQ);
    
    /* Perform blend - this should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, src1, src2);
    
    _mm512_store_ps(g_f32_data, result);
    
    /* Compute checksum */
    __m256 lo = _mm512_castps512_ps256(result);
    __m256 hi = _mm512_extractf32x8_ps(result, 1);
    
    float sum = 0.0f;
    for (int i = 0; i < 8; i++) sum += lo[i];
    for (int i = 0; i < 8; i++) sum += hi[i];
    g_checksum += (uint64_t)(sum * 1000);
}
#endif /* __AVX512F__ */

/* Initialize test data with non-zero, non-constant patterns */
static void init_test_data(void)
{
    /* Use a simple LCG to generate pseudo-random but deterministic data */
    uint32_t seed = 0xDEADBEEF;
    
    for (int i = 0; i < 64; i++) {
        seed = seed * 1103515245 + 12345;
        g_u8_data[i] = (seed >> 16) & 0xFF;
    }
    
    for (int i = 0; i < 32; i++) {
        seed = seed * 1103515245 + 12345;
        g_u16_data[i] = (seed >> 16) & 0xFFFF;
    }
    
    for (int i = 0; i < 16; i++) {
        seed = seed * 1103515245 + 12345;
        g_i32_data[i] = (int32_t)seed;
    }
    
    for (int i = 0; i < 8; i++) {
        seed = seed * 1103515245 + 12345;
        g_i64_data[i] = ((int64_t)seed << 32) | (seed * 1664525 + 1013904223);
    }
    
    for (int i = 0; i < 16; i++) {
        seed = seed * 1103515245 + 12345;
        g_f32_data[i] = (float)(seed % 100) / 10.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        seed = seed * 1103515245 + 12345;
        g_f64_data[i] = (double)(seed % 100) / 10.0;
    }
    
#ifdef __AVX512FP16__
    for (int i = 0; i < 32; i++) {
        seed = seed * 1103515245 + 12345;
        g_f16_data[i] = (_Float16)((seed % 100) / 10.0f);
    }
#endif
    
#ifdef __AVX512BF16__
    for (int i = 0; i < 32; i++) {
        seed = seed * 1103515245 + 12345;
        ((uint16_t*)g_bf16_data)[i] = (seed >> 16) & 0xFFFF;
    }
#endif
}

int main(void)
{
    /* Initialize with non-constant data */
    init_test_data();
    
#ifdef __AVX512F__
    printf("Testing AVX-512 blend instruction expansion...\n");
    
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
    
    printf("All blend tests completed. Checksum: %llu\n", 
           (unsigned long long)g_checksum);
#else
    printf("AVX-512 not supported on this platform\n");
#endif
    
    return 0;
}

#ifdef __cplusplus
}
#endif
