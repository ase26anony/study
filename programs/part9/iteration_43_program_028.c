#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Function to test 32-bit integer blend (E_V16SImode)
__attribute__((noinline))
uint64_t test_blend_epi32(const int32_t* src1, const int32_t* src2, int32_t* dst) {
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask using comparison
    __m512i cmp_val = _mm512_set1_epi32(100);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
    
    // Blend based on dynamic mask
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    // Compute checksum to prevent optimization
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

// Function to test 64-bit integer blend (E_V8DImode)
__attribute__((noinline))
uint64_t test_blend_epi64(const int64_t* src1, const int64_t* src2, int64_t* dst) {
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi64(500);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(a, cmp_val);
    
    // Blend 64-bit integers
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

// Function to test single-precision blend (E_V16SFmode)
__attribute__((noinline))
float test_blend_ps(const float* src1, const float* src2, float* dst) {
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Create dynamic mask using floating comparison
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    // Blend single-precision floats
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    _mm512_storeu_ps(dst, result);
    
    // Compute checksum
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

// Function to test double-precision blend (E_V8DFmode)
__attribute__((noinline))
double test_blend_pd(const double* src1, const double* src2, double* dst) {
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Create dynamic mask
    __m512d cmp_val = _mm512_set1_pd(0.5);
    __mmask8 mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
    
    // Blend double-precision floats
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    _mm512_storeu_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

#ifdef __AVX512BW__
// Function to test 16-bit integer blend (E_V32HImode)
__attribute__((noinline))
uint32_t test_blend_epi16(const int16_t* src1, const int16_t* src2, int16_t* dst) {
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi16(50);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(a, cmp_val);
    
    // Blend 16-bit integers (requires AVX512BW)
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}

// Function to test 8-bit integer blend (E_V64QImode)
__attribute__((noinline))
uint16_t test_blend_epi8(const int8_t* src1, const int8_t* src2, int8_t* dst) {
    __m512i a = _mm512_loadu_si512((const __m512i*)src1);
    __m512i b = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask
    __m512i cmp_val = _mm512_set1_epi8(25);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(a, cmp_val);
    
    // Blend 8-bit integers (requires AVX512BW)
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    uint16_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// Function to test half-precision blend (E_V32HFmode)
__attribute__((noinline))
float test_blend_ph(const _Float16* src1, const _Float16* src2, _Float16* dst) {
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Create dynamic mask
    __m512h cmp_val = _mm512_set1_ph(0.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
    
    // Blend half-precision floats (requires AVX512FP16)
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _mm512_storeu_ph(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

int main() {
    srand(42);
    
    // Initialize data arrays
    int32_t src1_32[16], src2_32[16], dst_32[16];
    int64_t src1_64[8], src2_64[8], dst_64[8];
    float src1_f32[16], src2_f32[16], dst_f32[16];
    double src1_f64[8], src2_f64[8], dst_f64[8];
    
    // Fill arrays with random values
    for (int i = 0; i < 16; i++) {
        src1_32[i] = rand() % 200;
        src2_32[i] = rand() % 200;
        src1_f32[i] = (float)rand() / RAND_MAX;
        src2_f32[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_64[i] = rand() % 1000;
        src2_64[i] = rand() % 1000;
        src1_f64[i] = (double)rand() / RAND_MAX;
        src2_f64[i] = (double)rand() / RAND_MAX;
    }
    
    // Test all blend operations
    uint64_t sum32 = test_blend_epi32(src1_32, src2_32, dst_32);
    uint64_t sum64 = test_blend_epi64(src1_64, src2_64, dst_64);
    float sumf32 = test_blend_ps(src1_f32, src2_f32, dst_f32);
    double sumf64 = test_blend_pd(src1_f64, src2_f64, dst_f64);
    
    printf("32-bit integer blend checksum: %lu\n", sum32);
    printf("64-bit integer blend checksum: %lu\n", sum64);
    printf("32-bit float blend checksum: %f\n", sumf32);
    printf("64-bit float blend checksum: %f\n", sumf64);
    
#ifdef __AVX512BW__
    // Test 16-bit and 8-bit blends if AVX512BW is available
    int16_t src1_16[32], src2_16[32], dst_16[32];
    int8_t src1_8[64], src2_8[64], dst_8[64];
    
    for (int i = 0; i < 32; i++) {
        src1_16[i] = rand() % 100;
        src2_16[i] = rand() % 100;
    }
    
    for (int i = 0; i < 64; i++) {
        src1_8[i] = rand() % 50;
        src2_8[i] = rand() % 50;
    }
    
    uint32_t sum16 = test_blend_epi16(src1_16, src2_16, dst_16);
    uint16_t sum8 = test_blend_epi8(src1_8, src2_8, dst_8);
    
    printf("16-bit integer blend checksum: %u\n", sum16);
    printf("8-bit integer blend checksum: %u\n", sum8);
#endif
    
#ifdef __AVX512FP16__
    // Test half-precision blend if AVX512FP16 is available
    _Float16 src1_f16[32], src2_f16[32], dst_f16[32];
    
    for (int i = 0; i < 32; i++) {
        src1_f16[i] = (_Float16)((float)rand() / RAND_MAX);
        src2_f16[i] = (_Float16)((float)rand() / RAND_MAX);
    }
    
    float sumf16 = test_blend_ph(src1_f16, src2_f16, dst_f16);
    printf("16-bit float blend checksum: %f\n", sumf16);
#endif
    
    printf("All blend operations completed successfully!\n");
    return 0;
}
