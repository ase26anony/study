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
    __m512i cmp_val = _mm512_set1_epi32(100);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    // Perform blend operation
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    // Store and compute checksum
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}

// Function to test 64-bit integer blend (E_V8DImode)
__attribute__((noinline))
uint64_t test_blend_epi64(const int64_t* src1, const int64_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi64(500);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}

// Function to test single-precision float blend (E_V16SFmode)
__attribute__((noinline))
float test_blend_ps(const float* src1, const float* src2) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create dynamic mask using floating-point comparison
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}

// Function to test double-precision float blend (E_V8DFmode)
__attribute__((noinline))
double test_blend_pd(const double* src1, const double* src2) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create dynamic mask
    __m512d cmp_val = _mm512_set1_pd(0.5);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
__attribute__((noinline))
uint32_t test_blend_epi16(const int16_t* src1, const int16_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi16(50);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    int16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}

// Function to test 8-bit integer blend (E_V64QImode)
__attribute__((noinline))
uint32_t test_blend_epi8(const int8_t* src1, const int8_t* src2) {
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi8(25);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    // Perform blend
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    int8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint32_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision float blend (E_V32HFmode)
__attribute__((noinline))
float test_blend_ph(const _Float16* src1, const _Float16* src2) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create dynamic mask
    __m512h cmp_val = _mm512_set1_ph(0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    // Perform blend
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

int main() {
    srand(42);
    
    // Initialize data arrays
    int32_t src1_epi32[16], src2_epi32[16];
    int64_t src1_epi64[8], src2_epi64[8];
    float src1_ps[16], src2_ps[16];
    double src1_pd[8], src2_pd[8];
    
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
    
    // Test AVX-512F blends
    uint64_t sum_epi32 = test_blend_epi32(src1_epi32, src2_epi32);
    uint64_t sum_epi64 = test_blend_epi64(src1_epi64, src2_epi64);
    float sum_ps = test_blend_ps(src1_ps, src2_ps);
    double sum_pd = test_blend_pd(src1_pd, src2_pd);
    
    printf("AVX-512F Blend Results:\n");
    printf("  V16SI blend checksum: %lu\n", sum_epi32);
    printf("  V8DI blend checksum: %lu\n", sum_epi64);
    printf("  V16SF blend checksum: %f\n", sum_ps);
    printf("  V8DF blend checksum: %f\n", sum_pd);
    
#ifdef __AVX512BW__
    // Initialize 16-bit and 8-bit data
    int16_t src1_epi16[32], src2_epi16[32];
    int8_t src1_epi8[64], src2_epi8[64];
    
    for (int i = 0; i < 32; i++) {
        src1_epi16[i] = rand() % 100;
        src2_epi16[i] = rand() % 100;
    }
    
    for (int i = 0; i < 64; i++) {
        src1_epi8[i] = rand() % 50;
        src2_epi8[i] = rand() % 50;
    }
    
    // Test AVX-512BW blends
    uint32_t sum_epi16 = test_blend_epi16(src1_epi16, src2_epi16);
    uint32_t sum_epi8 = test_blend_epi8(src1_epi8, src2_epi8);
    
    printf("\nAVX-512BW Blend Results:\n");
    printf("  V32HI blend checksum: %u\n", sum_epi16);
    printf("  V64QI blend checksum: %u\n", sum_epi8);
#endif
    
#ifdef __AVX512FP16__
    // Initialize half-precision data
    _Float16 src1_ph[32], src2_ph[32];
    
    for (int i = 0; i < 32; i++) {
        src1_ph[i] = (_Float16)((float)rand() / RAND_MAX);
        src2_ph[i] = (_Float16)((float)rand() / RAND_MAX);
    }
    
    // Test AVX-512FP16 blend
    float sum_ph = test_blend_ph(src1_ph, src2_ph);
    
    printf("\nAVX-512FP16 Blend Results:\n");
    printf("  V32HF blend checksum: %f\n", sum_ph);
#endif
    
    return 0;
}
