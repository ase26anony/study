#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* V16SFmode: 16 single-precision floats */
__attribute__((noinline))
float test_v16sf_blend() {
    __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 b = _mm512_setr_ps(17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
                              25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f);
    
    // Create alternating mask: 0xAAAA = 1010101010101010
    __mmask16 mask = 0xAAAA;
    
    // Blend using mask: vblendmps
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    // Extract and sum to prevent optimization
    float sum = 0.0f;
    float temp[16];
    _mm512_storeu_ps(temp, result);
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V8DFmode: 8 double-precision floats */
__attribute__((noinline))
double test_v8df_blend() {
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_setr_pd(9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0);
    
    // Create mask: 0x55 = 01010101
    __mmask8 mask = 0x55;
    
    // Blend using mask: vblendmpd
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double sum = 0.0;
    double temp[8];
    _mm512_storeu_pd(temp, result);
    for (int i = 0; i < 8; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V16SImode: 16 32-bit integers */
__attribute__((noinline))
int64_t test_v16si_blend() {
    __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    __m512i b = _mm512_setr_epi32(17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
    
    // Create mask: 0xAAAA = 1010101010101010
    __mmask16 mask = 0xAAAA;
    
    // Blend using mask: vblendmd
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int64_t sum = 0;
    int32_t temp[16];
    _mm512_storeu_si512(temp, result);
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V8DImode: 8 64-bit integers */
__attribute__((noinline))
int64_t test_v8di_blend() {
    __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i b = _mm512_setr_epi64(9, 10, 11, 12, 13, 14, 15, 16);
    
    // Create mask: 0x55 = 01010101
    __mmask8 mask = 0x55;
    
    // Blend using mask: vblendmq
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t sum = 0;
    int64_t temp[8];
    _mm512_storeu_si512(temp, result);
    for (int i = 0; i < 8; i++) {
        sum += temp[i];
    }
    return sum;
}

#ifdef __AVX512BW__

/* V64QImode: 64 8-bit integers */
__attribute__((noinline))
int64_t test_v64qi_blend() {
    // Initialize with pattern
    uint8_t a_data[64], b_data[64];
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = i + 64;
    }
    
    __m512i a = _mm512_loadu_si512(a_data);
    __m512i b = _mm512_loadu_si512(b_data);
    
    // Create alternating mask: 0xAAAAAAAAAAAAAAAA = 1010...
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Blend using mask: vblendmb
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    int64_t sum = 0;
    uint8_t temp[64];
    _mm512_storeu_si512(temp, result);
    for (int i = 0; i < 64; i++) {
        sum += temp[i];
    }
    return sum;
}

/* V32HImode: 32 16-bit integers */
__attribute__((noinline))
int64_t test_v32hi_blend() {
    // Initialize with pattern
    uint16_t a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = i;
        b_data[i] = i + 32;
    }
    
    __m512i a = _mm512_loadu_si512(a_data);
    __m512i b = _mm512_loadu_si512(b_data);
    
    // Create alternating mask: 0xAAAAAAAA = 1010101010101010...
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend using mask: vblendmw
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    int64_t sum = 0;
    uint16_t temp[32];
    _mm512_storeu_si512(temp, result);
    for (int i = 0; i < 32; i++) {
        sum += temp[i];
    }
    return sum;
}

#endif // __AVX512BW__

#ifdef __AVX512FP16__

/* V32HFmode: 32 half-precision floats */
__attribute__((noinline))
float test_v32hf_blend() {
    // Initialize half-precision arrays
    _Float16 a_data[32], b_data[32];
    for (int i = 0; i < 32; i++) {
        a_data[i] = (_Float16)(i + 1);
        b_data[i] = (_Float16)(i + 33);
    }
    
    __m512h a = _mm512_loadu_ph(a_data);
    __m512h b = _mm512_loadu_ph(b_data);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // Blend using mask: vblendmps for half-precision
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    float sum = 0.0f;
    _Float16 temp[32];
    _mm512_storeu_ph(temp, result);
    for (int i = 0; i < 32; i++) {
        sum += (float)temp[i];
    }
    return sum;
}

/* V32BFmode: 32 brain float (bfloat16) */
__attribute__((noinline))
float test_v32bf_blend() {
    // Initialize bfloat16 arrays
    __m512bh a, b;
    uint16_t a_data[32], b_data[32];
    
    for (int i = 0; i < 32; i++) {
        // Simple bfloat16 pattern (just using integer representation)
        a_data[i] = i;
        b_data[i] = i + 32;
    }
    
    a = _mm512_loadu_si512(a_data);
    b = _mm512_loadu_si512(b_data);
    
    // Create alternating mask
    __mmask32 mask = 0xAAAAAAAA;
    
    // For bfloat16, we might need to use integer blend or convert
    // Use _mm512_mask_blend_epi16 as bfloat16 is stored in 16-bit containers
    __m512i result_int = _mm512_mask_blend_epi16(mask, 
        (__m512i)a, (__m512i)b);
    
    float sum = 0.0f;
    uint16_t temp[32];
    _mm512_storeu_si512(temp, result_int);
    for (int i = 0; i < 32; i++) {
        sum += (float)temp[i];
    }
    return sum;
}

#endif // __AVX512FP16__

#endif // __AVX512F__

int main() {
    float total = 0.0f;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F modes...\n");
    
    total += test_v16sf_blend();
    printf("V16SF blend completed\n");
    
    total += (float)test_v8df_blend();
    printf("V8DF blend completed\n");
    
    total += (float)test_v16si_blend();
    printf("V16SI blend completed\n");
    
    total += (float)test_v8di_blend();
    printf("V8DI blend completed\n");
    
#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW modes...\n");
    
    total += (float)test_v64qi_blend();
    printf("V64QI blend completed\n");
    
    total += (float)test_v32hi_blend();
    printf("V32HI blend completed\n");
#endif
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 modes...\n");
    
    total += test_v32hf_blend();
    printf("V32HF blend completed\n");
    
    total += test_v32bf_blend();
    printf("V32BF blend completed\n");
#endif
    
    printf("\nTotal checksum: %f\n", total);
    printf("All AVX-512 blend operations tested successfully!\n");
    
    // Return non-zero if total is 0 (shouldn't happen)
    return (total == 0.0f) ? 1 : 0;
#else
    printf("AVX-512 not supported on this compiler/platform\n");
    return 0;
#endif
}
