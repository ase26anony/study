#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Helper function to print array (for debugging)
void print_array_epi32(const char* label, const int32_t* arr, int size) {
    printf("%s: ", label);
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

// Test blend for 32-bit integers (E_V16SImode)
int64_t test_blend_epi32() {
    const int N = 16;
    alignas(64) int32_t src1[N], src2[N], dst[N];
    
    // Fill with non-uniform values
    for (int i = 0; i < N; i++) {
        src1[i] = rand() % 100 - 50;
        src2[i] = rand() % 100 - 50;
    }
    
    // Load vectors
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create dynamic mask using comparison
    // Compare v1 > v2 element-wise
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_GT);
    
    // Perform blend operation
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    // Store result
    _mm512_storeu_si512((__m512i*)dst, result);
    
    // Compute checksum
    int64_t sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

// Test blend for 64-bit integers (E_V8DImode)
int64_t test_blend_epi64() {
    const int N = 8;
    alignas(64) int64_t src1[N], src2[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src1[i] = rand() % 200 - 100;
        src2[i] = rand() % 200 - 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create mask using comparison with a threshold
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, _mm512_set1_epi64(0), _MM_CMPINT_GT);
    
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

// Test blend for single-precision floats (E_V16SFmode)
float test_blend_ps() {
    const int N = 16;
    alignas(64) float src1[N], src2[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src1[i] = (rand() % 2000 - 1000) / 10.0f;
        src2[i] = (rand() % 2000 - 1000) / 10.0f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Compare v1 != v2 to create non-trivial mask
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_NEQ_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    _mm512_storeu_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

// Test blend for double-precision floats (E_V8DFmode)
double test_blend_pd() {
    const int N = 8;
    alignas(64) double src1[N], src2[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src1[i] = (rand() % 2000 - 1000) / 10.0;
        src2[i] = (rand() % 2000 - 1000) / 10.0;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Compare v1 < v2
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_storeu_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

#ifdef __AVX512BW__
// Test blend for 16-bit integers (E_V32HImode)
int64_t test_blend_epi16() {
    const int N = 32;
    alignas(64) int16_t src1[N], src2[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src1[i] = rand() % 1000 - 500;
        src2[i] = rand() % 1000 - 500;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create mask using comparison
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_GT);
    
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

// Test blend for 8-bit integers (E_V64QImode)
int64_t test_blend_epi8() {
    const int N = 64;
    alignas(64) int8_t src1[N], src2[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src1[i] = rand() % 200 - 100;
        src2[i] = rand() % 200 - 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Create mask using comparison with zero
    __mmask64 mask = _mm512_cmp_epi8_mask(v1, _mm512_setzero_si512(), _MM_CMPINT_GT);
    
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// Test blend for half-precision floats (E_V32HFmode)
float test_blend_ph() {
    const int N = 32;
    alignas(64) _Float16 src1[N], src2[N], dst[N];
    
    for (int i = 0; i < N; i++) {
        src1[i] = (rand() % 2000 - 1000) / 10.0f;
        src2[i] = (rand() % 2000 - 1000) / 10.0f;
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask using comparison
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_NEQ_OQ);
    
    __m512h result = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_storeu_ph(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

int main() {
    srand(42); // Seed for reproducible results
    
    printf("Testing AVX-512 blend operations...\n");
    
    // Test all blend operations
    int64_t sum_epi32 = test_blend_epi32();
    printf("Blend epi32 checksum: %ld\n", sum_epi32);
    
    int64_t sum_epi64 = test_blend_epi64();
    printf("Blend epi64 checksum: %ld\n", sum_epi64);
    
    float sum_ps = test_blend_ps();
    printf("Blend ps checksum: %f\n", sum_ps);
    
    double sum_pd = test_blend_pd();
    printf("Blend pd checksum: %f\n", sum_pd);
    
#ifdef __AVX512BW__
    int64_t sum_epi16 = test_blend_epi16();
    printf("Blend epi16 checksum: %ld\n", sum_epi16);
    
    int64_t sum_epi8 = test_blend_epi8();
    printf("Blend epi8 checksum: %ld\n", sum_epi8);
#endif
    
#ifdef __AVX512FP16__
    float sum_ph = test_blend_ph();
    printf("Blend ph checksum: %f\n", sum_ph);
#endif
    
    printf("All blend tests completed.\n");
    
    return 0;
}
