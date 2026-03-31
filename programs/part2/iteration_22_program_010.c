/* Test program to cover AVX-512 blend expansion in i386-expand.cc */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef32[16];
volatile double g_storef64[8];
volatile uint16_t g_storef16[32];  /* For half-precision */
volatile uint16_t g_storebf16[32]; /* For brain float */

/* Main test function with all blend operations */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* V64QImode: 64 x 8-bit integers */
    {
        __m512i a = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        __m512i b = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((void*)g_store8, res);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    /* V32HImode: 32 x 16-bit integers */
    {
        __m512i a = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        __m512i b = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    /* V16SImode: 16 x 32-bit integers */
    {
        __m512i a = _mm512_set_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        __m512i b = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask16 mask = 0xAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    /* V8DImode: 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    /* V16SFmode: 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
        );
        __m512 b = _mm512_set_ps(
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __mmask16 mask = 0xAAAA; /* Alternating pattern */
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps((void*)g_storef32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef32[i];
        }
    }
    
    /* V8DFmode: 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        __m512d b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd((void*)g_storef64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_storef64[i];
        }
    }
    
    /* V32HFmode: 32 x half-precision floats (requires AVX512-FP16) */
    #ifdef __AVX512FP16__
    {
        __m512h a = _mm512_set_ph(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        __m512h b = _mm512_set_ph(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph((void*)g_storef16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_storef16[i];
        }
    }
    #endif
    
    /* V32BFmode: 32 x brain floats (requires AVX512-BF16) */
    #ifdef __AVX512BF16__
    {
        __m512bh a = _mm512_set_ph(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        __m512bh b = _mm512_set_ph(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph((void*)g_storebf16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_storebf16[i];
        }
    }
    #endif
    
    return checksum + seed; /* Add seed to vary result */
}

int main(int argc, char *argv[]) {
    /* Use argv[1] as seed if provided, otherwise use fixed value */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call the test function */
    int result = test_avx512_blend(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Blend test checksum: %d\n", result);
    
    return 0;
}
