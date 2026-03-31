#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile arrays to prevent dead code elimination
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef32[16];
volatile double g_storef64[8];

// For half-precision and brain float (requires special handling)
#ifdef __AVX512FP16__
volatile _Float16 g_storef16[32];
#endif

__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // V64QImode: 64 x 8-bit integers
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
        
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
        __m512i res = _mm512_mask_blend_epi8(mask64, a, b);
        
        // Store to volatile memory to prevent elimination
        _mm512_storeu_si512((void*)g_store8, res);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    // V32HImode: 32 x 16-bit integers
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
        
        __mmask32 mask32 = 0xAAAAAAAA;
        __m512i res = _mm512_mask_blend_epi16(mask32, a, b);
        
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    // V16SImode: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __m512i b = _mm512_set_epi32(
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __mmask16 mask16 = 0xAAAA;
        __m512i res = _mm512_mask_blend_epi32(mask16, a, b);
        
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    // V8DImode: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7
        );
        
        __m512i b = _mm512_set_epi64(
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __mmask8 mask8 = 0xAA;
        __m512i res = _mm512_mask_blend_epi64(mask8, a, b);
        
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    // V16SFmode: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(
            seed+0.1f, seed+1.1f, seed+2.1f, seed+3.1f, seed+4.1f, seed+5.1f, seed+6.1f, seed+7.1f,
            seed+8.1f, seed+9.1f, seed+10.1f, seed+11.1f, seed+12.1f, seed+13.1f, seed+14.1f, seed+15.1f
        );
        
        __m512 b = _mm512_set_ps(
            seed+16.1f, seed+17.1f, seed+18.1f, seed+19.1f, seed+20.1f, seed+21.1f, seed+22.1f, seed+23.1f,
            seed+24.1f, seed+25.1f, seed+26.1f, seed+27.1f, seed+28.1f, seed+29.1f, seed+30.1f, seed+31.1f
        );
        
        __mmask16 mask16 = 0xAAAA;
        __m512 res = _mm512_mask_blend_ps(mask16, a, b);
        
        _mm512_storeu_ps((void*)g_storef32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef32[i];
        }
    }
    
    // V8DFmode: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(
            seed+0.1, seed+1.1, seed+2.1, seed+3.1, seed+4.1, seed+5.1, seed+6.1, seed+7.1
        );
        
        __m512d b = _mm512_set_pd(
            seed+8.1, seed+9.1, seed+10.1, seed+11.1, seed+12.1, seed+13.1, seed+14.1, seed+15.1
        );
        
        __mmask8 mask8 = 0xAA;
        __m512d res = _mm512_mask_blend_pd(mask8, a, b);
        
        _mm512_storeu_pd((void*)g_storef64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_storef64[i];
        }
    }
    
#ifdef __AVX512FP16__
    // V32HFmode: 32 x half-precision floats (requires AVX512-FP16)
    {
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(seed + i);
            b_data[i] = (_Float16)(seed + i + 32);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        __mmask32 mask32 = 0xAAAAAAAA;
        __m512h res = _mm512_mask_blend_ph(mask32, a, b);
        
        _mm512_storeu_ph((void*)g_storef16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storef16[i];
        }
    }
#endif
    
    return checksum;
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
