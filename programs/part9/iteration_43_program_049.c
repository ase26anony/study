/* Test program to trigger AVX-512 blend expansion patterns in i386-expand.cc */
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Function prototypes for different blend operations */
#ifdef __AVX512F__
void test_blend_epi32(int32_t* result);
void test_blend_epi64(int64_t* result);
void test_blend_ps(float* result);
void test_blend_pd(double* result);
#endif

#ifdef __AVX512BW__
void test_blend_epi16(int16_t* result);
void test_blend_epi8(int8_t* result);
#endif

#ifdef __AVX512FP16__
void test_blend_ph(_Float16* result);
#endif

int main() {
    /* Seed random number generator for dynamic data */
    srand(42);
    
    printf("Testing AVX-512 blend operations for i386-expand.cc coverage\n");
    
#ifdef __AVX512F__
    /* Test 32-bit integer blend (E_V16SImode) */
    {
        int32_t result[16] = {0};
        test_blend_epi32(result);
        int32_t sum = 0;
        for (int i = 0; i < 16; i++) sum += result[i];
        printf("V16SImode blend checksum: %d\n", sum);
    }
    
    /* Test 64-bit integer blend (E_V8DImode) */
    {
        int64_t result[8] = {0};
        test_blend_epi64(result);
        int64_t sum = 0;
        for (int i = 0; i < 8; i++) sum += result[i];
        printf("V8DImode blend checksum: %ld\n", sum);
    }
    
    /* Test single-precision blend (E_V16SFmode) */
    {
        float result[16] = {0};
        test_blend_ps(result);
        float sum = 0;
        for (int i = 0; i < 16; i++) sum += result[i];
        printf("V16SFmode blend checksum: %f\n", sum);
    }
    
    /* Test double-precision blend (E_V8DFmode) */
    {
        double result[8] = {0};
        test_blend_pd(result);
        double sum = 0;
        for (int i = 0; i < 8; i++) sum += result[i];
        printf("V8DFmode blend checksum: %f\n", sum);
    }
#endif
    
#ifdef __AVX512BW__
    /* Test 16-bit integer blend (E_V32HImode) */
    {
        int16_t result[32] = {0};
        test_blend_epi16(result);
        int16_t sum = 0;
        for (int i = 0; i < 32; i++) sum += result[i];
        printf("V32HImode blend checksum: %d\n", sum);
    }
    
    /* Test 8-bit integer blend (E_V64QImode) */
    {
        int8_t result[64] = {0};
        test_blend_epi8(result);
        int8_t sum = 0;
        for (int i = 0; i < 64; i++) sum += result[i];
        printf("V64QImode blend checksum: %d\n", sum);
    }
#endif
    
#ifdef __AVX512FP16__
    /* Test half-precision blend (E_V32HFmode/E_V32BFmode) */
    {
        _Float16 result[32] = {0};
        test_blend_ph(result);
        _Float16 sum = 0;
        for (int i = 0; i < 32; i++) sum += result[i];
        printf("V32HFmode blend checksum: %f\n", (double)sum);
    }
#endif
    
    printf("All blend tests completed.\n");
    return 0;
}

#ifdef __AVX512F__
/* Test blend for 32-bit integers (targets gen_avx512f_blendmv16si) */
void test_blend_epi32(int32_t* result) {
    /* Create non-constant data arrays */
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = rand() % 100;
        src2[i] = rand() % 100 + 100;
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create dynamic mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(50);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Perform blend operation */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Store result */
    _mm512_storeu_si512((__m512i*)result, blended);
}

/* Test blend for 64-bit integers (targets gen_avx512f_blendmv8di) */
void test_blend_epi64(int64_t* result) {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = rand() % 1000;
        src2[i] = rand() % 1000 + 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Use arithmetic operation to create varied data */
    __m512i v1_squared = _mm512_mullo_epi64(v1, v1);
    __m512i v2_squared = _mm512_mullo_epi64(v2, v2);
    
    /* Dynamic mask based on comparison */
    __m512i threshold = _mm512_set1_epi64(250000);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1_squared, threshold);
    
    /* Blend the squared values */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1_squared, v2_squared);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

/* Test blend for single-precision (targets gen_avx512f_blendmv16sf) */
void test_blend_ps(float* result) {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(rand() % 100) / 10.0f;
        src2[i] = (float)(rand() % 100 + 100) / 10.0f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    /* Create mask using floating-point comparison */
    __m512 threshold = _mm512_set1_ps(5.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(result, blended);
}

/* Test blend for double-precision (targets gen_avx512f_blendmv8df) */
void test_blend_pd(double* result) {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(rand() % 100) / 10.0;
        src2[i] = (double)(rand() % 100 + 100) / 10.0;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    /* Perform arithmetic operation before blend */
    __m512d v1_sqrt = _mm512_sqrt_pd(v1);
    __m512d v2_sqrt = _mm512_sqrt_pd(v2);
    
    /* Dynamic mask */
    __m512d threshold = _mm512_set1_pd(2.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1_sqrt, threshold, _CMP_LT_OQ);
    
    /* Blend the square roots */
    __m512d blended = _mm512_mask_blend_pd(mask, v1_sqrt, v2_sqrt);
    
    _mm512_storeu_pd(result, blended);
}
#endif

#ifdef __AVX512BW__
/* Test blend for 16-bit integers (targets gen_avx512bw_blendmv32hi) */
void test_blend_epi16(int16_t* result) {
    int16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(rand() % 1000);
        src2[i] = (int16_t)(rand() % 1000 + 1000);
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create complex mask using multiple conditions */
    __m512i threshold = _mm512_set1_epi16(500);
    __mmask32 mask1 = _mm512_cmpgt_epi16_mask(v1, threshold);
    
    /* Second condition for more complex mask */
    __m512i v1_shifted = _mm512_slli_epi16(v1, 1);
    __mmask32 mask2 = _mm512_cmpeq_epi16_mask(v1_shifted, v2);
    
    /* Combine masks for non-trivial pattern */
    __mmask32 final_mask = mask1 ^ mask2;  /* XOR creates interesting pattern */
    
    /* Blend operation */
    __m512i blended = _mm512_mask_blend_epi16(final_mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

/* Test blend for 8-bit integers (targets gen_avx512bw_blendmv64qi) */
void test_blend_epi8(int8_t* result) {
    int8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(rand() % 100);
        src2[i] = (int8_t)(rand() % 100 + 100);
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    /* Create data-dependent mask */
    __m512i threshold = _mm512_set1_epi8(50);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, threshold);
    
    /* Blend operation */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}
#endif

#ifdef __AVX512FP16__
/* Test blend for half-precision (targets gen_avx512bw_blendmv32hf/bf) */
void test_blend_ph(_Float16* result) {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(rand() % 100) / 10.0f;
        src2[i] = (_Float16)(rand() % 100 + 100) / 10.0f;
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    /* Create mask using half-precision comparison */
    __m512h threshold = _mm512_set1_ph((_Float16)5.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, threshold, _CMP_GT_OQ);
    
    /* Blend operation */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(result, blended);
}
#endif
