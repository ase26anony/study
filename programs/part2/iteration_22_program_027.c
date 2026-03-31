#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Global volatile arrays to prevent dead code elimination
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef32[16];
volatile double g_storedf64[8];
volatile _Float16 g_storef16[32];
volatile __bf16 g_storebf16[32];

// Complex test function with all blend operations
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // V64QImode: 64 x 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        __m512i b = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((void*)g_store8, res);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    // V32HImode: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    // V32HFmode: 32 x half-precision floats
    {
        _Float16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (_Float16)(i * 0.5f + seed);
            b_arr[i] = (_Float16)((31 - i) * 0.5f + seed);
        }
        
        __m512h a = _mm512_set_ph(
            a_arr[0], a_arr[1], a_arr[2], a_arr[3], a_arr[4], a_arr[5], a_arr[6], a_arr[7],
            a_arr[8], a_arr[9], a_arr[10], a_arr[11], a_arr[12], a_arr[13], a_arr[14], a_arr[15],
            a_arr[16], a_arr[17], a_arr[18], a_arr[19], a_arr[20], a_arr[21], a_arr[22], a_arr[23],
            a_arr[24], a_arr[25], a_arr[26], a_arr[27], a_arr[28], a_arr[29], a_arr[30], a_arr[31]
        );
        
        __m512h b = _mm512_set_ph(
            b_arr[0], b_arr[1], b_arr[2], b_arr[3], b_arr[4], b_arr[5], b_arr[6], b_arr[7],
            b_arr[8], b_arr[9], b_arr[10], b_arr[11], b_arr[12], b_arr[13], b_arr[14], b_arr[15],
            b_arr[16], b_arr[17], b_arr[18], b_arr[19], b_arr[20], b_arr[21], b_arr[22], b_arr[23],
            b_arr[24], b_arr[25], b_arr[26], b_arr[27], b_arr[28], b_arr[29], b_arr[30], b_arr[31]
        );
        
        __mmask32 mask = 0x55555555; // Alternating pattern (different from previous)
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph((void*)g_storef16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storef16[i];
        }
    }
    
    // V32BFmode: 32 x brain floats (if supported)
    #ifdef __AVX512BF16__
    {
        __bf16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (__bf16)(i * 0.3f + seed);
            b_arr[i] = (__bf16)((31 - i) * 0.3f + seed);
        }
        
        __m512bh a = _mm512_set_epi16(
            *(unsigned short*)&a_arr[0], *(unsigned short*)&a_arr[1],
            *(unsigned short*)&a_arr[2], *(unsigned short*)&a_arr[3],
            *(unsigned short*)&a_arr[4], *(unsigned short*)&a_arr[5],
            *(unsigned short*)&a_arr[6], *(unsigned short*)&a_arr[7],
            *(unsigned short*)&a_arr[8], *(unsigned short*)&a_arr[9],
            *(unsigned short*)&a_arr[10], *(unsigned short*)&a_arr[11],
            *(unsigned short*)&a_arr[12], *(unsigned short*)&a_arr[13],
            *(unsigned short*)&a_arr[14], *(unsigned short*)&a_arr[15],
            *(unsigned short*)&a_arr[16], *(unsigned short*)&a_arr[17],
            *(unsigned short*)&a_arr[18], *(unsigned short*)&a_arr[19],
            *(unsigned short*)&a_arr[20], *(unsigned short*)&a_arr[21],
            *(unsigned short*)&a_arr[22], *(unsigned short*)&a_arr[23],
            *(unsigned short*)&a_arr[24], *(unsigned short*)&a_arr[25],
            *(unsigned short*)&a_arr[26], *(unsigned short*)&a_arr[27],
            *(unsigned short*)&a_arr[28], *(unsigned short*)&a_arr[29],
            *(unsigned short*)&a_arr[30], *(unsigned short*)&a_arr[31]
        );
        
        __m512bh b = _mm512_set_epi16(
            *(unsigned short*)&b_arr[0], *(unsigned short*)&b_arr[1],
            *(unsigned short*)&b_arr[2], *(unsigned short*)&b_arr[3],
            *(unsigned short*)&b_arr[4], *(unsigned short*)&b_arr[5],
            *(unsigned short*)&b_arr[6], *(unsigned short*)&b_arr[7],
            *(unsigned short*)&b_arr[8], *(unsigned short*)&b_arr[9],
            *(unsigned short*)&b_arr[10], *(unsigned short*)&b_arr[11],
            *(unsigned short*)&b_arr[12], *(unsigned short*)&b_arr[13],
            *(unsigned short*)&b_arr[14], *(unsigned short*)&b_arr[15],
            *(unsigned short*)&b_arr[16], *(unsigned short*)&b_arr[17],
            *(unsigned short*)&b_arr[18], *(unsigned short*)&b_arr[19],
            *(unsigned short*)&b_arr[20], *(unsigned short*)&b_arr[21],
            *(unsigned short*)&b_arr[22], *(unsigned short*)&b_arr[23],
            *(unsigned short*)&b_arr[24], *(unsigned short*)&b_arr[25],
            *(unsigned short*)&b_arr[26], *(unsigned short*)&b_arr[27],
            *(unsigned short*)&b_arr[28], *(unsigned short*)&b_arr[29],
            *(unsigned short*)&b_arr[30], *(unsigned short*)&b_arr[31]
        );
        
        __mmask32 mask = 0x33333333; // Different pattern
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph((void*)g_storebf16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storebf16[i];
        }
    }
    #endif
    
    // V16SImode: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __mmask16 mask = 0xAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    // V8DImode: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        __mmask8 mask = 0xAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    // V8DFmode: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        __mmask8 mask = 0x55; // Different pattern
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd((void*)g_storedf64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_storedf64[i];
        }
    }
    
    // V16SFmode: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __mmask16 mask = 0x5555; // Alternating pattern
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps((void*)g_storef32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef32[i];
        }
    }
    
    return checksum;
}

int main(int argc, char* argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
