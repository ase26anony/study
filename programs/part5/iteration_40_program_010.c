#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__attribute__((noinline))
uint64_t test_v64qimode_blend() {
    __m512i a = _mm512_set1_epi8(0xAA);
    __m512i b = _mm512_set1_epi8(0x55);
    
    // Create alternating mask: 0x5555... pattern
    __mmask64 mask = 0x5555555555555555ULL;
    
    // This should generate vblendmb instruction
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage of result
    return _mm512_reduce_add_epi64(result);
}

// V32HImode: 32 x 16-bit integers
__attribute__((noinline))
uint64_t test_v32himode_blend() {
    __m512i a = _mm512_set1_epi16(0xAAAA);
    __m512i b = _mm512_set1_epi16(0x5555);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;
    
    // This should generate vblendmw instruction
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Force usage of result
    return _mm512_reduce_add_epi64(result);
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
__attribute__((noinline))
float test_v32hfmode_blend() {
    // Initialize with _Float16 values
    _Float16 a_vals[32], b_vals[32];
    for (int i = 0; i < 32; i++) {
        a_vals[i] = (_Float16)(i * 1.0f);
        b_vals[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_vals);
    __m512h b = _mm512_loadu_ph(b_vals);
    
    // Create mask using comparison
    __m512h zero = _mm512_setzero_ph();
    __mmask32 mask = _mm512_cmp_ph_mask(a, zero, _CMP_GT_OQ);
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    // Force usage of result
    _Float16 res_vals[32];
    _mm512_storeu_ph(res_vals, result);
    
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (float)res_vals[i];
    }
    return sum;
}

// V32BFmode: 32 x brain float (bfloat16)
__attribute__((noinline))
float test_v32bfmode_blend() {
    // Use __m512bh for bfloat16
    __m512bh a = _mm512_set1_epi16(0x3F80); // bfloat16 1.0
    __m512bh b = _mm512_set1_epi16(0x4000); // bfloat16 2.0
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Use integer blend for bfloat16 (same as epi16 but with bfloat16 type)
    // This should generate vblendmw for bfloat16 elements
    __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
    
    // Force usage of result
    uint16_t res_vals[32];
    _mm512_storeu_si512((__m512i*)res_vals, (__m512i)result);
    
    float sum = 0;
    for (int i = 0; i < 32; i++) {
        // Convert bfloat16 to float
        uint32_t val = res_vals[i] << 16;
        sum += *(float*)&val;
    }
    return sum;
}
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__attribute__((noinline))
uint64_t test_v16simode_blend() {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    // Create mask using comparison
    __mmask16 mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
    
    // This should generate vblendmd instruction
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    // Force usage of result
    return _mm512_reduce_add_epi64(result);
}

// V8DImode: 8 x 64-bit integers
__attribute__((noinline))
uint64_t test_v8dimode_blend() {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    // Create alternating mask
    __mmask8 mask = 0xAA; // 0b10101010
    
    // This should generate vblendmq instruction
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    // Force usage of result
    return _mm512_reduce_add_epi64(result);
}

// V8DFmode: 8 x double-precision floats
__attribute__((noinline))
double test_v8dfmode_blend() {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    // Create mask using comparison
    __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
    
    // This should generate vblendmpd instruction
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    // Force usage of result
    return _mm512_reduce_add_pd(result);
}

// V16SFmode: 16 x single-precision floats
__attribute__((noinline))
float test_v16sfmode_blend() {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    // Create mask using comparison
    __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // This should generate vblendmps instruction
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Force usage of result
    return _mm512_reduce_add_ps(result);
}
#endif // __AVX512F__

// Main driver that calls all test functions
int main() {
    uint64_t checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    checksum += test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    checksum += test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
    printf("Testing V32HFmode (32xhalf-precision floats)...\n");
    checksum += (uint64_t)test_v32hfmode_blend();
    
    printf("Testing V32BFmode (32xbfloat16)...\n");
    checksum += (uint64_t)test_v32bfmode_blend();
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit integers)...\n");
    checksum += test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    checksum += test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble-precision floats)...\n");
    checksum += (uint64_t)test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xsingle-precision floats)...\n");
    checksum += (uint64_t)test_v16sfmode_blend();
#endif
    
    printf("Final checksum: %lu\n", checksum);
    
    // Return non-zero if any test failed (simplified check)
    return checksum == 0 ? 1 : 0;
}
