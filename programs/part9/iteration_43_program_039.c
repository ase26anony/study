#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __AVX512F__
// 32-bit integer blend (E_V16SImode)
__attribute__((noinline))
int64_t test_blend_epi32(int32_t* src1, int32_t* src2, int32_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Dynamic mask: compare v1 > v2 (non-constant)
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, v2);
    
    // Blend based on comparison result
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, blended);
    
    // Compute checksum to prevent optimization
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

// 64-bit integer blend (E_V8DImode)
__attribute__((noinline))
int64_t test_blend_epi64(int64_t* src1, int64_t* src2, int64_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Dynamic mask: compare v1 != v2
    __mmask8 mask = _mm512_cmpneq_epi64_mask(v1, v2);
    
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, blended);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}

// Single-precision float blend (E_V16SFmode)
__attribute__((noinline))
float test_blend_ps(float* src1, float* src2, float* dst) {
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Dynamic mask: compare v1 > v2
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    _mm512_storeu_ps(dst, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    return sum;
}

// Double-precision float blend (E_V8DFmode)
__attribute__((noinline))
double test_blend_pd(double* src1, double* src2, double* dst) {
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Dynamic mask: compare v1 < v2
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_storeu_pd(dst, blended);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

#ifdef __AVX512BW__
// 8-bit integer blend (E_V64QImode)
__attribute__((noinline))
int64_t test_blend_epi8(int8_t* src1, int8_t* src2, int8_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Dynamic mask: compare v1 > v2
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, v2);
    
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, blended);
    
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    return sum;
}

// 16-bit integer blend (E_V32HImode)
__attribute__((noinline))
int64_t test_blend_epi16(int16_t* src1, int16_t* src2, int16_t* dst) {
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Dynamic mask: compare v1 != v2
    __mmask32 mask = _mm512_cmpneq_epi16_mask(v1, v2);
    
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)dst, blended);
    
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    return sum;
}
#endif

#ifdef __AVX512FP16__
// 16-bit float blend (E_V32HFmode)
__attribute__((noinline))
float test_blend_ph(_Float16* src1, _Float16* src2, _Float16* dst) {
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Dynamic mask: compare v1 > v2
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_GT_OQ);
    
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_storeu_ph(dst, blended);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst[i];
    }
    return sum;
}
#endif

int main() {
    srand(42);
    
    // Initialize data arrays with random values
    int32_t src1_epi32[16], src2_epi32[16], dst_epi32[16];
    int64_t src1_epi64[8], src2_epi64[8], dst_epi64[8];
    float src1_ps[16], src2_ps[16], dst_ps[16];
    double src1_pd[8], src2_pd[8], dst_pd[8];
    
    for (int i = 0; i < 16; i++) {
        src1_epi32[i] = rand() % 100;
        src2_epi32[i] = rand() % 100;
        src1_ps[i] = (float)(rand() % 100) / 10.0f;
        src2_ps[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_epi64[i] = rand() % 100;
        src2_epi64[i] = rand() % 100;
        src1_pd[i] = (double)(rand() % 100) / 10.0;
        src2_pd[i] = (double)(rand() % 100) / 10.0;
    }
    
    int64_t checksum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blends...\n");
    checksum += test_blend_epi32(src1_epi32, src2_epi32, dst_epi32);
    checksum += test_blend_epi64(src1_epi64, src2_epi64, dst_epi64);
    checksum += (int64_t)test_blend_ps(src1_ps, src2_ps, dst_ps);
    checksum += (int64_t)test_blend_pd(src1_pd, src2_pd, dst_pd);
#endif
    
#ifdef __AVX512BW__
    printf("Testing AVX-512BW blends...\n");
    int8_t src1_epi8[64], src2_epi8[64], dst_epi8[64];
    int16_t src1_epi16[32], src2_epi16[32], dst_epi16[32];
    
    for (int i = 0; i < 64; i++) {
        src1_epi8[i] = rand() % 100;
        src2_epi8[i] = rand() % 100;
    }
    for (int i = 0; i < 32; i++) {
        src1_epi16[i] = rand() % 100;
        src2_epi16[i] = rand() % 100;
    }
    
    checksum += test_blend_epi8(src1_epi8, src2_epi8, dst_epi8);
    checksum += test_blend_epi16(src1_epi16, src2_epi16, dst_epi16);
#endif
    
#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blends...\n");
    _Float16 src1_ph[32], src2_ph[32], dst_ph[32];
    
    for (int i = 0; i < 32; i++) {
        src1_ph[i] = (_Float16)(rand() % 100) / 10.0f;
        src2_ph[i] = (_Float16)(rand() % 100) / 10.0f;
    }
    
    checksum += (int64_t)test_blend_ph(src1_ph, src2_ph, dst_ph);
#endif
    
    printf("Final checksum: %ld\n", checksum);
    printf("Blend operations completed successfully.\n");
    
    return 0;
}
