#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
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

// Main test function with all blend operations
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // 1. V64QImode: 64 x 8-bit integers
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
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL; // Pattern: 10101010...
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((void*)g_store8, res);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i += 8) {
            checksum += g_store8[i];
        }
    }
    
    // 2. V32HImode: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        __m512i b = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask32 mask = 0xAAAAAAAA; // Pattern: 10101010...
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i += 4) {
            checksum += g_store16[i];
        }
    }
    
    // 3. V32HFmode: 32 x half-precision floats
    {
        _Float16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (_Float16)(i * 1.5f);
            b_vals[i] = (_Float16)(i * 2.5f);
        }
        __m512h a = _mm512_loadu_ph(a_vals);
        __m512h b = _mm512_loadu_ph(b_vals);
        __mmask32 mask = 0x55555555; // Pattern: 01010101... (different from previous)
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph((void*)g_storef16, res);
        
        for (int i = 0; i < 32; i += 4) {
            checksum += (int)g_storef16[i];
        }
    }
    
    // 4. V32BFmode: 32 x brain floats (bfloat16)
    {
        __bf16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (__bf16)(i * 1.25f);
            b_vals[i] = (__bf16)(i * 3.75f);
        }
        __m512bh a = _mm512_loadu_bf16(a_vals);
        __m512bh b = _mm512_loadu_bf16(b_vals);
        __mmask32 mask = 0xCCCCCCCC; // Pattern: 11001100...
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_bf16((void*)g_storebf16, res);
        
        for (int i = 0; i < 32; i += 4) {
            checksum += (int)g_storebf16[i];
        }
    }
    
    // 5. V16SImode: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        __m512i b = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask16 mask = 0xAAAA; // Pattern: 10101010...
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i += 2) {
            checksum += g_store32[i];
        }
    }
    
    // 6. V8DImode: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __mmask8 mask = 0xAA; // Pattern: 10101010
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    // 7. V8DFmode: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7);
        __m512d b = _mm512_set_pd(7.7, 6.6, 5.5, 4.4, 3.3, 2.2, 1.1, 0.0);
        __mmask8 mask = 0x55; // Pattern: 01010101
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd((void*)g_storedf64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_storedf64[i];
        }
    }
    
    // 8. V16SFmode: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(
            0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f,
            8.8f, 9.9f, 10.1f, 11.1f, 12.2f, 13.3f, 14.4f, 15.5f
        );
        __m512 b = _mm512_set_ps(
            15.5f, 14.4f, 13.3f, 12.2f, 11.1f, 10.1f, 9.9f, 8.8f,
            7.7f, 6.6f, 5.5f, 4.4f, 3.3f, 2.2f, 1.1f, 0.0f
        );
        __mmask16 mask = 0xCCCC; // Pattern: 11001100...
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps((void*)g_storef32, res);
        
        for (int i = 0; i < 16; i += 2) {
            checksum += (int)g_storef32[i];
        }
    }
    
    return checksum + seed;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
