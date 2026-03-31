#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Global volatile arrays to prevent dead code elimination
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef32[16];
volatile double g_storedf64[8];
volatile _Float16 g_storef16[32];
#ifdef __bf16
volatile __bf16 g_storebf16[32];
#endif

// Main test function with all blend operations
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // 1. V64QImode - 64 x 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15,
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31,
            seed+32, seed+33, seed+34, seed+35, seed+36, seed+37, seed+38, seed+39,
            seed+40, seed+41, seed+42, seed+43, seed+44, seed+45, seed+46, seed+47,
            seed+48, seed+49, seed+50, seed+51, seed+52, seed+53, seed+54, seed+55,
            seed+56, seed+57, seed+58, seed+59, seed+60, seed+61, seed+62, seed+63
        );
        
        __m512i b = _mm512_set_epi8(
            seed+64, seed+65, seed+66, seed+67, seed+68, seed+69, seed+70, seed+71,
            seed+72, seed+73, seed+74, seed+75, seed+76, seed+77, seed+78, seed+79,
            seed+80, seed+81, seed+82, seed+83, seed+84, seed+85, seed+86, seed+87,
            seed+88, seed+89, seed+90, seed+91, seed+92, seed+93, seed+94, seed+95,
            seed+96, seed+97, seed+98, seed+99, seed+100, seed+101, seed+102, seed+103,
            seed+104, seed+105, seed+106, seed+107, seed+108, seed+109, seed+110, seed+111,
            seed+112, seed+113, seed+114, seed+115, seed+116, seed+117, seed+118, seed+119,
            seed+120, seed+121, seed+122, seed+123, seed+124, seed+125, seed+126, seed+127
        );
        
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi8(mask64, a, b);
        
        // Store to volatile memory
        _mm512_storeu_si512((void*)g_store8, res);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    // 2. V32HImode - 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15,
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __m512i b = _mm512_set_epi16(
            seed+32, seed+33, seed+34, seed+35, seed+36, seed+37, seed+38, seed+39,
            seed+40, seed+41, seed+42, seed+43, seed+44, seed+45, seed+46, seed+47,
            seed+48, seed+49, seed+50, seed+51, seed+52, seed+53, seed+54, seed+55,
            seed+56, seed+57, seed+58, seed+59, seed+60, seed+61, seed+62, seed+63
        );
        
        __mmask32 mask32 = 0xAAAAAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi16(mask32, a, b);
        
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    // 3. V32HFmode - 32 x half-precision floats
    {
        _Float16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (_Float16)(seed + i) * 0.5f;
            b_arr[i] = (_Float16)(seed + i + 32) * 0.25f;
        }
        
        __m512h a = _mm512_set_ph(
            a_arr[31], a_arr[30], a_arr[29], a_arr[28], a_arr[27], a_arr[26], a_arr[25], a_arr[24],
            a_arr[23], a_arr[22], a_arr[21], a_arr[20], a_arr[19], a_arr[18], a_arr[17], a_arr[16],
            a_arr[15], a_arr[14], a_arr[13], a_arr[12], a_arr[11], a_arr[10], a_arr[9], a_arr[8],
            a_arr[7], a_arr[6], a_arr[5], a_arr[4], a_arr[3], a_arr[2], a_arr[1], a_arr[0]
        );
        
        __m512h b = _mm512_set_ph(
            b_arr[31], b_arr[30], b_arr[29], b_arr[28], b_arr[27], b_arr[26], b_arr[25], b_arr[24],
            b_arr[23], b_arr[22], b_arr[21], b_arr[20], b_arr[19], b_arr[18], b_arr[17], b_arr[16],
            b_arr[15], b_arr[14], b_arr[13], b_arr[12], b_arr[11], b_arr[10], b_arr[9], b_arr[8],
            b_arr[7], b_arr[6], b_arr[5], b_arr[4], b_arr[3], b_arr[2], b_arr[1], b_arr[0]
        );
        
        __mmask32 mask32 = 0x55555555; // Different pattern
        __m512h res = _mm512_mask_blend_ph(mask32, a, b);
        
        _mm512_storeu_ph((void*)g_storef16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storef16[i];
        }
    }
    
    // 4. V32BFmode - 32 x brain floats (if supported)
#ifdef __bf16
    {
        __bf16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (__bf16)((seed + i) * 0.5f);
            b_arr[i] = (__bf16)((seed + i + 32) * 0.25f);
        }
        
        __m512bh a = _mm512_set_ph(
            a_arr[31], a_arr[30], a_arr[29], a_arr[28], a_arr[27], a_arr[26], a_arr[25], a_arr[24],
            a_arr[23], a_arr[22], a_arr[21], a_arr[20], a_arr[19], a_arr[18], a_arr[17], a_arr[16],
            a_arr[15], a_arr[14], a_arr[13], a_arr[12], a_arr[11], a_arr[10], a_arr[9], a_arr[8],
            a_arr[7], a_arr[6], a_arr[5], a_arr[4], a_arr[3], a_arr[2], a_arr[1], a_arr[0]
        );
        
        __m512bh b = _mm512_set_ph(
            b_arr[31], b_arr[30], b_arr[29], b_arr[28], b_arr[27], b_arr[26], b_arr[25], b_arr[24],
            b_arr[23], b_arr[22], b_arr[21], b_arr[20], b_arr[19], b_arr[18], b_arr[17], b_arr[16],
            b_arr[15], b_arr[14], b_arr[13], b_arr[12], b_arr[11], b_arr[10], b_arr[9], b_arr[8],
            b_arr[7], b_arr[6], b_arr[5], b_arr[4], b_arr[3], b_arr[2], b_arr[1], b_arr[0]
        );
        
        __mmask32 mask32 = 0x33333333; // Different pattern
        __m512bh res = _mm512_mask_blend_ph(mask32, a, b);
        
        _mm512_storeu_ph((void*)g_storebf16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storebf16[i];
        }
    }
#endif
    
    // 5. V16SImode - 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __m512i b = _mm512_set_epi32(
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __mmask16 mask16 = 0xAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi32(mask16, a, b);
        
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    // 6. V8DImode - 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(
            seed+0, seed+1, seed+2, seed+3,
            seed+4, seed+5, seed+6, seed+7
        );
        
        __m512i b = _mm512_set_epi64(
            seed+8, seed+9, seed+10, seed+11,
            seed+12, seed+13, seed+14, seed+15
        );
        
        __mmask8 mask8 = 0xAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi64(mask8, a, b);
        
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    // 7. V16SFmode - 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(
            (seed+0)*0.1f, (seed+1)*0.2f, (seed+2)*0.3f, (seed+3)*0.4f,
            (seed+4)*0.5f, (seed+5)*0.6f, (seed+6)*0.7f, (seed+7)*0.8f,
            (seed+8)*0.9f, (seed+9)*1.0f, (seed+10)*1.1f, (seed+11)*1.2f,
            (seed+12)*1.3f, (seed+13)*1.4f, (seed+14)*1.5f, (seed+15)*1.6f
        );
        
        __m512 b = _mm512_set_ps(
            (seed+16)*0.15f, (seed+17)*0.25f, (seed+18)*0.35f, (seed+19)*0.45f,
            (seed+20)*0.55f, (seed+21)*0.65f, (seed+22)*0.75f, (seed+23)*0.85f,
            (seed+24)*0.95f, (seed+25)*1.05f, (seed+26)*1.15f, (seed+27)*1.25f,
            (seed+28)*1.35f, (seed+29)*1.45f, (seed+30)*1.55f, (seed+31)*1.65f
        );
        
        __mmask16 mask16 = 0x5555; // Different pattern
        __m512 res = _mm512_mask_blend_ps(mask16, a, b);
        
        _mm512_storeu_ps((void*)g_storef32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef32[i];
        }
    }
    
    // 8. V8DFmode - 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(
            (seed+0)*0.01, (seed+1)*0.02, (seed+2)*0.03, (seed+3)*0.04,
            (seed+4)*0.05, (seed+5)*0.06, (seed+6)*0.07, (seed+7)*0.08
        );
        
        __m512d b = _mm512_set_pd(
            (seed+8)*0.09, (seed+9)*0.10, (seed+10)*0.11, (seed+11)*0.12,
            (seed+12)*0.13, (seed+13)*0.14, (seed+14)*0.15, (seed+15)*0.16
        );
        
        __mmask8 mask8 = 0x55; // Different pattern
        __m512d res = _mm512_mask_blend_pd(mask8, a, b);
        
        _mm512_storeu_pd((void*)g_storedf64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_storedf64[i];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    volatile int seed = 42; // Use volatile to prevent constant folding
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
