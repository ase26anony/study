#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Function to test 32-bit integer blend (E_V16SImode)
void test_blend_epi32(int32_t* src1, int32_t* src2, int32_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create non-constant mask using comparison
    __m512i cmp_val = _mm512_set1_epi32(100);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
}

// Function to test 64-bit integer blend (E_V8DImode)
void test_blend_epi64(int64_t* src1, int64_t* src2, int64_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create non-constant mask
    __m512i cmp_val = _mm512_set1_epi64(500);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
}

// Function to test single-precision blend (E_V16SFmode)
void test_blend_ps(float* src1, float* src2, float* dst) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create non-constant mask
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(dst, result);
}

// Function to test double-precision blend (E_V8DFmode)
void test_blend_pd(double* src1, double* src2, double* dst) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create non-constant mask
    __m512d cmp_val = _mm512_set1_pd(0.5);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_storeu_pd(dst, result);
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
void test_blend_epi16(int16_t* src1, int16_t* src2, int16_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create non-constant mask
    __m512i cmp_val = _mm512_set1_epi16(50);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
}

// Function to test 8-bit integer blend (E_V64QImode)
void test_blend_epi8(int8_t* src1, int8_t* src2, int8_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create non-constant mask
    __m512i cmp_val = _mm512_set1_epi8(25);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)dst, result);
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision blend (E_V32HFmode)
void test_blend_ph(_Float16* src1, _Float16* src2, _Float16* dst) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create non-constant mask
    __m512h cmp_val = _mm512_set1_ph(0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(dst, result);
}
#endif

int main() {
    srand(42);
    
    // Initialize arrays with random data
    int32_t src1_epi32[16], src2_epi32[16], dst_epi32[16];
    int64_t src1_epi64[8], src2_epi64[8], dst_epi64[8];
    float src1_ps[16], src2_ps[16], dst_ps[16];
    double src1_pd[8], src2_pd[8], dst_pd[8];
    
    // Fill arrays with non-uniform values
    for (int i = 0; i < 16; i++) {
        src1_epi32[i] = rand() % 200;
        src2_epi32[i] = rand() % 200;
        src1_ps[i] = (float)rand() / RAND_MAX;
        src2_ps[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_epi64[i] = rand() % 1000;
        src2_epi64[i] = rand() % 1000;
        src1_pd[i] = (double)rand() / RAND_MAX;
        src2_pd[i] = (double)rand() / RAND_MAX;
    }
    
    // Test all blend operations
    test_blend_epi32(src1_epi32, src2_epi32, dst_epi32);
    test_blend_epi64(src1_epi64, src2_epi64, dst_epi64);
    test_blend_ps(src1_ps, src2_ps, dst_ps);
    test_blend_pd(src1_pd, src2_pd, dst_pd);
    
    // Compute checksums to prevent dead code elimination
    int64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += dst_epi32[i];
        checksum += (int64_t)(dst_ps[i] * 1000);
    }
    for (int i = 0; i < 8; i++) {
        checksum += dst_epi64[i];
        checksum += (int64_t)(dst_pd[i] * 1000);
    }
    
#ifdef __AVX512BW__
    // Test 16-bit and 8-bit blends if supported
    int16_t src1_epi16[32], src2_epi16[32], dst_epi16[32];
    int8_t src1_epi8[64], src2_epi8[64], dst_epi8[64];
    
    for (int i = 0; i < 32; i++) {
        src1_epi16[i] = rand() % 100;
        src2_epi16[i] = rand() % 100;
    }
    for (int i = 0; i < 64; i++) {
        src1_epi8[i] = rand() % 50;
        src2_epi8[i] = rand() % 50;
    }
    
    test_blend_epi16(src1_epi16, src2_epi16, dst_epi16);
    test_blend_epi8(src1_epi8, src2_epi8, dst_epi8);
    
    for (int i = 0; i < 32; i++) checksum += dst_epi16[i];
    for (int i = 0; i < 64; i++) checksum += dst_epi8[i];
#endif
    
#ifdef __AVX512FP16__
    // Test half-precision blend if supported
    _Float16 src1_ph[32], src2_ph[32], dst_ph[32];
    for (int i = 0; i < 32; i++) {
        src1_ph[i] = (_Float16)((float)rand() / RAND_MAX);
        src2_ph[i] = (_Float16)((float)rand() / RAND_MAX);
    }
    
    test_blend_ph(src1_ph, src2_ph, dst_ph);
    
    for (int i = 0; i < 32; i++) {
        checksum += (int64_t)(dst_ph[i] * 1000);
    }
#endif
    
    printf("Final checksum: %ld\n", checksum);
    printf("All blend operations completed successfully.\n");
    
    return 0;
}
