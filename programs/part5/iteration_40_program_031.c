#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to print verification results
static void verify_result(const char* mode, int passed) {
    printf("%-15s: %s\n", mode, passed ? "PASS" : "FAIL");
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
static int test_v64qimode() {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64], expected[64], result[64];
    
    for (int i = 0; i < 64; i++) {
        src1[i] = i;          // 0, 1, 2, ...
        src2[i] = 255 - i;    // 255, 254, 253, ...
    }
    
    // Create alternating mask: 0xAA = 10101010 binary
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Load data into vectors
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    // Perform blend: select from v1 where mask bit=1, from v2 where mask bit=0
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    // Store and verify
    _mm512_storeu_si512((__m512i*)result, blended);
    
    // Compute expected values
    for (int i = 0; i < 64; i++) {
        expected[i] = (mask & (1ULL << i)) ? src1[i] : src2[i];
    }
    
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
    
    // Create checkerboard mask: 0x5555 = 01010101...
    __mmask32 mask = 0x55555555;
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 32; i++) {
        expected[i] = (mask & (1U << i)) ? src1[i] : src2[i];
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
    
    // Mask: first 16 elements from src1, last 16 from src2
    __mmask32 mask = 0x0000FFFF;
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Blend using mask
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_storeu_ph(result, blended);
    
    for (int i = 0; i < 32; i++) {
        expected[i] = (mask & (1U << i)) ? src1[i] : src2[i];
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

// ==================== V32BFmode (32x brain float) ====================
#ifdef __AVX512BF16__
#include <float.h>
static int test_v32bfmode() {
    // BF16 is typically handled through conversion intrinsics
    // We'll use a pattern that should trigger blend operations
    __m512bh src1, src2, blended;
    uint16_t temp1[32], temp2[32], result[32];
    
    for (int i = 0; i < 32; i++) {
        temp1[i] = i * 100;
        temp2[i] = 65535 - i * 100;
    }
    
    // Load as BF16 vectors
    src1 = _mm512_loadu_si512((const __m512i*)temp1);
    src2 = _mm512_loadu_si512((const __m512i*)temp2);
    
    // Create mask: alternating pattern
    __mmask32 mask = 0xAAAAAAAA;
    
    // Use integer blend since direct BF16 blend might not be available
    // This should still trigger the V32BFmode code path
    __m512i v1 = _mm512_castsi512_si512((__m512i)src1);
    __m512i v2 = _mm512_castsi512_si512((__m512i)src2);
    __m512i blended_i = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)result, blended_i);
    
    // Verify
    for (int i = 0; i < 32; i++) {
        uint16_t expected = (mask & (1U << i)) ? temp1[i] : temp2[i];
        if (result[i] != expected) {
            return 0;
        }
    }
    return 1;
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
    
    // Mask: select even indices from src1, odd from src2
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 16; i++) {
        expected[i] = (mask & (1U << i)) ? src1[i] : src2[i];
    }
    
    return memcmp(result, expected, 16 * sizeof(int32_t)) == 0;
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
static int test_v8dimode() {
    int64_t src1[8], src2[8], expected[8], result[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = -1000000LL * i;
    }
    
    // Mask: select first 4 from src1, last 4 from src2
    __mmask8 mask = 0x0F;  // 00001111
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_storeu_si512((__m512i*)result, blended);
    
    for (int i = 0; i < 8; i++) {
        expected[i] = (mask & (1U << i)) ? src1[i] : src2[i];
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
    
    // Mask: alternating pattern
    __mmask8 mask = 0xAA;  // 10101010
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    _mm512_storeu_pd(result, blended);
    
    for (int i = 0; i < 8; i++) {
        expected[i] = (mask & (1U << i)) ? src1[i] : src2[i];
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
    
    // Mask: first half from src1, second half from src2
    __mmask16 mask = 0x00FF;  // 0000000011111111
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    _mm512_storeu_ps(result, blended);
    
    for (int i = 0; i < 16; i++) {
        expected[i] = (mask & (1U << i)) ? src1[i] : src2[i];
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
    printf("V64QImode     : SKIP (AVX512BW not enabled)\n");
    printf("V32HImode     : SKIP (AVX512BW not enabled)\n");
#endif

#ifdef __AVX512FP16__
    all_passed &= test_v32hfmode();
    verify_result("V32HFmode", test_v32hfmode());
#else
    printf("V32HFmode     : SKIP (AVX512FP16 not enabled)\n");
#endif

#ifdef __AVX512BF16__
    all_passed &= test_v32bfmode();
    verify_result("V32BFmode", test_v32bfmode());
#else
    printf("V32BFmode     : SKIP (AVX512BF16 not enabled)\n");
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
    printf("V16SImode     : SKIP (AVX512F not enabled)\n");
    printf("V8DImode      : SKIP (AVX512F not enabled)\n");
    printf("V8DFmode      : SKIP (AVX512F not enabled)\n");
    printf("V16SFmode     : SKIP (AVX512F not enabled)\n");
#endif
    
    printf("\nOverall: %s\n", all_passed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    
    // Force compiler to generate all blend instructions by using results
    volatile int dummy = all_passed;
    return dummy ? 0 : 1;
}
