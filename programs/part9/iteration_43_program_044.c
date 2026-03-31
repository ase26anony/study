/* Test program to trigger AVX-512 blend operations in i386-expand.cc
 * Specifically targets the uncovered lines 4303-4326
 */

#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Function to test 64x8-bit blend (E_V64QImode) */
__attribute__((noinline))
uint64_t test_blend_epi8(uint8_t* src1, uint8_t* src2, uint8_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create non-constant mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(128);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    /* Perform the blend operation */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    /* Compute checksum to prevent optimization */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Function to test 32x16-bit blend (E_V32HImode) */
__attribute__((noinline))
uint64_t test_blend_epi16(int16_t* src1, int16_t* src2, int16_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create dynamic mask */
    __m512i cmp_val = _mm512_set1_epi16(0);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Perform blend */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}

#endif /* __AVX512BW__ */

/* Function to test 16x32-bit integer blend (E_V16SImode) */
__attribute__((noinline))
uint64_t test_blend_epi32(int32_t* src1, int32_t* src2, int32_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create data-dependent mask */
    __m512i cmp_val = _mm512_set1_epi32(1000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend operation */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Function to test 8x64-bit integer blend (E_V8DImode) */
__attribute__((noinline))
uint64_t test_blend_epi64(int64_t* src1, int64_t* src2, int64_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Dynamic mask generation */
    __m512i cmp_val = _mm512_set1_epi64(5000);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    /* Blend */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Function to test 16x32-bit float blend (E_V16SFmode) */
__attribute__((noinline))
float test_blend_ps(float* src1, float* src2, float* dst) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    /* Non-constant mask from comparison */
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    _mm512_storeu_ps(dst, result);
    
    /* Compute checksum */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Function to test 8x64-bit double blend (E_V8DFmode) */
__attribute__((noinline))
double test_blend_pd(double* src1, double* src2, double* dst) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    /* Dynamic mask */
    __m512d cmp_val = _mm512_set1_pd(0.25);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend */
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_storeu_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

#ifdef __AVX512FP16__

/* Function to test 32x16-bit float blend (E_V32HFmode) */
__attribute__((noinline))
float test_blend_ph(_Float16* src1, _Float16* src2, _Float16* dst) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    /* Create mask */
    __m512h cmp_val = _mm512_set1_ph(0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512h result = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_storeu_ph(dst, result);
    
    /* Compute checksum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}

#endif /* __AVX512FP16__ */

#endif /* __AVX512F__ */

int main() {
    srand(42);  /* Seed for reproducible results */
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    
    /* Test data for different types */
    int32_t src1_32[16], src2_32[16], dst_32[16];
    int64_t src1_64[8], src2_64[8], dst_64[8];
    float src1_f[16], src2_f[16], dst_f[16];
    double src1_d[8], src2_d[8], dst_d[8];
    
    /* Initialize with random values */
    for (int i = 0; i < 16; i++) {
        src1_32[i] = rand() % 2000;
        src2_32[i] = rand() % 2000;
        src1_f[i] = (float)rand() / RAND_MAX;
        src2_f[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_64[i] = rand() % 10000;
        src2_64[i] = rand() % 10000;
        src1_d[i] = (double)rand() / RAND_MAX;
        src2_d[i] = (double)rand() / RAND_MAX;
    }
    
    /* Execute blend operations */
    uint64_t sum32 = test_blend_epi32(src1_32, src2_32, dst_32);
    uint64_t sum64 = test_blend_epi64(src1_64, src2_64, dst_64);
    float sumf = test_blend_ps(src1_f, src2_f, dst_f);
    double sumd = test_blend_pd(src1_d, src2_d, dst_d);
    
    printf("32-bit integer blend checksum: %lu\n", sum32);
    printf("64-bit integer blend checksum: %lu\n", sum64);
    printf("32-bit float blend checksum: %f\n", sumf);
    printf("64-bit double blend checksum: %f\n", sumd);
    
#ifdef __AVX512BW__
    
    /* Test data for 8-bit and 16-bit types */
    uint8_t src1_8[64], src2_8[64], dst_8[64];
    int16_t src1_16[32], src2_16[32], dst_16[32];
    
    for (int i = 0; i < 64; i++) {
        src1_8[i] = rand() % 256;
        src2_8[i] = rand() % 256;
    }
    
    for (int i = 0; i < 32; i++) {
        src1_16[i] = (rand() % 2000) - 1000;
        src2_16[i] = (rand() % 2000) - 1000;
    }
    
    /* Execute blend operations */
    uint64_t sum8 = test_blend_epi8(src1_8, src2_8, dst_8);
    uint64_t sum16 = test_blend_epi16(src1_16, src2_16, dst_16);
    
    printf("8-bit integer blend checksum: %lu\n", sum8);
    printf("16-bit integer blend checksum: %lu\n", sum16);
    
#endif /* __AVX512BW__ */
    
#ifdef __AVX512FP16__
    
    /* Test data for half precision */
    _Float16 src1_h[32], src2_h[32], dst_h[32];
    
    for (int i = 0; i < 32; i++) {
        src1_h[i] = (_Float16)((float)rand() / RAND_MAX);
        src2_h[i] = (_Float16)((float)rand() / RAND_MAX);
    }
    
    float sumh = test_blend_ph(src1_h, src2_h, dst_h);
    printf("16-bit float blend checksum: %f\n", sumh);
    
#endif /* __AVX512FP16__ */
    
#endif /* __AVX512F__ */
    
    printf("Blend operations completed.\n");
    return 0;
}
