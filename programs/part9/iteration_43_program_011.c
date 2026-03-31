#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __AVX512F__
// 32-bit integer blend (E_V16SImode)
uint64_t test_blend_epi32() {
    alignas(64) int32_t src1[16], src2[16], result[16];
    
    // Fill with non-uniform data
    for (int i = 0; i < 16; i++) {
        src1[i] = rand() % 1000;
        src2[i] = rand() % 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create data-dependent mask using comparison
    __m512i cmp_val = _mm512_set1_epi32(500);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    // Perform blend operation
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    // Compute checksum to prevent optimization
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

// 64-bit integer blend (E_V8DImode)
uint64_t test_blend_epi64() {
    alignas(64) int64_t src1[8], src2[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = rand() % 1000;
        src2[i] = rand() % 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi64(500);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}

// Single-precision float blend (E_V16SFmode)
float test_blend_ps() {
    alignas(64) float src1[16], src2[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(rand() % 1000) / 10.0f;
        src2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    __m512 cmp_val = _mm512_set1_ps(50.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(result, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    return sum;
}

// Double-precision float blend (E_V8DFmode)
double test_blend_pd() {
    alignas(64) double src1[8], src2[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(rand() % 1000) / 10.0;
        src2[i] = (double)(rand() % 1000) / 10.0;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __m512d cmp_val = _mm512_set1_pd(50.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_storeu_pd(result, blended);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    return sum;
}
#endif

#ifdef __AVX512BW__
// 16-bit integer blend (E_V32HImode)
uint64_t test_blend_epi16() {
    alignas(64) int16_t src1[32], src2[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = rand() % 1000;
        src2[i] = rand() % 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi16(500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}

// 8-bit integer blend (E_V64QImode)
uint64_t test_blend_epi8() {
    alignas(64) int8_t src1[64], src2[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = rand() % 256;
        src2[i] = rand() % 256;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    __m512i cmp_val = _mm512_set1_epi8(100);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// Half-precision float blend (E_V32HFmode)
float test_blend_ph() {
    alignas(64) _Float16 src1[32], src2[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(rand() % 1000) / 10.0f;
        src2[i] = (_Float16)(rand() % 1000) / 10.0f;
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    __m512h cmp_val = _mm512_set1_ph(50.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(result, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    return sum;
}
#endif

int main() {
    srand(42);  // Seed for reproducible results
    
    uint64_t total_checksum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    total_checksum += test_blend_epi32();
    total_checksum += test_blend_epi64();
    total_checksum += (uint64_t)test_blend_ps();
    total_checksum += (uint64_t)test_blend_pd();
#endif

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    total_checksum += test_blend_epi16();
    total_checksum += test_blend_epi8();
#endif

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    total_checksum += (uint64_t)test_blend_ph();
#endif
    
    printf("Final checksum: %lu\n", total_checksum);
    printf("All blend operations completed successfully.\n");
    
    return 0;
}
