/* AVX-512 Vector Blend Coverage Test
 * Targets all vector modes in i386-expand.cc switch statement
 * Compile with: gcc -O3 -mavx512f -mavx512bw -mavx512fp16 -march=native test.c -o test
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef __AVX512F__

/* V16SFmode: 16 single-precision floats */
__attribute__((noinline))
float test_v16sf_blend(void) {
    __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                              9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
    __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                              900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
    
    /* Create alternating mask: 0xAAAA = 1010101010101010 */
    __mmask16 mask = 0xAAAA;
    
    /* This should generate vblendmps */
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    /* Use result to prevent optimization */
    float sum = 0.0f;
    float* res = (float*)&result;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DFmode: 8 double-precision floats */
__attribute__((noinline))
double test_v8df_blend(void) {
    __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
    
    /* Create mask: 0xAA = 10101010 */
    __mmask8 mask = 0xAA;
    
    /* This should generate vblendmpd */
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double sum = 0.0;
    double* res = (double*)&result;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

/* V16SImode: 16 32-bit integers */
__attribute__((noinline))
int64_t test_v16si_blend(void) {
    __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800,
                                  900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
    
    __mmask16 mask = 0xAAAA; /* Alternating pattern */
    
    /* This should generate vblendmd */
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    int64_t sum = 0;
    int32_t* res = (int32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += res[i];
    }
    return sum;
}

/* V8DImode: 8 64-bit integers */
__attribute__((noinline))
int64_t test_v8di_blend(void) {
    __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
    __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
    
    __mmask8 mask = 0xAA; /* Alternating pattern */
    
    /* This should generate vblendmq */
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    int64_t sum = 0;
    int64_t* res = (int64_t*)&result;
    for (int i = 0; i < 8; i++) {
        sum += res[i];
    }
    return sum;
}

#ifdef __AVX512BW__

/* V64QImode: 64 8-bit integers */
__attribute__((noinline))
int64_t test_v64qi_blend(void) {
    /* Create arrays with distinct patterns */
    uint8_t a_data[64];
    uint8_t b_data[64];
    
    for (int i = 0; i < 64; i++) {
        a_data[i] = i;
        b_data[i] = i + 100;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    /* Create complex mask pattern */
    __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL; /* Alternating bits */
    
    /* This should generate vblendmb */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    int64_t sum = 0;
    uint8_t* res = (uint8_t*)&result;
    for (int i = 0; i < 64; i++) {
        sum += res[i];
    }
    return sum;
}

/* V32HImode: 32 16-bit integers */
__attribute__((noinline))
int64_t test_v32hi_blend(void) {
    /* Create arrays with distinct patterns */
    uint16_t a_data[32];
    uint16_t b_data[32];
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = i;
        b_data[i] = i + 1000;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA; /* 10101010... pattern */
    
    /* This should generate vblendmw */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    int64_t sum = 0;
    uint16_t* res = (uint16_t*)&result;
    for (int i = 0; i < 32; i++) {
        sum += res[i];
    }
    return sum;
}

#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__

/* V32HFmode: 32 half-precision floats */
__attribute__((noinline))
float test_v32hf_blend(void) {
    /* Create half-precision arrays */
    _Float16 a_data[32];
    _Float16 b_data[32];
    
    for (int i = 0; i < 32; i++) {
        a_data[i] = (_Float16)(i + 1);
        b_data[i] = (_Float16)((i + 1) * 10.0f);
    }
    
    __m512h a = _mm512_loadu_ph(a_data);
    __m512h b = _mm512_loadu_ph(b_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA; /* 10101010... pattern */
    
    /* This should generate vblendmps for half-precision */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    float sum = 0.0f;
    _Float16* res = (_Float16*)&result;
    for (int i = 0; i < 32; i++) {
        sum += (float)res[i];
    }
    return sum;
}

/* V32BFmode: 32 brain float (bfloat16) */
__attribute__((noinline))
float test_v32bf_blend(void) {
    /* bfloat16 requires special handling - use integer blend as fallback */
    uint16_t a_data[32];
    uint16_t b_data[32];
    
    /* Create simple bfloat16-like patterns */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 representation of small integers */
        a_data[i] = (i + 1) << 7;  /* Approximate bfloat16 of (i+1) */
        b_data[i] = ((i + 1) * 10) << 7;
    }
    
    __m512i a = _mm512_loadu_si512((__m512i*)a_data);
    __m512i b = _mm512_loadu_si512((__m512i*)b_data);
    
    __mmask32 mask = 0x55555555; /* Different pattern: 01010101... */
    
    /* For bfloat16, we might need to use integer blend since direct
     * bfloat16 blend intrinsics might not be available */
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    float sum = 0.0f;
    uint16_t* res = (uint16_t*)&result;
    for (int i = 0; i < 32; i++) {
        /* Convert back to approximate float */
        sum += (float)(res[i] >> 7);
    }
    return sum;
}

#endif /* __AVX512FP16__ */

#endif /* __AVX512F__ */

/* Main test driver */
int main(void) {
    printf("AVX-512 Vector Blend Coverage Test\n");
    printf("==================================\n");
    
#ifdef __AVX512F__
    printf("Testing AVX-512F modes:\n");
    
    float sf_result = test_v16sf_blend();
    printf("  V16SFmode blend result sum: %f\n", sf_result);
    
    double df_result = test_v8df_blend();
    printf("  V8DFmode blend result sum: %f\n", df_result);
    
    int64_t si_result = test_v16si_blend();
    printf("  V16SImode blend result sum: %ld\n", si_result);
    
    int64_t di_result = test_v8di_blend();
    printf("  V8DImode blend result sum: %ld\n", di_result);
    
#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW modes:\n");
    
    int64_t qi_result = test_v64qi_blend();
    printf("  V64QImode blend result sum: %ld\n", qi_result);
    
    int64_t hi_result = test_v32hi_blend();
    printf("  V32HImode blend result sum: %ld\n", hi_result);
#endif
    
#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 modes:\n");
    
    float hf_result = test_v32hf_blend();
    printf("  V32HFmode blend result sum: %f\n", hf_result);
    
    float bf_result = test_v32bf_blend();
    printf("  V32BFmode blend result sum: %f\n", bf_result);
#endif
    
    printf("\nAll tests completed successfully!\n");
#else
    printf("AVX-512F not supported on this compiler/platform.\n");
#endif
    
    return 0;
}
