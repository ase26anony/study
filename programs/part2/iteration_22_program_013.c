#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Global volatile arrays to prevent dead code elimination
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef[16];
volatile double g_stored[8];
volatile _Float16 g_storeh[32];
#ifdef __bf16
volatile __bf16 g_storeb[32];
#endif

// Complex test function with all blend operations
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // V64QImode: 64 x 8-bit integers
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
        
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory
        _mm512_storeu_si512((void*)g_store8, res);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    // V32HImode: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi16(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32
        );
        
        __mmask32 mask = 0xAAAAAAAA;
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    // V32HFmode: 32 x half-precision floats
    {
        _Float16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (_Float16)(i * 1.5f + seed);
            b_vals[i] = (_Float16)(i * 2.5f + seed);
        }
        
        __m512h a = _mm512_set_ph(
            a_vals[31], a_vals[30], a_vals[29], a_vals[28], a_vals[27], a_vals[26], a_vals[25], a_vals[24],
            a_vals[23], a_vals[22], a_vals[21], a_vals[20], a_vals[19], a_vals[18], a_vals[17], a_vals[16],
            a_vals[15], a_vals[14], a_vals[13], a_vals[12], a_vals[11], a_vals[10], a_vals[9], a_vals[8],
            a_vals[7], a_vals[6], a_vals[5], a_vals[4], a_vals[3], a_vals[2], a_vals[1], a_vals[0]
        );
        
        __m512h b = _mm512_set_ph(
            b_vals[31], b_vals[30], b_vals[29], b_vals[28], b_vals[27], b_vals[26], b_vals[25], b_vals[24],
            b_vals[23], b_vals[22], b_vals[21], b_vals[20], b_vals[19], b_vals[18], b_vals[17], b_vals[16],
            b_vals[15], b_vals[14], b_vals[13], b_vals[12], b_vals[11], b_vals[10], b_vals[9], b_vals[8],
            b_vals[7], b_vals[6], b_vals[5], b_vals[4], b_vals[3], b_vals[2], b_vals[1], b_vals[0]
        );
        
        __mmask32 mask = 0x55555555;
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        _mm512_storeu_ph((void*)g_storeh, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storeh[i];
        }
    }
    
#ifdef __bf16
    // V32BFmode: 32 x brain floats (if supported)
    {
        __bfloat16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (__bfloat16)(i * 1.1f + seed);
            b_vals[i] = (__bfloat16)(i * 2.2f + seed);
        }
        
        __m512bh a = _mm512_set_ph(
            a_vals[31], a_vals[30], a_vals[29], a_vals[28], a_vals[27], a_vals[26], a_vals[25], a_vals[24],
            a_vals[23], a_vals[22], a_vals[21], a_vals[20], a_vals[19], a_vals[18], a_vals[17], a_vals[16],
            a_vals[15], a_vals[14], a_vals[13], a_vals[12], a_vals[11], a_vals[10], a_vals[9], a_vals[8],
            a_vals[7], a_vals[6], a_vals[5], a_vals[4], a_vals[3], a_vals[2], a_vals[1], a_vals[0]
        );
        
        __m512bh b = _mm512_set_ph(
            b_vals[31], b_vals[30], b_vals[29], b_vals[28], b_vals[27], b_vals[26], b_vals[25], b_vals[24],
            b_vals[23], b_vals[22], b_vals[21], b_vals[20], b_vals[19], b_vals[18], b_vals[17], b_vals[16],
            b_vals[15], b_vals[14], b_vals[13], b_vals[12], b_vals[11], b_vals[10], b_vals[9], b_vals[8],
            b_vals[7], b_vals[6], b_vals[5], b_vals[4], b_vals[3], b_vals[2], b_vals[1], b_vals[0]
        );
        
        __mmask32 mask = 0xAAAAAAAA;
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        _mm512_storeu_ph((void*)g_storeb, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storeb[i];
        }
    }
#endif
    
    // V16SImode: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __m512i b = _mm512_set_epi32(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16
        );
        
        __mmask16 mask = 0xAAAA;
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    // V8DImode: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(15, 14, 13, 12, 11, 10, 9, 8);
        
        __mmask8 mask = 0xAA;
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    // V8DFmode: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0);
        
        __mmask8 mask = 0x55;
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        _mm512_storeu_pd((void*)g_stored, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_stored[i];
        }
    }
    
    // V16SFmode: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __m512 b = _mm512_set_ps(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
        );
        
        __mmask16 mask = 0xAAAA;
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        _mm512_storeu_ps((void*)g_storef, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef[i];
        }
    }
    
    return checksum;
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
