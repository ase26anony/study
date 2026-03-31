/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction expansion coverage */
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
void test_v64qi_blend(void);
__attribute__((target("avx512f,avx512bw")))
void test_v32hi_blend(void);
#endif

#ifdef __AVX512FP16__
/* Half-precision float blend functions */
__attribute__((target("avx512f,avx512fp16")))
void test_v32hf_blend(void);
#endif

#ifdef __AVX512BF16__
/* BFloat16 blend functions */
__attribute__((target("avx512f,avx512bf16")))
void test_v32bf_blend(void);
#endif

/* Functions requiring only AVX512F */
__attribute__((target("avx512f")))
void test_v16si_blend(void);
__attribute__((target("avx512f")))
void test_v8di_blend(void);
__attribute__((target("avx512f")))
void test_v8df_blend(void);
__attribute__((target("avx512f")))
void test_v16sf_blend(void);
#endif

/* Global arrays to prevent optimization */
static uint64_t g_checksum = 0;

#ifdef __AVX512F__
#ifdef __AVX512BW__
/* V64QImode test - 64 bytes */
__attribute__((target("avx512f,avx512bw")))
void test_v64qi_blend(void) {
    /* Create two distinct 64-byte patterns */
    uint8_t src1[64], src2[64], dst[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = (uint8_t)(i * 3 + 1);
        src2[i] = (uint8_t)(i * 5 + 2);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create dynamic mask based on pattern - every other byte */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i & 1) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    /* Update checksum */
    for (int i = 0; i < 64; i++) {
        g_checksum += dst[i];
    }
}

/* V32HImode test - 32 half-words */
__attribute__((target("avx512f,avx512bw")))
void test_v32hi_blend(void) {
    uint16_t src1[32], src2[32], dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 7 + 1);
        src2[i] = (uint16_t)(i * 11 + 2);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create dynamic mask - pattern based on i % 3 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 3) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    for (int i = 0; i < 32; i++) {
        g_checksum += dst[i];
    }
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode test - 32 half-precision floats */
__attribute__((target("avx512f,avx512fp16")))
void test_v32hf_blend(void) {
    _Float16 src1[32], src2[32], dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f + 1.0f);
        src2[i] = (_Float16)(i * 0.3f + 2.0f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    /* Create dynamic mask - alternating pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i & 2) == 0) {  /* Different pattern from previous tests */
            mask |= (1U << i);
        }
    }
    
    /* Perform the blend operation */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(dst, result);
    
    /* Convert to integer for checksum */
    for (int i = 0; i < 32; i++) {
        g_checksum += (uint64_t)(dst[i] * 1000);
    }
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode test - 32 bfloat16 values */
__attribute__((target("avx512f,avx512bf16")))
void test_v32bf_blend(void) {
    /* Use __bf16 type if available, otherwise use uint16_t */
    #ifdef __bf16
    __bf16 src1[32], src2[32], dst[32];
    #else
    uint16_t src1[32], src2[32], dst[32];
    #endif
    
    for (int i = 0; i < 32; i++) {
        float f1 = i * 0.25f + 1.0f;
        float f2 = i * 0.15f + 2.0f;
        #ifdef __bf16
        src1[i] = (__bf16)f1;
        src2[i] = (__bf16)f2;
        #else
        /* Simple bfloat16 encoding (just truncate) */
        uint32_t u1 = *(uint32_t*)&f1;
        uint32_t u2 = *(uint32_t*)&f2;
        src1[i] = (uint16_t)(u1 >> 16);
        src2[i] = (uint16_t)(u2 >> 16);
        #endif
    }
    
    /* Load as __m512bh for bfloat16 */
    __m512bh v1 = _mm512_loadu_ph((const void*)src1);
    __m512bh v2 = _mm512_loadu_ph((const void*)src2);
    
    /* Create dynamic mask - pattern based on i % 5 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 5) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Perform the blend operation - same intrinsic as FP16 */
    __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph((void*)dst, result);
    
    for (int i = 0; i < 32; i++) {
        g_checksum += dst[i];
    }
}
#endif /* __AVX512BF16__ */

/* V16SImode test - 16 signed integers */
__attribute__((target("avx512f")))
void test_v16si_blend(void) {
    int32_t src1[16], src2[16], dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10 + 1;
        src2[i] = i * 20 + 2;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create dynamic mask using comparison */
    __m512i cmp = _mm512_set1_epi32(0);
    __mmask16 mask = _mm512_cmpeq_epi32_mask(v1, cmp);
    
    /* Invert mask to get some ones */
    mask = ~mask;
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    for (int i = 0; i < 16; i++) {
        g_checksum += dst[i];
    }
}

/* V8DImode test - 8 double integers */
__attribute__((target("avx512f")))
void test_v8di_blend(void) {
    int64_t src1[8], src2[8], dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL + 1;
        src2[i] = i * 200LL + 2;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create dynamic mask - pattern 0b10101010 */
    __mmask8 mask = 0xAA;
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    for (int i = 0; i < 8; i++) {
        g_checksum += dst[i];
    }
}

/* V8DFmode test - 8 double floats */
__attribute__((target("avx512f")))
void test_v8df_blend(void) {
    double src1[8], src2[8], dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5 + 1.0;
        src2[i] = i * 2.5 + 2.0;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    /* Create dynamic mask using comparison */
    __m512d zero = _mm512_setzero_pd();
    __mmask8 mask = _mm512_cmp_pd_mask(v1, zero, _CMP_GT_OQ);
    
    /* Perform the blend operation */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_storeu_pd(dst, result);
    
    for (int i = 0; i < 8; i++) {
        g_checksum += (uint64_t)(dst[i] * 1000);
    }
}

/* V16SFmode test - 16 single floats */
__attribute__((target("avx512f")))
void test_v16sf_blend(void) {
    float src1[16], src2[16], dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f + 1.0f;
        src2[i] = i * 1.25f + 2.0f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    /* Create dynamic mask using comparison */
    __m512 zero = _mm512_setzero_ps();
    __mmask16 mask = _mm512_cmp_ps_mask(v1, zero, _CMP_GT_OQ);
    
    /* Perform the blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(dst, result);
    
    for (int i = 0; i < 16; i++) {
        g_checksum += (uint64_t)(dst[i] * 1000);
    }
}
#endif /* __AVX512F__ */

#ifdef __cplusplus
}
#endif

int main(void) {
    printf("Starting AVX-512 blend coverage test...\n");
    
    #ifdef __AVX512F__
    printf("AVX512F supported\n");
    
    /* Test all AVX512F blend operations */
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
    
    printf("Checksum: %lu\n", g_checksum);
    printf("All AVX-512 blend tests completed.\n");
    
    #else
    printf("AVX512F not supported - skipping tests\n");
    #endif
    
    return 0;
}
