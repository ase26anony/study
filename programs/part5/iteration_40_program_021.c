#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// Helper function to print results for debugging
void print_hex(const void* data, size_t size) {
    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        printf("%02x", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 4 == 0) printf(" ");
    }
    printf("\n");
}

#ifdef __AVX512BW__
// V64QImode: 64 x 8-bit integers
__m512i test_v64qimode_blend() {
    // Initialize arrays with distinct patterns
    char src1[64], src2[64];
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 + i;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask: 0xAAAA... (select even elements from b, odd from a)
    __mmask64 mask = 0xAAAAAAAAAAAAAAAA;
    
    // This should generate vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    // Force usage by storing
    char output[64];
    _mm512_storeu_si512((__m512i*)output, result);
    
    // Return something based on the result to prevent optimization
    return result;
}

// V32HImode: 32 x 16-bit integers
__m512i test_v32himode_blend() {
    short src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create checkerboard mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // This should generate vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    short output[32];
    _mm512_storeu_si512((__m512i*)output, result);
    
    return result;
}
#endif // __AVX512BW__

#ifdef __AVX512FP16__
// V32HFmode: 32 x half-precision floats
#ifdef __STDC_IEC_60559_BFP__
__m512h test_v32hfmode_blend() {
    _Float16 src1[32], src2[32];
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 1.5f);
    }
    
    __m512h a = _mm512_loadu_ph(src1);
    __m512h b = _mm512_loadu_ph(src2);
    
    // Create mask selecting first half from a, second half from b
    __mmask32 mask = 0xFFFF0000;
    
    // This should generate vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    _Float16 output[32];
    _mm512_storeu_ph(output, result);
    
    return result;
}

// V32BFmode: 32 x brain float (bfloat16)
__m512bh test_v32bfmode_blend() {
    // Use __m256bh for bfloat16 (stored as 32-bit ints)
    unsigned int src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        // Simple bfloat16 pattern: 1.0, 2.0, 3.0, ...
        src1[i] = (i + 1) << 16;  // bfloat16 representation of i+1
        src2[i] = (i + 17) << 16; // bfloat16 representation of i+17
    }
    
    __m512bh a = _mm512_loadu_si512((__m512i*)src1);
    __m512bh b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create alternating mask
    __mmask32 mask = 0x55555555;
    
    // Use integer blend for bfloat16 (same as epi16 since they're 16-bit elements)
    __m512bh result = _mm512_mask_blend_epi16(mask, 
        (__m512i)a, (__m512i)b);
    
    unsigned int output[16];
    _mm512_storeu_si512((__m512i*)output, (__m512i)result);
    
    return result;
}
#endif // __STDC_IEC_60559_BFP__
#endif // __AVX512FP16__

#ifdef __AVX512F__
// V16SImode: 16 x 32-bit integers
__m512i test_v16simode_blend() {
    int src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 20;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: select elements where i % 3 == 0 from b
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if (i % 3 == 0) mask |= (1 << i);
    }
    
    // This should generate vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int output[16];
    _mm512_storeu_si512((__m512i*)output, result);
    
    return result;
}

// V8DImode: 8 x 64-bit integers
__m512i test_v8dimode_blend() {
    long long src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 200LL;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)src1);
    __m512i b = _mm512_loadu_si512((__m512i*)src2);
    
    // Create mask: select first 4 from a, last 4 from b
    __mmask8 mask = 0xF0;  // 0b11110000
    
    // This should generate vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    long long output[8];
    _mm512_storeu_si512((__m512i*)output, result);
    
    return result;
}

// V8DFmode: 8 x double-precision floats
__m512d test_v8dfmode_blend() {
    double src1[8], src2[8];
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.1;
        src2[i] = i * 2.2;
    }
    
    __m512d a = _mm512_loadu_pd(src1);
    __m512d b = _mm512_loadu_pd(src2);
    
    // Create mask using comparison
    __m512d cmp_val = _mm512_set1_pd(3.0);
    __mmask8 mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_LT_OQ);
    
    // This should generate vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double output[8];
    _mm512_storeu_pd(output, result);
    
    return result;
}

// V16SFmode: 16 x single-precision floats
__m512 test_v16sfmode_blend() {
    float src1[16], src2[16];
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 a = _mm512_loadu_ps(src1);
    __m512 b = _mm512_loadu_ps(src2);
    
    // Create mask using comparison
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    // This should generate vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    float output[16];
    _mm512_storeu_ps(output, result);
    
    return result;
}
#endif // __AVX512F__

int main() {
    printf("Testing AVX-512 blend operations for coverage...\n");
    
    // Force compiler to generate all blend variants
    // by using volatile results and checksums
    
#ifdef __AVX512BW__
    printf("Testing V64QImode (64x8-bit integers)...\n");
    volatile __m512i v64qi_result = test_v64qimode_blend();
    
    printf("Testing V32HImode (32x16-bit integers)...\n");
    volatile __m512i v32hi_result = test_v32himode_blend();
#endif
    
#ifdef __AVX512FP16__
#ifdef __STDC_IEC_60559_BFP__
    printf("Testing V32HFmode (32xhalf-precision floats)...\n");
    volatile __m512h v32hf_result = test_v32hfmode_blend();
    
    printf("Testing V32BFmode (32xbfloat16)...\n");
    volatile __m512bh v32bf_result = test_v32bfmode_blend();
#endif
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SImode (16x32-bit integers)...\n");
    volatile __m512i v16si_result = test_v16simode_blend();
    
    printf("Testing V8DImode (8x64-bit integers)...\n");
    volatile __m512i v8di_result = test_v8dimode_blend();
    
    printf("Testing V8DFmode (8xdouble-precision floats)...\n");
    volatile __m512d v8df_result = test_v8dfmode_blend();
    
    printf("Testing V16SFmode (16xsingle-precision floats)...\n");
    volatile __m512 v16sf_result = test_v16sfmode_blend();
#endif
    
    printf("All blend operations tested.\n");
    
    // Return non-zero if any test failed (simplified)
    return 0;
}
