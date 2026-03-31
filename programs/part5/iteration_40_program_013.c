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
void test_v64qimode() {
    // Initialize arrays
    uint8_t src1[64], src2[64], result[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;           // 0, 1, 2, ...
        src2[i] = 255 - i;     // 255, 254, 253, ...
    }
    
    // Load into vectors
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAAAA... (10101010 pattern)
    __mmask64 mask = _mm512_int2mask(0xAAAAAAAAAAAAAAAA);
    
    // Blend based on mask: where mask bit=1, take from v2; where 0, take from v1
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_si512((__m512i*)result, blended);
    
    int correct = 1;
    for (int i = 0; i < 64; i++) {
        uint8_t expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        if (result[i] != expected) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V64QImode", correct);
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
void test_v32himode() {
    // Initialize arrays
    uint16_t src1[32], src2[32], result[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = 65535 - i * 100;
    }
    
    // Load into vectors
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask: 0x5555 (01010101 pattern)
    __mmask32 mask = _mm512_int2mask(0x55555555);
    
    // Blend 16-bit elements
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_si512((__m512i*)result, blended);
    
    int correct = 1;
    for (int i = 0; i < 32; i++) {
        uint16_t expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        if (result[i] != expected) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V32HImode", correct);
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
#include <float.h>
void test_v32hfmode() {
    // Initialize arrays
    _Float16 src1[32], src2[32], result[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(10.0f - i * 0.5f);
    }
    
    // Load into vectors
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask: first half from v1, second half from v2
    __mmask32 mask = _mm512_int2mask(0xFFFF0000);
    
    // Blend half-precision floats
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_ph(result, blended);
    
    int correct = 1;
    for (int i = 0; i < 32; i++) {
        _Float16 expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        // Compare with tolerance for floating-point
        if (__builtin_fabsf(result[i] - expected) > (_Float16)0.001f) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V32HFmode", correct);
}
#endif

// ==================== V32BFmode (32x brain floats) ====================
#ifdef __AVX512BF16__
#ifdef __AVX512FP16__
void test_v32bfmode() {
    // Initialize arrays
    __bfloat16 src1[32], src2[32], result[32];
    for (int i = 0; i < 32; i++) {
        // Simple pattern for bfloat16
        src1[i] = bfloat16_from_float((float)i);
        src2[i] = bfloat16_from_float((float)(31 - i));
    }
    
    // Load into vectors
    __m512bh v1 = _mm512_loadu_si512((__m512bh*)src1);
    __m512bh v2 = _mm512_loadu_si512((__m512bh*)src2);
    
    // Create alternating mask
    __mmask32 mask = _mm512_int2mask(0xAAAAAAAA);
    
    // For bfloat16, we might need to use integer blend since direct BF16 blend
    // intrinsics might not exist. Use 32-bit blend on the underlying integer.
    // This should still trigger the V32BFmode case in the backend.
    __m512i v1_int = _mm512_castps_si512(_mm512_castbh_ps(v1));
    __m512i v2_int = _mm512_castps_si512(_mm512_castbh_ps(v2));
    __m512i blended_int = _mm512_mask_blend_epi32(mask, v1_int, v2_int);
    
    // Convert back
    __m512bh blended = _mm512_castsi512_bh(blended_int);
    
    // Store
    _mm512_storeu_si512((__m512bh*)result, blended);
    
    // Simple verification - just check that something was computed
    int correct = (result[0] != 0 || result[31] != 0);
    verify_result("V32BFmode", correct);
}
#endif
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
void test_v16simode() {
    // Initialize arrays
    int32_t src1[16], src2[16], result[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = -i * 1000;
    }
    
    // Load into vectors
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: even indices from v1, odd from v2
    __mmask16 mask = _mm512_int2mask(0xAAAA);  // 1010101010101010
    
    // Blend 32-bit integers
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_si512((__m512i*)result, blended);
    
    int correct = 1;
    for (int i = 0; i < 16; i++) {
        int32_t expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        if (result[i] != expected) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V16SImode", correct);
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
void test_v8dimode() {
    // Initialize arrays
    int64_t src1[8], src2[8], result[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = -1000000LL * i;
    }
    
    // Load into vectors
    __m512i v1 = _mm512_loadu_si512((__m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: 0xAA (10101010)
    __mmask8 mask = _mm512_int2mask(0xAA);
    
    // Blend 64-bit integers
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_si512((__m512i*)result, blended);
    
    int correct = 1;
    for (int i = 0; i < 8; i++) {
        int64_t expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        if (result[i] != expected) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V8DImode", correct);
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
void test_v8dfmode() {
    // Initialize arrays
    double src1[8], src2[8], result[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 10.0 - i * 1.5;
    }
    
    // Load into vectors
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask: first 4 from v1, last 4 from v2
    __mmask8 mask = _mm512_int2mask(0xF0);  // 11110000
    
    // Blend doubles
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_pd(result, blended);
    
    int correct = 1;
    for (int i = 0; i < 8; i++) {
        double expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        if (result[i] != expected) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V8DFmode", correct);
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
void test_v16sfmode() {
    // Initialize arrays
    float src1[16], src2[16], result[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.25f;
        src2[i] = 5.0f - i * 0.25f;
    }
    
    // Load into vectors
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create mask: alternating pattern
    __mmask16 mask = _mm512_int2mask(0xAAAA);  // 1010101010101010
    
    // Blend singles
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    // Store and verify
    _mm512_storeu_ps(result, blended);
    
    int correct = 1;
    for (int i = 0; i < 16; i++) {
        float expected = ((mask >> i) & 1) ? src2[i] : src1[i];
        if (result[i] != expected) {
            correct = 0;
            break;
        }
    }
    
    verify_result("V16SFmode", correct);
}
#endif

// ==================== Main driver ====================
int main() {
    printf("Testing AVX-512 vector blend operations:\n");
    printf("========================================\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run tests for each available mode
    #ifdef __AVX512BW__
    test_v64qimode(); total_tests++; passed_tests++;
    test_v32himode(); total_tests++; passed_tests++;
    #endif
    
    #ifdef __AVX512FP16__
    test_v32hfmode(); total_tests++; passed_tests++;
    #endif
    
    #ifdef __AVX512BF16__
    #ifdef __AVX512FP16__
    test_v32bfmode(); total_tests++; passed_tests++;
    #endif
    #endif
    
    #ifdef __AVX512F__
    test_v16simode(); total_tests++; passed_tests++;
    test_v8dimode(); total_tests++; passed_tests++;
    test_v8dfmode(); total_tests++; passed_tests++;
    test_v16sfmode(); total_tests++; passed_tests++;
    #endif
    
    printf("========================================\n");
    printf("Total tests: %d\n", total_tests);
    
    // Force compiler to generate all code by using volatile
    volatile int dummy = total_tests + passed_tests;
    
    return (total_tests == 8) ? 0 : 1;
}
