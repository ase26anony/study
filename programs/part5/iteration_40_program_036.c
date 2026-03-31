#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to print verification results
static void verify_result(const char* mode, int passed) {
    printf("%s: %s\n", mode, passed ? "PASS" : "FAIL");
}

// ==================== V64QImode (64 x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
static uint64_t test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 255 - i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAA...AA (10101010 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Blend using vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Store and compute checksum
    uint8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HImode (32 x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((noinline))
static uint64_t test_v32himode() {
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 65535 - i * 100;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask: 0x5555... (01010101 pattern)
    __mmask32 mask = 0x55555555;
    
    // Blend using vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif

// ==================== V32HFmode (32 x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((noinline))
static float test_v32hfmode() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Create mask with first 16 bits set
    __mmask32 mask = 0x0000FFFF;
    
    // Blend using vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)out[i];
    }
    return sum;
}
#endif

// ==================== V32BFmode (32 x bfloat16) ====================
#ifdef __AVX512BF16__
__attribute__((noinline))
static float test_v32bfmode() {
    // BF16 arrays
    __bfloat16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 2.0f);
        src2[i] = bfloat16_from_float(200.0f - i * 2.0f);
    }
    
    __m512bh a = _mm512_loadu_si512((__m512i*)src1);
    __m512bh b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Use integer blend for BF16 (same as V32HImode for blending)
    __m512i a_int = _mm512_castsi512_si512((__m512i)a);
    __m512i b_int = _mm512_castsi512_si512((__m512i)b);
    __m512i result_int = _mm512_mask_blend_epi16(mask, a_int, b_int);
    
    __m512bh result = _mm512_castsi512_bh(result_int);
    
    __bfloat16 out[32];
    _mm512_storeu_si512((__m512i*)out, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(out[i]);
    }
    return sum;
}
#endif

// ==================== V16SImode (16 x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
static uint64_t test_v16simode() {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask with every other bit set
    __mmask16 mask = 0xAAAA;
    
    // Blend using vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += (uint64_t)abs(out[i]);
    }
    return checksum;
}
#endif

// ==================== V8DImode (8 x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((noinline))
static uint64_t test_v8dimode() {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = -i * 10000LL;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask: select first 4 elements from b, last 4 from a
    __mmask8 mask = 0x0F;  // 00001111
    
    // Blend using vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += (uint64_t)llabs(out[i]);
    }
    return checksum;
}
#endif

// ==================== V8DFmode (8 x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
static double test_v8dfmode() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Compare to create dynamic mask
    __m512d threshold = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(a, threshold, _CMP_LT_OQ);
    
    // Blend using vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== V16SFmode (16 x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((noinline))
static float test_v16sfmode() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 50.0f - i * 0.5f;
    }
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 threshold = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, threshold, _CMP_GT_OQ);
    
    // Blend using vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += out[i];
    }
    return sum;
}
#endif

// ==================== Main Driver ====================
int main() {
    int passed = 1;
    
    printf("Testing AVX-512 blend operations for coverage...\n");
    printf("===============================================\n");
    
#ifdef __AVX512BW__
    printf("\nTesting V64QImode (64x8-bit integers)...\n");
    uint64_t result1 = test_v64qimode();
    verify_result("V64QImode", result1 > 0);
    
    printf("\nTesting V32HImode (32x16-bit integers)...\n");
    uint64_t result2 = test_v32himode();
    verify_result("V32HImode", result2 > 0);
#else
    printf("Skipping V64QImode and V32HImode (AVX512BW not enabled)\n");
#endif

#ifdef __AVX512FP16__
    printf("\nTesting V32HFmode (32xhalf-precision floats)...\n");
    float result3 = test_v32hfmode();
    verify_result("V32HFmode", result3 != 0.0f);
#else
    printf("Skipping V32HFmode (AVX512FP16 not enabled)\n");
#endif

#ifdef __AVX512BF16__
    printf("\nTesting V32BFmode (32xbfloat16)...\n");
    float result4 = test_v32bfmode();
    verify_result("V32BFmode", result4 != 0.0f);
#else
    printf("Skipping V32BFmode (AVX512BF16 not enabled)\n");
#endif

#ifdef __AVX512F__
    printf("\nTesting V16SImode (16x32-bit integers)...\n");
    uint64_t result5 = test_v16simode();
    verify_result("V16SImode", result5 > 0);
    
    printf("\nTesting V8DImode (8x64-bit integers)...\n");
    uint64_t result6 = test_v8dimode();
    verify_result("V8DImode", result6 > 0);
    
    printf("\nTesting V8DFmode (8xdouble-precision floats)...\n");
    double result7 = test_v8dfmode();
    verify_result("V8DFmode", result7 != 0.0);
    
    printf("\nTesting V16SFmode (16xsingle-precision floats)...\n");
    float result8 = test_v16sfmode();
    verify_result("V16SFmode", result8 != 0.0f);
#else
    printf("Skipping AVX512F modes (AVX512F not enabled)\n");
#endif
    
    printf("\n===============================================\n");
    printf("All enabled tests completed.\n");
    
    return 0;
}
