/* Test program to cover AVX-512 blend expansion in i386-expand.cc */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int8_t g_result_i8[64];
volatile int16_t g_result_i16[32];
volatile int32_t g_result_i32[16];
volatile int64_t g_result_i64[8];
volatile float g_result_f32[16];
volatile double g_result_f64[8];
volatile uint16_t g_result_f16[32];
volatile uint16_t g_result_bf16[32];

/* Main test function with all blend operations */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* 1. V64QImode - 64 x 8-bit integers */
    {
        __m512i a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi8(
            127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112,
            111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96,
            95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80,
            79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64
        );
        
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((void*)g_result_i8, result);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_result_i8[i];
        }
    }
    
    /* 2. V32HImode - 32 x 16-bit integers */
    {
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32
        );
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((void*)g_result_i16, result);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_i16[i];
        }
    }
    
    /* 3. V16SImode - 16 x 32-bit integers */
    {
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16
        );
        
        __mmask16 mask = 0xAAAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((void*)g_result_i32, result);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_result_i32[i];
        }
    }
    
    /* 4. V8DImode - 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((void*)g_result_i64, result);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_result_i64[i];
        }
    }
    
    /* 5. V16SFmode - 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 b = _mm512_set_ps(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
        );
        
        __mmask16 mask = 0xAAAA; /* Alternating pattern */
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps((void*)g_result_f32, result);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_result_f32[i];
        }
    }
    
    /* 6. V8DFmode - 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd((void*)g_result_f64, result);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_result_f64[i];
        }
    }
    
    /* 7. V32HFmode - 32 x half-precision floats (16-bit floats) */
    {
        /* Use integer operations to create half-precision patterns */
        __m512i a = _mm512_set_epi16(
            0x3C00, 0x3B80, 0x3B00, 0x3A80, 0x3A00, 0x3980, 0x3900, 0x3880,
            0x3800, 0x3780, 0x3700, 0x3680, 0x3600, 0x3580, 0x3500, 0x3480,
            0x3400, 0x3380, 0x3300, 0x3280, 0x3200, 0x3180, 0x3100, 0x3080,
            0x3000, 0x2F80, 0x2F00, 0x2E80, 0x2E00, 0x2D80, 0x2D00, 0x2C80
        );
        
        __m512i b = _mm512_set_epi16(
            0x4000, 0x3F80, 0x3F00, 0x3E80, 0x3E00, 0x3D80, 0x3D00, 0x3C80,
            0x3C00, 0x3B80, 0x3B00, 0x3A80, 0x3A00, 0x3980, 0x3900, 0x3880,
            0x3800, 0x3780, 0x3700, 0x3680, 0x3600, 0x3580, 0x3500, 0x3480,
            0x3400, 0x3380, 0x3300, 0x3280, 0x3200, 0x3180, 0x3100, 0x3080
        );
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((void*)g_result_f16, result);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_f16[i];
        }
    }
    
    /* 8. V32BFmode - 32 x bfloat16 floats */
    {
        /* Use integer operations to create bfloat16 patterns */
        __m512i a = _mm512_set_epi16(
            0x3F80, 0x3F00, 0x3E80, 0x3E00, 0x3D80, 0x3D00, 0x3C80, 0x3C00,
            0x3B80, 0x3B00, 0x3A80, 0x3A00, 0x3980, 0x3900, 0x3880, 0x3800,
            0x3780, 0x3700, 0x3680, 0x3600, 0x3580, 0x3500, 0x3480, 0x3400,
            0x3380, 0x3300, 0x3280, 0x3200, 0x3180, 0x3100, 0x3080, 0x3000
        );
        
        __m512i b = _mm512_set_epi16(
            0x4000, 0x3F80, 0x3F00, 0x3E80, 0x3E00, 0x3D80, 0x3D00, 0x3C80,
            0x3C00, 0x3B80, 0x3B00, 0x3A80, 0x3A00, 0x3980, 0x3900, 0x3880,
            0x3800, 0x3780, 0x3700, 0x3680, 0x3600, 0x3580, 0x3500, 0x3480,
            0x3400, 0x3380, 0x3300, 0x3280, 0x3200, 0x3180, 0x3100, 0x3080
        );
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((void*)g_result_bf16, result);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_bf16[i];
        }
    }
    
    return checksum + seed;
}

int main(int argc, char *argv[]) {
    volatile int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
