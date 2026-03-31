#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Function to test 32-bit integer blend (E_V16SImode)
void test_blend_epi32(int32_t* result) {
    alignas(64) int32_t src1[16], src2[16];
    
    // Initialize with non-uniform values
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 3 + 1;
        src2[i] = i * 5 - 2;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create dynamic mask using comparison
    __m512i cmp_val = _mm512_set1_epi32(20);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    // Perform blend operation
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// Function to test 64-bit integer blend (E_V8DImode)
void test_blend_epi64(int64_t* result) {
    alignas(64) int64_t src1[8], src2[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 7LL + 3;
        src2[i] = i * 11LL - 5;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi64(25);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// Function to test single-precision blend (E_V16SFmode)
void test_blend_ps(float* result) {
    alignas(64) float src1[16], src2[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f + 0.1f;
        src2[i] = i * 0.3f - 0.2f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    __m512 cmp_val = _mm512_set1_ps(3.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(result, blended);
}

// Function to test double-precision blend (E_V8DFmode)
void test_blend_pd(double* result) {
    alignas(64) double src1[8], src2[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 0.7 + 0.15;
        src2[i] = i * 0.9 - 0.25;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __m512d cmp_val = _mm512_set1_pd(2.5);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_storeu_pd(result, blended);
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
void test_blend_epi16(int16_t* result) {
    alignas(64) int16_t src1[32], src2[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2 + 1;
        src2[i] = i * 3 - 1;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi16(30);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}

// Function to test 8-bit integer blend (E_V64QImode)
void test_blend_epi8(int8_t* result) {
    alignas(64) int8_t src1[64], src2[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = (i % 16) - 8;
        src2[i] = (i % 12) - 6;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi8(0);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision blend (E_V32HFmode)
void test_blend_ph(_Float16* result) {
    alignas(64) _Float16 src1[32], src2[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.2f + 0.05f);
        src2[i] = (_Float16)(i * 0.15f - 0.1f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    __m512h cmp_val = _mm512_set1_ph((_Float16)2.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(result, blended);
}
#endif

int main() {
    srand(42);
    
    // Test results storage
    alignas(64) int32_t result_i32[16];
    alignas(64) int64_t result_i64[8];
    alignas(64) float result_f32[16];
    alignas(64) double result_f64[8];
    
    // Test each blend operation
    test_blend_epi32(result_i32);
    test_blend_epi64(result_i64);
    test_blend_ps(result_f32);
    test_blend_pd(result_f64);
    
    // Compute checksums to prevent dead code elimination
    int64_t checksum = 0;
    
    for (int i = 0; i < 16; i++) {
        checksum += result_i32[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum += result_i64[i];
    }
    
    printf("Basic checksum: %ld\n", checksum);
    
#ifdef __AVX512BW__
    alignas(64) int16_t result_i16[32];
    alignas(64) int8_t result_i8[64];
    
    test_blend_epi16(result_i16);
    test_blend_epi8(result_i8);
    
    for (int i = 0; i < 32; i++) {
        checksum += result_i16[i];
    }
    
    for (int i = 0; i < 64; i++) {
        checksum += result_i8[i];
    }
    
    printf("With AVX512BW checksum: %ld\n", checksum);
#endif

#ifdef __AVX512FP16__
    alignas(64) _Float16 result_f16[32];
    
    test_blend_ph(result_f16);
    
    // Convert half to int for checksum
    for (int i = 0; i < 32; i++) {
        checksum += (int)(result_f16[i] * 100);
    }
    
    printf("With AVX512FP16 checksum: %ld\n", checksum);
#endif
    
    return 0;
}
