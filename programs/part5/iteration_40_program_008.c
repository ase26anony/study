#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to create alternating mask pattern
__mmask64 create_mask64(int pattern) {
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i & pattern) == 0) {
            mask |= (1ULL << i);
        }
    }
    return mask;
}

__mmask32 create_mask32(int pattern) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i & pattern) == 0) {
            mask |= (1U << i);
        }
    }
    return mask;
}

__mmask16 create_mask16(int pattern) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i & pattern) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

__mmask8 create_mask8(int pattern) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i & pattern) == 0) {
            mask |= (1 << i);
        }
    }
    return mask;
}

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__m512i test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    int8_t a_data[64];
    int8_t b_data[64];
    
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = 64 - i;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
    __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
    
    // Create alternating mask (every other element)
    __mmask64 mask = create_mask64(1);
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage to prevent optimization
    volatile __m512i volatile_result = result;
    return volatile_result;
}

// V32HImode: 32 x 16-bit integers
__m512i test_v32himode_blend() {
    int16_t a_data[32];
    int16_t b_data[32];
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = i * 2;
        b_data[i] = i * 3;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
    __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
    
    // Create mask with pattern 0b0101
    __mmask32 mask = create_mask32(0b0101);
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
#if defined(__AVX512FP16__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
__m512h test_v32hfmode_blend() {
    _Float16 a_data[32];
    _Float16 b_data[32];
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = i * 1.5f;
        b_data[i] = i * 2.5f;
    }
    
    __m512h a = _mm512_loadu_ph(a_data);
    __m512h b = _mm512_loadu_ph(b_data);
    
    // Create mask with alternating pattern
    __mmask32 mask = create_mask32(1);
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    volatile __m512h volatile_result = result;
    return volatile_result;
}

// V32BFmode: 32 x brain float (bfloat16)
// Note: BF16 support may vary, using appropriate intrinsics
__m512bh test_v32bfmode_blend() {
    // Use __m512i as storage for bfloat16 data
    __m512i a = _mm512_set1_epi16(0x3F80); // bfloat16 1.0
    __m512i b = _mm512_set1_epi16(0x4000); // bfloat16 2.0
    
    // Create alternating mask
    __mmask32 mask = create_mask32(1);
    
    // For BF16, we might need to use integer blend or convert
    // This is compiler-dependent and may use vblendmps for BF16
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512bh)a, (__m512bh)b);
    
    volatile __m512bh volatile_result = result;
    return volatile_result;
}
#endif // __AVX512FP16__
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__m512i test_v16simode_blend() {
    int32_t a_data[16];
    int32_t b_data[16];
    
    for (int i = 0; i < 16; i++) {
        a_data[i] = i * 100;
        b_data[i] = i * 200;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
    __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
    
    // Create mask with pattern 0b0011
    __mmask16 mask = create_mask16(0b0011);
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}

// V8DImode: 8 x 64-bit integers
__m512i test_v8dimode_blend() {
    int64_t a_data[8];
    int64_t b_data[8];
    
    for (int i = 0; i < 8; i++) {
        a_data[i] = i * 1000LL;
        b_data[i] = i * 2000LL;
    }
    
    __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
    __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
    
    // Create mask with alternating pattern
    __mmask8 mask = create_mask8(1);
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    volatile __m512i volatile_result = result;
    return volatile_result;
}

// V8DFmode: 8 x double-precision floats
__m512d test_v8dfmode_blend() {
    double a_data[8];
    double b_data[8];
    
    for (int i = 0; i < 8; i++) {
        a_data[i] = i * 1.1;
        b_data[i] = i * 2.2;
    }
    
    __m512d a = _mm512_loadu_pd(a_data);
    __m512d b = _mm512_loadu_pd(b_data);
    
    // Create mask with pattern 0b0101
    __mmask8 mask = create_mask8(0b0101);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    volatile __m512d volatile_result = result;
    return volatile_result;
}

// V16SFmode: 16 x single-precision floats
__m512 test_v16sfmode_blend() {
    float a_data[16];
    float b_data[16];
    
    for (int i = 0; i < 16; i++) {
        a_data[i] = i * 1.5f;
        b_data[i] = i * 3.0f;
    }
    
    __m512 a = _mm512_loadu_ps(a_data);
    __m512 b = _mm512_loadu_ps(b_data);
    
    // Create mask with alternating pattern
    __mmask16 mask = create_mask16(1);
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    volatile __m512 volatile_result = result;
    return volatile_result;
}
#endif // __AVX512F__

// Main driver that calls all test functions
int main() {
    int checksum = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    __m512i v64qi_result = test_v64qimode_blend();
    checksum += _mm512_reduce_add_epi64(v64qi_result);
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    __m512i v32hi_result = test_v32himode_blend();
    checksum += _mm512_reduce_add_epi64(v32hi_result);
#endif
    
#ifdef __AVX512FP16__
#if defined(__AVX512FP16__) && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    printf("Testing V32HFmode (32xhalf-precision floats)...\n");
    __m512h v32hf_result = test_v32hfmode_blend();
    // Convert to integer for checksum
    __m512i v32hf_as_int = (__m512i)v32hf_result;
    checksum += _mm512_reduce_add_epi64(v32hf_as_int);
    
    printf("Testing V32BFmode (32xbfloat16)...\n");
    __m512bh v32bf_result = test_v32bfmode_blend();
    __m512i v32bf_as_int = (__m512i)v32bf_result;
    checksum += _mm512_reduce_add_epi64(v32bf_as_int);
#endif
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit integers)...\n");
    __m512i v16si_result = test_v16simode_blend();
    checksum += _mm512_reduce_add_epi64(v16si_result);
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    __m512i v8di_result = test_v8dimode_blend();
    checksum += _mm512_reduce_add_epi64(v8di_result);
    
    printf("Testing V8DFmode (8xdouble-precision floats)...\n");
    __m512d v8df_result = test_v8dfmode_blend();
    __m512i v8df_as_int = (__m512i)v8df_result;
    checksum += _mm512_reduce_add_epi64(v8df_as_int);
    
    printf("Testing V16SFmode (16xsingle-precision floats)...\n");
    __m512 v16sf_result = test_v16sfmode_blend();
    __m512i v16sf_as_int = (__m512i)v16sf_result;
    checksum += _mm512_reduce_add_epi64(v16sf_as_int);
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("All blend operations tested.\n");
    
    return 0;
}
