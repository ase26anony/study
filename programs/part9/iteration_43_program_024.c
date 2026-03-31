#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Function to test 32-bit integer blend (E_V16SImode)
int64_t test_blend_epi32() {
    alignas(64) int32_t src1[16];
    alignas(64) int32_t src2[16];
    alignas(64) int32_t result[16];
    
    // Initialize with non-uniform values
    for (int i = 0; i < 16; i++) {
        src1[i] = rand() % 1000;
        src2[i] = rand() % 1000;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    // Create dynamic mask using comparison
    __m512i cmp_val = _mm512_set1_epi32(500);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    // Perform blend operation
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    // Compute checksum
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

// Function to test 64-bit integer blend (E_V8DImode)
int64_t test_blend_epi64() {
    alignas(64) int64_t src1[8];
    alignas(64) int64_t src2[8];
    alignas(64) int64_t result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = rand() % 1000;
        src2[i] = rand() % 1000;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi64(500);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    // Perform blend
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

// Function to test single-precision blend (E_V16SFmode)
float test_blend_ps() {
    alignas(64) float src1[16];
    alignas(64) float src2[16];
    alignas(64) float result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(rand() % 1000) / 10.0f;
        src2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    // Create dynamic mask using floating-point comparison
    __m512 cmp_val = _mm512_set1_ps(50.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(result, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

// Function to test double-precision blend (E_V8DFmode)
double test_blend_pd() {
    alignas(64) double src1[8];
    alignas(64) double src2[8];
    alignas(64) double result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(rand() % 1000) / 10.0;
        src2[i] = (double)(rand() % 1000) / 10.0;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    // Create dynamic mask
    __m512d cmp_val = _mm512_set1_pd(50.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(result, blended);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
int32_t test_blend_epi16() {
    alignas(64) int16_t src1[32];
    alignas(64) int16_t src2[32];
    alignas(64) int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(rand() % 1000);
        src2[i] = (int16_t)(rand() % 1000);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi16(500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    // Perform blend
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    int32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

// Function to test 8-bit integer blend (E_V64QImode)
int32_t test_blend_epi8() {
    alignas(64) int8_t src1[64];
    alignas(64) int8_t src2[64];
    alignas(64) int8_t result[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(rand() % 128);
        src2[i] = (int8_t)(rand() % 128);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi8(64);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    // Perform blend
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)result, blended);
    
    int32_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision blend (E_V32HFmode)
float test_blend_ph() {
    alignas(64) _Float16 src1[32];
    alignas(64) _Float16 src2[32];
    alignas(64) _Float16 result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(rand() % 100) / 2.0f;
        src2[i] = (_Float16)(rand() % 100) / 2.0f;
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    // Create dynamic mask
    __m512h cmp_val = _mm512_set1_ph(25.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph(result, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    return sum;
}
#endif

int main() {
    srand(42);  // Seed for reproducible results
    
    printf("Testing AVX-512 blend operations...\n");
    
    // Test all blend operations
    int64_t sum_epi32 = test_blend_epi32();
    printf("32-bit integer blend checksum: %ld\n", sum_epi32);
    
    int64_t sum_epi64 = test_blend_epi64();
    printf("64-bit integer blend checksum: %ld\n", sum_epi64);
    
    float sum_ps = test_blend_ps();
    printf("Single-precision blend checksum: %f\n", sum_ps);
    
    double sum_pd = test_blend_pd();
    printf("Double-precision blend checksum: %f\n", sum_pd);
    
#ifdef __AVX512BW__
    int32_t sum_epi16 = test_blend_epi16();
    printf("16-bit integer blend checksum: %d\n", sum_epi16);
    
    int32_t sum_epi8 = test_blend_epi8();
    printf("8-bit integer blend checksum: %d\n", sum_epi8);
#else
    printf("AVX-512BW not enabled, skipping 8/16-bit blends\n");
#endif
    
#ifdef __AVX512FP16__
    float sum_ph = test_blend_ph();
    printf("Half-precision blend checksum: %f\n", sum_ph);
#else
    printf("AVX-512FP16 not enabled, skipping half-precision blends\n");
#endif
    
    printf("All blend operations completed.\n");
    return 0;
}
