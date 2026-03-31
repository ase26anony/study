#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Function to test 32-bit integer blend (E_V16SImode)
__attribute__((noinline))
uint64_t test_blend_epi32(const int32_t* src1, const int32_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask using comparison
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, v2);
    
    // Blend based on comparison result
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    // Store and compute checksum
    int32_t res_arr[16];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)res_arr[i];
    }
    return sum;
}

// Function to test 64-bit integer blend (E_V8DImode)
__attribute__((noinline))
uint64_t test_blend_epi64(const int64_t* src1, const int64_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, v2);
    
    // Blend operation
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t res_arr[8];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)res_arr[i];
    }
    return sum;
}

// Function to test single-precision blend (E_V16SFmode)
__attribute__((noinline))
float test_blend_ps(const float* src1, const float* src2) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask using floating-point comparison
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    // Blend operation
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float res_arr[16];
    _mm512_storeu_ps(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    return sum;
}

// Function to test double-precision blend (E_V8DFmode)
__attribute__((noinline))
double test_blend_pd(const double* src1, const double* src2) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_GT_OQ);
    
    // Blend operation
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double res_arr[8];
    _mm512_storeu_pd(res_arr, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res_arr[i];
    }
    return sum;
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
__attribute__((noinline))
uint64_t test_blend_epi16(const int16_t* src1, const int16_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, v2);
    
    // Blend operation
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    int16_t res_arr[32];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint64_t)res_arr[i];
    }
    return sum;
}

// Function to test 8-bit integer blend (E_V64QImode)
__attribute__((noinline))
uint64_t test_blend_epi8(const int8_t* src1, const int8_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, v2);
    
    // Blend operation
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    int8_t res_arr[64];
    _mm512_storeu_si512((__m512i*)res_arr, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint64_t)res_arr[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision blend (E_V32HFmode)
__attribute__((noinline))
float test_blend_ph(const _Float16* src1, const _Float16* src2) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_GT_OQ);
    
    // Blend operation
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 res_arr[32];
    _mm512_storeu_ph(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res_arr[i];
    }
    return sum;
}
#endif

int main() {
    srand(42);
    
    // Initialize data arrays with random values
    int32_t src1_32[16], src2_32[16];
    int64_t src1_64[8], src2_64[8];
    float src1_f[16], src2_f[16];
    double src1_d[8], src2_d[8];
    
    for (int i = 0; i < 16; i++) {
        src1_32[i] = rand() % 1000;
        src2_32[i] = rand() % 1000;
        src1_f[i] = (float)(rand() % 1000) / 10.0f;
        src2_f[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_64[i] = rand() % 1000;
        src2_64[i] = rand() % 1000;
        src1_d[i] = (double)(rand() % 1000) / 10.0;
        src2_d[i] = (double)(rand() % 1000) / 10.0;
    }
    
    // Test all blend operations
    uint64_t sum32 = test_blend_epi32(src1_32, src2_32);
    uint64_t sum64 = test_blend_epi64(src1_64, src2_64);
    float sumf = test_blend_ps(src1_f, src2_f);
    double sumd = test_blend_pd(src1_d, src2_d);
    
    printf("32-bit integer blend checksum: %lu\n", sum32);
    printf("64-bit integer blend checksum: %lu\n", sum64);
    printf("Single-precision blend checksum: %f\n", sumf);
    printf("Double-precision blend checksum: %f\n", sumd);
    
#ifdef __AVX512BW__
    // Initialize 16-bit and 8-bit data
    int16_t src1_16[32], src2_16[32];
    int8_t src1_8[64], src2_8[64];
    
    for (int i = 0; i < 32; i++) {
        src1_16[i] = rand() % 1000;
        src2_16[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 64; i++) {
        src1_8[i] = rand() % 256;
        src2_8[i] = rand() % 256;
    }
    
    uint64_t sum16 = test_blend_epi16(src1_16, src2_16);
    uint64_t sum8 = test_blend_epi8(src1_8, src2_8);
    
    printf("16-bit integer blend checksum: %lu\n", sum16);
    printf("8-bit integer blend checksum: %lu\n", sum8);
#endif
    
#ifdef __AVX512FP16__
    // Initialize half-precision data
    _Float16 src1_h[32], src2_h[32];
    
    for (int i = 0; i < 32; i++) {
        src1_h[i] = (_Float16)(rand() % 1000) / 10.0f;
        src2_h[i] = (_Float16)(rand() % 1000) / 10.0f;
    }
    
    float sumh = test_blend_ph(src1_h, src2_h);
    printf("Half-precision blend checksum: %f\n", sumh);
#endif
    
    printf("All blend operations completed successfully!\n");
    return 0;
}
