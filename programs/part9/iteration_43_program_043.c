/* Test program to trigger AVX-512 blend operations in i386-expand.cc
 * Specifically targets the uncovered lines 4303-4326
 */

#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __AVX512F__

/* 32-bit integer blend (E_V16SImode) */
static uint64_t test_blend_epi32(void) {
    alignas(64) int32_t src1[16], src2[16], dst[16];
    uint64_t sum = 0;
    
    /* Fill with non-uniform data */
    for (int i = 0; i < 16; i++) {
        src1[i] = (i * 3) % 31;
        src2[i] = (i * 7) % 29;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create data-dependent mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(15);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Perform blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)dst[i];
    }
    
    return sum;
}

/* 64-bit integer blend (E_V8DImode) */
static uint64_t test_blend_epi64(void) {
    alignas(64) int64_t src1[8], src2[8], dst[8];
    uint64_t sum = 0;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (i * 5) % 37;
        src2[i] = (i * 11) % 41;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Dynamic mask based on comparison */
    __m512i cmp_val = _mm512_set1_epi64(20);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)dst[i];
    }
    
    return sum;
}

/* Single-precision float blend (E_V16SFmode) */
static float test_blend_ps(void) {
    alignas(64) float src1[16], src2[16], dst[16];
    float sum = 0.0f;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(i * 2) / 7.0f;
        src2[i] = (float)(i * 3) / 11.0f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    /* Create mask using floating-point comparison */
    __m512 cmp_val = _mm512_set1_ps(1.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(dst, result);
    
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Double-precision float blend (E_V8DFmode) */
static double test_blend_pd(void) {
    alignas(64) double src1[8], src2[8], dst[8];
    double sum = 0.0;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(i * 3) / 13.0;
        src2[i] = (double)(i * 5) / 17.0;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __m512d cmp_val = _mm512_set1_pd(0.5);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_storeu_pd(dst, result);
    
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#ifdef __AVX512BW__
/* 16-bit integer blend (E_V32HImode) */
static uint64_t test_blend_epi16(void) {
    alignas(64) int16_t src1[32], src2[32], dst[32];
    uint64_t sum = 0;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)((i * 7) % 127);
        src2[i] = (int16_t)((i * 13) % 127);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create dynamic mask */
    __m512i cmp_val = _mm512_set1_epi16(50);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    for (int i = 0; i < 32; i++) {
        sum += (uint64_t)dst[i];
    }
    
    return sum;
}

/* 8-bit integer blend (E_V64QImode) */
static uint64_t test_blend_epi8(void) {
    alignas(64) int8_t src1[64], src2[64], dst[64];
    uint64_t sum = 0;
    
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)((i * 3) % 31);
        src2[i] = (int8_t)((i * 5) % 31);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi8(15);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    for (int i = 0; i < 64; i++) {
        sum += (uint64_t)dst[i];
    }
    
    return sum;
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* Half-precision float blend (E_V32HFmode) */
static float test_blend_ph(void) {
    alignas(64) _Float16 src1[32], src2[32], dst[32];
    float sum = 0.0f;
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)((float)(i * 2) / 19.0f);
        src2[i] = (_Float16)((float)(i * 3) / 23.0f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    __m512h cmp_val = _mm512_set1_ph((_Float16)0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(dst, result);
    
    for (int i = 0; i < 32; i++) {
        sum += (float)dst[i];
    }
    
    return sum;
}
#endif /* __AVX512FP16__ */

#endif /* __AVX512F__ */

int main(void) {
    srand(42);  /* Seed for reproducibility */
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    uint64_t sum1 = test_blend_epi32();
    printf("  blend_epi32 checksum: %lu\n", sum1);
    
    uint64_t sum2 = test_blend_epi64();
    printf("  blend_epi64 checksum: %lu\n", sum2);
    
    float sum3 = test_blend_ps();
    printf("  blend_ps checksum: %f\n", sum3);
    
    double sum4 = test_blend_pd();
    printf("  blend_pd checksum: %f\n", sum4);
    
#ifdef __AVX512BW__
    uint64_t sum5 = test_blend_epi16();
    printf("  blend_epi16 checksum: %lu\n", sum5);
    
    uint64_t sum6 = test_blend_epi8();
    printf("  blend_epi8 checksum: %lu\n", sum6);
#endif
    
#ifdef __AVX512FP16__
    float sum7 = test_blend_ph();
    printf("  blend_ph checksum: %f\n", sum7);
#endif
    
    printf("All blend tests completed.\n");
#else
    printf("AVX-512 not supported on this platform.\n");
#endif
    
    return 0;
}
