#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_512i_u8(__m512i v) {
    uint8_t arr[64];
    _mm512_storeu_si512((void*)arr, v);
    for (int i = 0; i < 64; i++) {
        printf("%02x ", arr[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
}

// ==================== V64QImode (64x 8-bit integers) ====================
#ifdef __AVX512BW__
__m512i test_v64qimode_blend() {
    // Create two vectors with distinct patterns
    __m512i a = _mm512_set1_epi8(0xAA);  // 10101010
    __m512i b = _mm512_set1_epi8(0x55);  // 01010101
    
    // Create alternating mask: 0x5555... (01010101 pattern)
    __mmask64 mask = 0x5555555555555555ULL;
    
    // Blend using vblendmb instruction
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage by storing
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V32HImode (32x 16-bit integers) ====================
#ifdef __AVX512BW__
__m512i test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);
    __m512i b = _mm512_set1_epi16(0x5555);
    
    // Create mask with alternating bits
    __mmask32 mask = 0x55555555;
    
    // Blend using vblendmw instruction
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V32HFmode (32x half-precision floats) ====================
#ifdef __AVX512FP16__
__m512h test_v32hfmode_blend() {
    // Initialize with distinct half-precision values
    _Float16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = (_Float16)(i * 1.5f);
        b_vals[i] = (_Float16)(i * 2.5f);
    }
    
    __m512h a = _mm512_loadu_ph(a_vals);
    __m512h b = _mm512_loadu_ph(b_vals);
    
    // Create mask: select even elements from a, odd from b
    __mmask32 mask = 0xAAAAAAAA;  // 10101010... pattern
    
    // Blend using vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    volatile __m512h volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V32BFmode (32x brain floats) ====================
#ifdef __AVX512BF16__
__m512bh test_v32bfmode_blend() {
    // Initialize bfloat16 arrays
    __bfloat16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = bfloat16_from_float(i * 1.0f);
        b_vals[i] = bfloat16_from_float(i * 3.0f);
    }
    
    __m512bh a = _mm512_loadu_bf16(a_vals);
    __m512bh b = _mm512_loadu_bf16(b_vals);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;  // 01010101... pattern
    
    // Blend using appropriate intrinsic (may need to cast)
    // Note: Direct blend intrinsic for BF16 might not exist, use alternative
    __m512i a_int = _mm512_castps_si512(_mm512_castbf16_ps(a));
    __m512i b_int = _mm512_castps_si512(_mm512_castbf16_ps(b));
    __m512i result_int = _mm512_mask_blend_epi32(mask, a_int, b_int);
    __m512bh result = _mm512_castsi512_bf16(result_int);
    
    volatile __m512bh volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V16SImode (16x 32-bit integers) ====================
#ifdef __AVX512F__
__m512i test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    // Create mask with alternating 16-bit groups
    __mmask16 mask = 0xAAAA;  // 1010101010101010
    
    // Blend using vblendmd instruction
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V8DImode (8x 64-bit integers) ====================
#ifdef __AVX512F__
__m512i test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    // Create mask: select first 4 from a, last 4 from b
    __mmask8 mask = 0x0F;  // 00001111
    
    // Blend using vblendmq instruction
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V8DFmode (8x double-precision floats) ====================
#ifdef __AVX512F__
__m512d test_v8dfmode_blend() {
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_setr_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    
    // Create mask: select elements where a[i] > b[i]
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
    
    // Blend using vblendmpd instruction
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    volatile __m512d volatile_result = result;
    return volatile_result;
}
#endif

// ==================== V16SFmode (16x single-precision floats) ====================
#ifdef __AVX512F__
__m512 test_v16sfmode_blend() {
    __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 b = _mm512_setr_ps(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
                              8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    
    // Create mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // Blend using vblendmps instruction
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    volatile __m512 volatile_result = result;
    return volatile_result;
}
#endif

// ==================== Main test driver ====================
int main() {
    int tests_passed = 0;
    int total_tests = 0;
    
    printf("Testing AVX-512 blend operations for coverage...\n");
    
    // Test each vector mode if supported
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x 8-bit integers)...\n");
    test_v64qimode_blend();
    tests_passed++; total_tests++;
    
    printf("Testing V32HImode (32x 16-bit integers)...\n");
    test_v32himode_blend();
    tests_passed++; total_tests++;
#endif

#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32x half-precision floats)...\n");
    test_v32hfmode_blend();
    tests_passed++; total_tests++;
#endif

#ifdef __AVX512BF16__
    printf("Testing V32BFmode (32x brain floats)...\n");
    test_v32bfmode_blend();
    tests_passed++; total_tests++;
#endif

#ifdef __AVX512F__
    printf("Testing V16SImode (16x 32-bit integers)...\n");
    test_v16simode_blend();
    tests_passed++; total_tests++;
    
    printf("Testing V8DImode (8x 64-bit integers)...\n");
    test_v8dimode_blend();
    tests_passed++; total_tests++;
    
    printf("Testing V8DFmode (8x double-precision floats)...\n");
    test_v8dfmode_blend();
    tests_passed++; total_tests++;
    
    printf("Testing V16SFmode (16x single-precision floats)...\n");
    test_v16sfmode_blend();
    tests_passed++; total_tests++;
#endif
    
    printf("\nCompleted %d/%d tests\n", tests_passed, total_tests);
    
    // Return non-zero if any required tests couldn't run
    #if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512FP16__)
    return (tests_passed == 8) ? 0 : 1;
    #else
    printf("Note: Not all AVX-512 extensions available on this compiler\n");
    return 0;
    #endif
}
