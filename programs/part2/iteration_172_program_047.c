/* avx512_blend_coverage.c
 * Test program to cover AVX-512 blend instruction expansion in GCC's i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_coverage.c -o avx512_blend_coverage
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes with target attributes */
#ifdef __AVX512F__
#ifdef __AVX512BW__
/* Integer blend functions requiring AVX512BW */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void);
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void);
#endif

#ifdef __AVX512FP16__
/* Half-precision float blend functions */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void);
#endif

#ifdef __AVX512BF16__
/* BFloat16 blend functions */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void);
#endif

/* AVX512F blend functions */
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
static uint64_t g_checksum = 0;

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* Test V64QImode blend: 64 x 8-bit integers */
__attribute__((target("avx512f,avx512bw")))
static void test_v64qi_blend(void) {
    /* Use volatile to prevent constant propagation */
    volatile int8_t src1[64] __attribute__((aligned(64)));
    volatile int8_t src2[64] __attribute__((aligned(64)));
    int8_t dst[64] __attribute__((aligned(64)));
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    /* Load into registers */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create dynamic mask based on data pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Perform blend - this should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store and compute checksum */
    _mm512_store_si512((__m512i*)dst, result);
    for (int i = 0; i < 64; i++) {
        g_checksum += dst[i];
    }
}

/* Test V32HImode blend: 32 x 16-bit integers */
__attribute__((target("avx512f,avx512bw")))
static void test_v32hi_blend(void) {
    volatile int16_t src1[32] __attribute__((aligned(64)));
    volatile int16_t src2[32] __attribute__((aligned(64)));
    int16_t dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Dynamic mask using comparison */
    __mmask32 mask = _mm512_cmpeq_epi16_mask(v1, _mm512_set1_epi16(0));
    mask = ~mask; /* Invert to get non-constant mask */
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    for (int i = 0; i < 32; i++) {
        g_checksum += dst[i];
    }
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* Test V32HFmode blend: 32 x half-precision floats */
__attribute__((target("avx512f,avx512fp16")))
static void test_v32hf_blend(void) {
    volatile _Float16 src1[32] __attribute__((aligned(64)));
    volatile _Float16 src2[32] __attribute__((aligned(64)));
    _Float16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 0.75f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(dst, result);
    for (int i = 0; i < 32; i++) {
        g_checksum += (uint64_t)(dst[i] * 1000);
    }
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* Test V32BFmode blend: 32 x bfloat16 */
__attribute__((target("avx512f,avx512bf16")))
static void test_v32bf_blend(void) {
    /* Use __bf16 type for bfloat16 */
    volatile __bf16 src1[32] __attribute__((aligned(64)));
    volatile __bf16 src2[32] __attribute__((aligned(64)));
    __bf16 dst[32] __attribute__((aligned(64)));
    
    for (int i = 0; i < 32; i++) {
        /* Simple pattern */
        src1[i] = (__bf16)((i & 1) ? 1.0f : 0.5f);
        src2[i] = (__bf16)((i & 1) ? 0.5f : 1.0f);
    }
    
    /* Load bfloat16 vectors */
    __m512bh v1 = _mm512_load_si512((const __m512i*)src1);
    __m512bh v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - use integer comparison on the bit pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 4) < 2) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend bfloat16 - should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    for (int i = 0; i < 32; i++) {
        g_checksum += (uint64_t)(*(uint16_t*)&dst[i]);
    }
}
#endif /* __AVX512BF16__ */

/* Test V16SImode blend: 16 x 32-bit integers */
__attribute__((target("avx512f")))
static void test_v16si_blend(void) {
    volatile int32_t src1[16] __attribute__((aligned(64)));
    volatile int32_t src2[16] __attribute__((aligned(64)));
    int32_t dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 15;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Dynamic mask using runtime calculation */
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((src1[i] % 20) == 0) {
            mask |= (1 << i);
        }
    }
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    for (int i = 0; i < 16; i++) {
        g_checksum += dst[i];
    }
}

/* Test V8DImode blend: 8 x 64-bit integers */
__attribute__((target("avx512f")))
static void test_v8di_blend(void) {
    volatile int64_t src1[8] __attribute__((aligned(64)));
    volatile int64_t src2[8] __attribute__((aligned(64)));
    int64_t dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 150LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __mmask8 mask = _mm512_cmpeq_epi64_mask(v1, _mm512_set1_epi64(0));
    mask = ~mask; /* Ensure non-zero mask */
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, result);
    for (int i = 0; i < 8; i++) {
        g_checksum += dst[i];
    }
}

/* Test V8DFmode blend: 8 x double precision floats */
__attribute__((target("avx512f")))
static void test_v8df_blend(void) {
    volatile double src1[8] __attribute__((aligned(64)));
    volatile double src2[8] __attribute__((aligned(64)));
    double dst[8] __attribute__((aligned(64)));
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Dynamic mask using comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(dst, result);
    for (int i = 0; i < 8; i++) {
        g_checksum += (uint64_t)(dst[i] * 1000);
    }
}

/* Test V16SFmode blend: 16 x single precision floats */
__attribute__((target("avx512f")))
static void test_v16sf_blend(void) {
    volatile float src1[16] __attribute__((aligned(64)));
    volatile float src2[16] __attribute__((aligned(64)));
    float dst[16] __attribute__((aligned(64)));
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.25f;
        src2[i] = i * 0.375f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create pattern-based mask */
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i % 5) != 0) {
            mask |= (1 << i);
        }
    }
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(dst, result);
    for (int i = 0; i < 16; i++) {
        g_checksum += (uint64_t)(dst[i] * 1000);
    }
}
#endif /* __AVX512F__ */

#ifdef __cplusplus
}
#endif

int main(void) {
    printf("Testing AVX-512 blend instruction expansion coverage...\n");
    
#ifdef __AVX512F__
    printf("AVX512F supported\n");
    
    /* Test all AVX512F blend modes */
    test_v16si_blend();
    test_v8di_blend();
    test_v8df_blend();
    test_v16sf_blend();
    
#ifdef __AVX512BW__
    printf("AVX512BW supported\n");
    test_v64qi_blend();
    test_v32hi_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("AVX512FP16 supported\n");
    test_v32hf_blend();
#endif
    
#ifdef __AVX512BF16__
    printf("AVX512BF16 supported\n");
    test_v32bf_blend();
#endif
    
#else
    printf("AVX512F not supported - skipping tests\n");
#endif
    
    printf("Checksum: %lu\n", g_checksum);
    printf("Test completed.\n");
    
    return 0;
}
