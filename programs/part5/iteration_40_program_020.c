#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to verify results
static int verify_result(const char* test_name, int passed) {
    printf("%s: %s\n", test_name, passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
static int test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64], expected[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ..., 63
        src2[i] = 255 - i;     // 255, 254, ..., 192
    }
    
    // Create alternating mask: 0xAA...AA (alternating 1s and 0s)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Load data
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Perform blend: vblendmb instruction
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store result
    _mm512_storeu_si512((__m512i*)result, blended);
    
    // Compute expected values
    for (int i = 0; i < 64; i++) {
        expected[i] = (mask & (1ULL << i)) ? src2[i] : src1[i];
    }
    
    // Verify
    return memcmp(result, expected, 64) == 0;
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
static int test_v32himode() {
    uint16_t src1[32], src2[32], expected[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 65535 - i * 100;
    }
    
    // Create checkerboard mask
    __mmask32 mask = 0xAAAAAAAA;
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // vblendmw instruction
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 32; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    return memcmp(result, expected, 32 * sizeof(uint16_t)) == 0;
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
static int test_v32hfmode() {
    _Float16 src1[32], src2[32], expected[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(100.0f - i * 1.5f);
    }
    
    // Create mask with first half zeros, second half ones
    __mmask32 mask = 0xFFFF0000;
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // vblendmps instruction for half-precision
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_storeu_ph(result, blended);
    
    for (int i = 0; i < 32; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    // Compare with tolerance for floating-point
    for (int i = 0; i < 32; i++) {
        if (result[i] != expected[i]) {
            return 0;
        }
    }
    return 1;
}
#endif

// ==================== V32BFmode (32x brain floats) ====================
#ifdef __AVX512BF16__
static int test_v32bfmode() {
    // BF16 is stored as uint16_t in memory
    uint16_t src1[32], src2[32], expected[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        // Simple patterns for BF16 values
        src1[i] = i * 128;          // Increasing values
        src2[i] = 32768 + i * 128;  // Different range
    }
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;
    
    __m512bh v1 = _mm512_loadu_epi16(src1);
    __m512bh v2 = _mm512_loadu_epi16(src2);
    
    // Use integer blend since BF16 blend intrinsics might not be directly available
    // This should still generate vblendmps for BF16 elements
    __m512i v1_int = _mm512_castsi512_si512(_mm512_loadu_si512(src1));
    __m512i v2_int = _mm512_castsi512_si512(_mm512_loadu_si512(src2));
    __m512i blended = _mm512_mask_blend_epi16(mask, v1_int, v2_int);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 32; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    return memcmp(result, expected, 32 * sizeof(uint16_t)) == 0;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
static int test_v16simode() {
    int32_t src1[16], src2[16], expected[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
    }
    
    // Create mask: every third element is 1
    __mmask16 mask = 0x9249;  // Binary: 1001001001001001
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // vblendmd instruction
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 16; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    return memcmp(result, expected, 16 * sizeof(int32_t)) == 0;
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
static int test_v8dimode() {
    int64_t src1[8], src2[8], expected[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1LL << (i * 8);
        src2[i] = -(1LL << (i * 8));
    }
    
    // Create mask: alternating pattern
    __mmask8 mask = 0xAA;  // 10101010
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // vblendmq instruction
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 8; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    return memcmp(result, expected, 8 * sizeof(int64_t)) == 0;
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
static int test_v8dfmode() {
    double src1[8], src2[8], expected[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 100.0 - i * 1.5;
    }
    
    // Create mask: first 4 zeros, last 4 ones
    __mmask8 mask = 0xF0;  // 11110000
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // vblendmpd instruction
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_storeu_pd(result, blended);
    
    for (int i = 0; i < 8; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    // Compare with tolerance
    for (int i = 0; i < 8; i++) {
        if (result[i] != expected[i]) {
            return 0;
        }
    }
    return 1;
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
static int test_v16sfmode() {
    float src1[16], src2[16], expected[16], result[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 50.0f - i * 0.5f;
    }
    
    // Create checkerboard mask
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // vblendmps instruction
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_storeu_ps(result, blended);
    
    for (int i = 0; i < 16; i++) {
        expected[i] = (mask & (1U << i)) ? src2[i] : src1[i];
    }
    
    // Compare with tolerance
    for (int i = 0; i < 16; i++) {
        if (result[i] != expected[i]) {
            return 0;
        }
    }
    return 1;
}
#endif

// ==================== Main driver ====================
int main() {
    int failures = 0;
    
    printf("Testing AVX-512 vector blend operations...\n");
    printf("==========================================\n");
    
#ifdef __AVX512BW__
    failures += verify_result("V64QImode (64x 8-bit integers)", test_v64qimode());
    failures += verify_result("V32HImode (32x 16-bit integers)", test_v32himode());
#else
    printf("AVX512BW not available - skipping V64QImode and V32HImode tests\n");
#endif

#ifdef __AVX512FP16__
    failures += verify_result("V32HFmode (32x half-precision floats)", test_v32hfmode());
#else
    printf("AVX512FP16 not available - skipping V32HFmode test\n");
#endif

#ifdef __AVX512BF16__
    failures += verify_result("V32BFmode (32x brain floats)", test_v32bfmode());
#else
    printf("AVX512BF16 not available - skipping V32BFmode test\n");
#endif

#ifdef __AVX512F__
    failures += verify_result("V16SImode (16x 32-bit integers)", test_v16simode());
    failures += verify_result("V8DImode (8x 64-bit integers)", test_v8dimode());
    failures += verify_result("V8DFmode (8x double-precision floats)", test_v8dfmode());
    failures += verify_result("V16SFmode (16x single-precision floats)", test_v16sfmode());
#else
    printf("AVX512F not available - skipping AVX512F tests\n");
#endif
    
    printf("==========================================\n");
    printf("Total failures: %d\n", failures);
    
    return failures > 0 ? 1 : 0;
}
