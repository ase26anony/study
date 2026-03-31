#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print verification results
static void verify_result(const char* mode, int passed) {
    printf("%s: %s\n", mode, passed ? "PASS" : "FAIL");
}

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__attribute__((noinline))
static uint64_t test_v64qimode(void) {
    // Initialize arrays with distinct patterns
    uint8_t src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 + i;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create alternating mask: 0xAA...AA (alternating bits)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // Blend based on mask
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    // Force usage by computing checksum
    uint8_t res_arr[64];
    _mm512_storeu_si512(res_arr, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += res_arr[i];
    }
    return checksum;
}

// V32HImode: 32 x 16-bit integers
__attribute__((noinline))
static uint64_t test_v32himode(void) {
    uint16_t src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = 1000 + i * 3;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create pattern mask: 0x5555 (alternating)
    __mmask32 mask = 0x55555555;
    
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    uint16_t res_arr[32];
    _mm512_storeu_si512(res_arr, result);
    
    uint64_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += res_arr[i];
    }
    return checksum;
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
__attribute__((noinline))
static float test_v32hfmode(void) {
    // Use _Float16 type for half precision
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(10.0f + i * 0.25f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1);
    __m512h v2 = _mm512_loadu_ph(src2);
    
    // Create mask for blending
    __mmask32 mask = 0xAAAAAAAA;  // Alternating pattern
    
    // Blend half-precision floats
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    // Store and compute sum
    _Float16 res_arr[32];
    _mm512_storeu_ph(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)res_arr[i];
    }
    return sum;
}

// V32BFmode: 32 x brain float (bfloat16)
__attribute__((noinline))
static float test_v32bfmode(void) {
    // Use __m512bh for bfloat16
    uint16_t src1_data[32], src2_data[32];
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 pattern (just using uint16_t representation)
        src1_data[i] = i * 0x40;  // Small increments
        src2_data[i] = 0x4000 + i * 0x40;
    }
    
    __m512bh v1 = _mm512_loadu_epi16(src1_data);
    __m512bh v2 = _mm512_loadu_epi16(src2_data);
    
    __mmask32 mask = 0x55555555;  // Different alternating pattern
    
    // For bfloat16, we might need to use integer blend or cast
    // Use integer blend since bfloat16 blend might not have direct intrinsic
    __m512i v1_int = _mm512_castps_si512(_mm512_castbh_ps(v1));
    __m512i v2_int = _mm512_castps_si512(_mm512_castbh_ps(v2));
    
    __m512i result_int = _mm512_mask_blend_epi32(mask, v1_int, v2_int);
    
    // Convert back and compute checksum
    uint16_t res_arr[32];
    _mm512_storeu_si512(res_arr, result_int);
    
    uint32_t checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += res_arr[i];
    }
    return (float)checksum;
}
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__attribute__((noinline))
static uint64_t test_v16simode(void) {
    int32_t src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = 1000 + i * 20;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create mask using comparison
    __m512i cmp = _mm512_set1_epi32(0);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp);
    
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    int32_t res_arr[16];
    _mm512_storeu_si512(res_arr, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    return sum;
}

// V8DImode: 8 x 64-bit integers
__attribute__((noinline))
static uint64_t test_v8dimode(void) {
    int64_t src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = 10000LL + i * 200LL;
    }
    
    __m512i v1 = _mm512_loadu_si512(src1);
    __m512i v2 = _mm512_loadu_si512(src2);
    
    // Create mask: select even elements from v1, odd from v2
    __mmask8 mask = 0xAA;  // 0b10101010
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    int64_t res_arr[8];
    _mm512_storeu_si512(res_arr, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += res_arr[i];
    }
    return sum;
}

// V8DFmode: 8 x double-precision floats
__attribute__((noinline))
static double test_v8dfmode(void) {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = 10.0 + i * 2.5;
    }
    
    __m512d v1 = _mm512_loadu_pd(src1);
    __m512d v2 = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d zero = _mm512_setzero_pd();
    __mmask8 mask = _mm512_cmp_pd_mask(v1, zero, _CMP_GT_OQ);
    
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    double res_arr[8];
    _mm512_storeu_pd(res_arr, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += res_arr[i];
    }
    return sum;
}

// V16SFmode: 16 x single-precision floats
__attribute__((noinline))
static float test_v16sfmode(void) {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = 5.0f + i * 0.75f;
    }
    
    __m512 v1 = _mm512_loadu_ps(src1);
    __m512 v2 = _mm512_loadu_ps(src2);
    
    // Create alternating mask
    __mmask16 mask = 0xAAAA;  // 0b1010101010101010
    
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    float res_arr[16];
    _mm512_storeu_ps(res_arr, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += res_arr[i];
    }
    return sum;
}
#endif // __AVX512F__

int main(void) {
    int passed = 1;
    
    printf("Testing AVX-512 blend operations for code coverage...\n\n");
    
#ifdef __AVX512BW__
    printf("Testing AVX-512BW modes:\n");
    uint64_t res1 = test_v64qimode();
    verify_result("V64QImode", res1 > 0);
    
    uint64_t res2 = test_v32himode();
    verify_result("V32HImode", res2 > 0);
#endif
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 modes:\n");
    float res3 = test_v32hfmode();
    verify_result("V32HFmode", res3 > 0.0f);
    
    float res4 = test_v32bfmode();
    verify_result("V32BFmode", res4 > 0.0f);
#endif
    
#ifdef __AVX512F__
    printf("\nTesting AVX-512F modes:\n");
    uint64_t res5 = test_v16simode();
    verify_result("V16SImode", res5 > 0);
    
    uint64_t res6 = test_v8dimode();
    verify_result("V8DImode", res6 > 0);
    
    double res7 = test_v8dfmode();
    verify_result("V8DFmode", res7 > 0.0);
    
    float res8 = test_v16sfmode();
    verify_result("V16SFmode", res8 > 0.0f);
#endif
    
    printf("\nAll applicable tests completed.\n");
    
    // Return non-zero if any required feature was missing
#ifdef __AVX512F__
    return 0;
#else
    printf("Warning: AVX-512F not supported by compiler\n");
    return 1;
#endif
}
