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
    
    // 1. V64QImode: 64 x 8-bit integers
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
        
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Store to volatile memory to prevent elimination
        _mm512_storeu_si512((void*)g_store8, res);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    // 2. V32HImode: 32 x 16-bit integers
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
        
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    // 3. V32HFmode: 32 x half-precision floats
    {
        _Float16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (seed + i) * 0.1f;
            b_vals[i] = (seed + 32 + i) * 0.2f;
        }
        
        __m512h a = _mm512_set_ph(
            a_vals[0], a_vals[1], a_vals[2], a_vals[3], a_vals[4], a_vals[5], a_vals[6], a_vals[7],
            a_vals[8], a_vals[9], a_vals[10], a_vals[11], a_vals[12], a_vals[13], a_vals[14], a_vals[15],
            a_vals[16], a_vals[17], a_vals[18], a_vals[19], a_vals[20], a_vals[21], a_vals[22], a_vals[23],
            a_vals[24], a_vals[25], a_vals[26], a_vals[27], a_vals[28], a_vals[29], a_vals[30], a_vals[31]
        );
        
        __m512h b = _mm512_set_ph(
            b_vals[0], b_vals[1], b_vals[2], b_vals[3], b_vals[4], b_vals[5], b_vals[6], b_vals[7],
            b_vals[8], b_vals[9], b_vals[10], b_vals[11], b_vals[12], b_vals[13], b_vals[14], b_vals[15],
            b_vals[16], b_vals[17], b_vals[18], b_vals[19], b_vals[20], b_vals[21], b_vals[22], b_vals[23],
            b_vals[24], b_vals[25], b_vals[26], b_vals[27], b_vals[28], b_vals[29], b_vals[30], b_vals[31]
        );
        
        __mmask32 mask = 0x55555555; // Complementary pattern
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        _mm512_storeu_ph((void*)g_storeh, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storeh[i];
        }
    }
    
#ifdef __bf16
    // 4. V32BFmode: 32 x brain floats (if supported)
    {
        __bf16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (seed + i) * 0.1f;
            b_vals[i] = (seed + 32 + i) * 0.2f;
        }
        
        __m512bh a = _mm512_set_ph(
            a_vals[0], a_vals[1], a_vals[2], a_vals[3], a_vals[4], a_vals[5], a_vals[6], a_vals[7],
            a_vals[8], a_vals[9], a_vals[10], a_vals[11], a_vals[12], a_vals[13], a_vals[14], a_vals[15],
            a_vals[16], a_vals[17], a_vals[18], a_vals[19], a_vals[20], a_vals[21], a_vals[22], a_vals[23],
            a_vals[24], a_vals[25], a_vals[26], a_vals[27], a_vals[28], a_vals[29], a_vals[30], a_vals[31]
        );
        
        __m512bh b = _mm512_set_ph(
            b_vals[0], b_vals[1], b_vals[2], b_vals[3], b_vals[4], b_vals[5], b_vals[6], b_vals[7],
            b_vals[8], b_vals[9], b_vals[10], b_vals[11], b_vals[12], b_vals[13], b_vals[14], b_vals[15],
            b_vals[16], b_vals[17], b_vals[18], b_vals[19], b_vals[20], b_vals[21], b_vals[22], b_vals[23],
            b_vals[24], b_vals[25], b_vals[26], b_vals[27], b_vals[28], b_vals[29], b_vals[30], b_vals[31]
        );
        
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        _mm512_storeu_ph((void*)g_storeb, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storeb[i];
        }
    }
#endif
    
    // 5. V16SImode: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __m512i b = _mm512_set_epi32(
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __mmask16 mask = 0xAAAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    // 6. V8DImode: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(
            seed+0, seed+1, seed+2, seed+3,
            seed+4, seed+5, seed+6, seed+7
        );
        
        __m512i b = _mm512_set_epi64(
            seed+8, seed+9, seed+10, seed+11,
            seed+12, seed+13, seed+14, seed+15
        );
        
        __mmask8 mask = 0xAA; // Alternating pattern
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    // 7. V16SFmode: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(
            (seed+0)*0.1f, (seed+1)*0.2f, (seed+2)*0.3f, (seed+3)*0.4f,
            (seed+4)*0.5f, (seed+5)*0.6f, (seed+6)*0.7f, (seed+7)*0.8f,
            (seed+8)*0.9f, (seed+9)*1.0f, (seed+10)*1.1f, (seed+11)*1.2f,
            (seed+12)*1.3f, (seed+13)*1.4f, (seed+14)*1.5f, (seed+15)*1.6f
        );
        
        __m512 b = _mm512_set_ps(
            (seed+16)*0.2f, (seed+17)*0.3f, (seed+18)*0.4f, (seed+19)*0.5f,
            (seed+20)*0.6f, (seed+21)*0.7f, (seed+22)*0.8f, (seed+23)*0.9f,
            (seed+24)*1.0f, (seed+25)*1.1f, (seed+26)*1.2f, (seed+27)*1.3f,
            (seed+28)*1.4f, (seed+29)*1.5f, (seed+30)*1.6f, (seed+31)*1.7f
        );
        
        __mmask16 mask = 0x5555; // Complementary pattern
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        _mm512_storeu_ps((void*)g_storef, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef[i];
        }
    }
    
    // 8. V8DFmode: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(
            (seed+0)*0.01, (seed+1)*0.02, (seed+2)*0.03, (seed+3)*0.04,
            (seed+4)*0.05, (seed+5)*0.06, (seed+6)*0.07, (seed+7)*0.08
        );
        
        __m512d b = _mm512_set_pd(
            (seed+8)*0.02, (seed+9)*0.03, (seed+10)*0.04, (seed+11)*0.05,
            (seed+12)*0.06, (seed+13)*0.07, (seed+14)*0.08, (seed+15)*0.09
        );
        
        __mmask8 mask = 0x55; // Complementary pattern
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        _mm512_storeu_pd((void*)g_stored, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_stored[i];
        }
    }
    
    return checksum;
}

int main(int argc, char* argv[]) {
    // Use command line argument or fixed seed
    volatile int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Call the complex blend test function
    int result = test_avx512_blend(seed);
    
    // Print result to prevent dead code elimination
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
