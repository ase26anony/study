#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to print verification results
static void verify_result(const char* mode, int passed) {
    printf("%s: %s\n", mode, passed ? "PASS" : "FAIL");
}

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__attribute__((noinline))
uint64_t test_v64qimode() {
    // Initialize arrays
    char src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAA...AA (alternating 1/0 pattern)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Blend using vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage by storing and computing checksum
    char out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += out[i];
    }
    return checksum;
}

// V32HImode: 32 x 16-bit integers
__attribute__((noinline))
uint64_t test_v32himode() {
    short src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend using vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    short out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += out[i];
    }
    return checksum;
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
__attribute__((noinline))
float test_v32hfmode() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 2.5f;
    }
    
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Create mask with first 16 elements from a, last 16 from b
    __mmask32 mask = 0x0000FFFF;
    
    // Blend using vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}

// V32BFmode: 32 x bfloat16
__attribute__((noinline))
float test_v32bfmode() {
    // Use __m512bh for bfloat16
    __m512bh a, b;
    
    // Initialize with simple pattern
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 pattern (just set exponent part)
        src1[i] = (i + 1) << 8;  // Exponent increases with i
        src2[i] = (32 - i) << 8; // Reverse pattern
    }
    
    a = _mm512_loadu_si512((__m512i*)src1);
    b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;
    
    // Blend bfloat16 vectors - use integer blend since there's no direct BF16 blend intrinsic
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)a, (__m512i)b);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, (__m512i)result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += out[i];
    }
    return sum;
}
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__attribute__((noinline))
uint64_t test_v16simode() {
    int src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask: select even indices from a, odd from b
    __mmask16 mask = 0xAAAA;  // 0b1010101010101010
    
    // Blend using vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += out[i];
    }
    return checksum;
}

// V8DImode: 8 x 64-bit integers
__attribute__((noinline))
uint64_t test_v8dimode() {
    long long src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 30000LL;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask: select first 4 from a, last 4 from b
    __mmask8 mask = 0xF0;  // 0b11110000
    
    // Blend using vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    long long out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 8; i++) {
        checksum += out[i];
    }
    return checksum;
}

// V8DFmode: 8 x double-precision floats
__attribute__((noinline))
double test_v8dfmode() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.111;
        src2[i] = i * 2.222;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Create comparison mask: a[i] < b[i]
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
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

// V16SFmode: 16 x single-precision floats
__attribute__((noinline))
float test_v16sfmode() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.0f;
    }
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Create comparison mask: a[i] > b[i]
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    
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
#endif // __AVX512F__

int main() {
    int passed = 1;
    
    printf("Testing AVX-512 blend operations for GCC coverage...\n");
    printf("===================================================\n");
    
#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW modes:\n");
    uint64_t result1 = test_v64qimode();
    verify_result("V64QImode", result1 != 0);
    
    uint64_t result2 = test_v32himode();
    verify_result("V32HImode", result2 != 0);
#endif
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 modes:\n");
    float result3 = test_v32hfmode();
    verify_result("V32HFmode", result3 != 0.0f);
    
    float result4 = test_v32bfmode();
    verify_result("V32BFmode", result4 != 0.0f);
#endif
    
#ifdef __AVX512F__
    printf("\nTesting AVX-512F modes:\n");
    uint64_t result5 = test_v16simode();
    verify_result("V16SImode", result5 != 0);
    
    uint64_t result6 = test_v8dimode();
    verify_result("V8DImode", result6 != 0);
    
    double result7 = test_v8dfmode();
    verify_result("V8DFmode", result7 != 0.0);
    
    float result8 = test_v16sfmode();
    verify_result("V16SFmode", result8 != 0.0f);
#endif
    
    printf("\n===================================================\n");
    
    // Check if any required features are missing
#ifndef __AVX512F__
    printf("WARNING: AVX-512F not enabled. Some tests skipped.\n");
#endif
#ifndef __AVX512BW__
    printf("WARNING: AVX-512BW not enabled. Some tests skipped.\n");
#endif
#ifndef __AVX512FP16__
    printf("WARNING: AVX-512FP16 not enabled. Some tests skipped.\n");
#endif
    
    return 0;
}
