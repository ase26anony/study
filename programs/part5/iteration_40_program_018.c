#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to print verification results
static void verify_result(const char* mode, int passed) {
    printf("%-12s: %s\n", mode, passed ? "PASS" : "FAIL");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static int test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64], expected[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
        // Mask selects src1 for even indices, src2 for odd
        expected[i] = (i % 2 == 0) ? src1[i] : src2[i];
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: 0xAAAAAAAAAAAAAAAA (alternating bits)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Verify result
    uint8_t out[64];
    _mm512_storeu_si512((__m512i*)out, result);
    
    for (int i = 0; i < 64; i++) {
        if (out[i] != expected[i]) return 0;
    }
    return 1;
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static int test_v32himode() {
    uint16_t src1[32], src2[32], expected[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 65535 - i * 100;
        // Mask selects src1 for first half, src2 for second half
        expected[i] = (i < 16) ? src1[i] : src2[i];
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Mask: lower 16 bits = 1, upper 16 bits = 0
    __mmask32 mask = 0x0000FFFF;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, result);
    
    for (int i = 0; i < 32; i++) {
        if (out[i] != expected[i]) return 0;
    }
    return 1;
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
__attribute__((target("avx512fp16")))
static int test_v32hfmode() {
    _Float16 src1[32], src2[32], expected[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
        // Mask selects based on comparison
        expected[i] = (i % 3 == 0) ? src1[i] : src2[i];
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask using comparison
    __m512h threshold = _mm512_set1_ph((_Float16)24.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, threshold, _CMP_LT_OQ);
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    _Float16 out[32];
    _mm512_storeu_ph(out, result);
    
    // Verify by recomputing expected based on mask
    for (int i = 0; i < 32; i++) {
        int bit = (mask >> i) & 1;
        _Float16 exp = bit ? src1[i] : src2[i];
        if (out[i] != exp) return 0;
    }
    return 1;
}
#endif

// ==================== V32BFmode (32x brain floats) ====================
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
static int test_v32bfmode() {
    // BF16 is stored as 16-bit integers
    uint16_t src1[32], src2[32], expected[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 0x0400;  // Roughly i * 1.0 in BF16
        src2[i] = 0x3F80 - i * 0x0400;  // Roughly 1.0 - i
        expected[i] = (i % 4 == 0) ? src1[i] : src2[i];
    }
    
    __m512bh v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512bh v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;
    
    // Use generic blend intrinsic - compiler should lower to appropriate instruction
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)v1, (__m512i)v2);
    
    uint16_t out[32];
    _mm512_storeu_si512((__m512i*)out, (__m512i)result);
    
    for (int i = 0; i < 32; i++) {
        if (out[i] != expected[i]) return 0;
    }
    return 1;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static int test_v16simode() {
    int32_t src1[16], src2[16], expected[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
        expected[i] = (src1[i] > 0) ? src1[i] : src2[i];
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask using comparison
    __m512i zero = _mm512_setzero_si512();
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, zero);
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t out[16];
    _mm512_storeu_si512((__m512i*)out, result);
    
    for (int i = 0; i < 16; i++) {
        int bit = (mask >> i) & 1;
        int32_t exp = bit ? src1[i] : src2[i];
        if (out[i] != exp) return 0;
    }
    return 1;
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static int test_v8dimode() {
    int64_t src1[8], src2[8], expected[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 1LL << (i * 8);
        src2[i] = ~src1[i];
        expected[i] = (i % 2 == 0) ? src1[i] : src2[i];
    }
    
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Alternating mask
    __mmask8 mask = 0x55;  // 01010101
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t out[8];
    _mm512_storeu_si512((__m512i*)out, result);
    
    for (int i = 0; i < 8; i++) {
        if (out[i] != expected[i]) return 0;
    }
    return 1;
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static int test_v8dfmode() {
    double src1[8], src2[8], expected[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
        expected[i] = (src1[i] < 6.0) ? src1[i] : src2[i];
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d threshold = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, threshold, _CMP_LT_OQ);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double out[8];
    _mm512_storeu_pd(out, result);
    
    for (int i = 0; i < 8; i++) {
        int bit = (mask >> i) & 1;
        double exp = bit ? src1[i] : src2[i];
        if (out[i] != exp) return 0;
    }
    return 1;
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static int test_v16sfmode() {
    float src1[16], src2[16], expected[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 10.0f - i * 0.5f;
        expected[i] = (i < 8) ? src1[i] : src2[i];
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Mask: lower 8 bits = 1, upper 8 bits = 0
    __mmask16 mask = 0x00FF;
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float out[16];
    _mm512_storeu_ps(out, result);
    
    for (int i = 0; i < 16; i++) {
        if (out[i] != expected[i]) return 0;
    }
    return 1;
}
#endif

// ==================== Main Driver ====================
int main() {
    int all_passed = 1;
    
    printf("Testing AVX-512 vector blend operations:\n");
    printf("========================================\n");
    
#ifdef __AVX512BW__
    all_passed &= test_v64qimode();
    verify_result("V64QImode", test_v64qimode());
    
    all_passed &= test_v32himode();
    verify_result("V32HImode", test_v32himode());
#else
    printf("V64QImode   : SKIP (AVX512BW not enabled)\n");
    printf("V32HImode   : SKIP (AVX512BW not enabled)\n");
#endif

#ifdef __AVX512FP16__
    all_passed &= test_v32hfmode();
    verify_result("V32HFmode", test_v32hfmode());
#else
    printf("V32HFmode   : SKIP (AVX512FP16 not enabled)\n");
#endif

#ifdef __AVX512BF16__
    all_passed &= test_v32bfmode();
    verify_result("V32BFmode", test_v32bfmode());
#else
    printf("V32BFmode   : SKIP (AVX512BF16 not enabled)\n");
#endif

#ifdef __AVX512F__
    all_passed &= test_v16simode();
    verify_result("V16SImode", test_v16simode());
    
    all_passed &= test_v8dimode();
    verify_result("V8DImode", test_v8dimode());
    
    all_passed &= test_v8dfmode();
    verify_result("V8DFmode", test_v8dfmode());
    
    all_passed &= test_v16sfmode();
    verify_result("V16SFmode", test_v16sfmode());
#else
    printf("V16SImode   : SKIP (AVX512F not enabled)\n");
    printf("V8DImode    : SKIP (AVX512F not enabled)\n");
    printf("V8DFmode    : SKIP (AVX512F not enabled)\n");
    printf("V16SFmode   : SKIP (AVX512F not enabled)\n");
#endif
    
    printf("\nOverall: %s\n", all_passed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return all_passed ? 0 : 1;
}
