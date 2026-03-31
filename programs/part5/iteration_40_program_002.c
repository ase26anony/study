#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to prevent optimization
static void escape(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
void test_v64qimode() {
    // Initialize arrays
    uint8_t src1[64], src2[64], dst[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 255 - i;
    }
    
    // Load into vectors
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAA... (10101010 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Blend based on mask
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and use result
    _mm512_storeu_si512((__m512i*)dst, result);
    escape(dst);
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    printf("V64QImode blend sum: %d\n", sum);
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
void test_v32himode() {
    uint16_t src1[32], src2[32], dst[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 30000 - i * 100;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask
    __mmask32 mask = 0xAAAAAAAA;
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
    escape(dst);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    printf("V32HImode blend sum: %d\n", sum);
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
void test_v32hfmode() {
    _Float16 src1[32], src2[32], dst[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 1.5f;
        src2[i] = 100.0f - i * 1.5f;
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Compare to create dynamic mask
    __m512h zero = _mm512_setzero_ph();
    __mmask32 mask = _mm512_cmp_ph_mask(v1, zero, _CMP_GT_OQ);
    
    // Blend using mask
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_ph(dst, result);
    escape(dst);
    
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    printf("V32HFmode blend sum: %.2f\n", sum);
}
#endif

// ==================== V32BFmode (32x bfloat16) ====================
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
void test_v32bfmode() {
    // Note: Direct BF16 blend intrinsics might not exist, but compiler
    // will generate blends when operating on BF16 vectors
    __m512bh src1, src2, result;
    uint16_t data1[32], data2[32], dst[32];
    
    for (int i = 0; i < 32; i++) {
        data1[i] = i << 8;  // Simple bfloat16 pattern
        data2[i] = (31 - i) << 8;
    }
    
    src1 = _mm512_loadu_si512((__m512i*)data1);
    src2 = _mm512_loadu_si512((__m512i*)data2);
    
    // Create mask
    __mmask32 mask = 0x55555555;  // 01010101 pattern
    
    // Use integer blend on BF16 data (same as V32HImode for storage)
    __m512i int_result = _mm512_mask_blend_epi16(mask, 
        (__m512i)src1, (__m512i)src2);
    
    _mm512_storeu_si512((__m512i*)dst, int_result);
    escape(dst);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    printf("V32BFmode blend sum: %d\n", sum);
}
#endif
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
void test_v16simode() {
    int32_t src1[16], src2[16], dst[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = 50000 - i * 1000;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask from comparison
    __m512i threshold = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, threshold);
    
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
    escape(dst);
    
    long long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    printf("V16SImode blend sum: %lld\n", sum);
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
void test_v8dimode() {
    int64_t src1[8], src2[8], dst[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = 1000000LL - i * 10000LL;
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Alternating mask
    __mmask8 mask = 0xAA;  // 10101010
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst, result);
    escape(dst);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    printf("V8DImode blend sum: %lld\n", sum);
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
void test_v8dfmode() {
    double src1[8], src2[8], dst[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = 10.0 - i * 1.1;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask from comparison
    __m512d threshold = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_GT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    _mm512_storeu_pd(dst, result);
    escape(dst);
    
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    printf("V8DFmode blend sum: %.2f\n", sum);
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
void test_v16sfmode() {
    float src1[16], src2[16], dst[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 8.0f - i * 0.5f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create complex mask pattern
    __m512 threshold = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, threshold, _CMP_LT_OQ);
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    _mm512_storeu_ps(dst, result);
    escape(dst);
    
    float sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    printf("V16SFmode blend sum: %.2f\n", sum);
}
#endif

// ==================== Main driver ====================
int main() {
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512F__
    test_v16sfmode();
    test_v8dfmode();
    test_v16simode();
    test_v8dimode();
#endif

#ifdef __AVX512BW__
    test_v64qimode();
    test_v32himode();
#endif

#ifdef __AVX512FP16__
    test_v32hfmode();
#endif

#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
    test_v32bfmode();
#endif
#endif
    
    printf("All blend tests completed.\n");
    return 0;
}
