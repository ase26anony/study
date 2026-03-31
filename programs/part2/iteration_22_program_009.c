/* Test program for AVX-512 blend expansion coverage in i386-expand.cc */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int64_t g_store64[8] = {0};
volatile int32_t g_store32[16] = {0};
volatile int16_t g_store16[32] = {0};
volatile int8_t g_store8[64] = {0};
volatile float g_storef[16] = {0};
volatile double g_stored[8] = {0};

/* Prevent optimization of the test function */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* 1. V64QImode - 64 x 8-bit integers */
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
        
        __mmask64 mask = 0xAAAAAAAAAAAAAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        /* Store to volatile memory to prevent elimination */
        _mm512_storeu_si512((void*)g_store8, res);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    /* 2. V32HImode - 32 x 16-bit integers */
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
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    /* 3. V16SImode - 16 x 32-bit integers */
    {
        __m512i a = _mm512_set_epi32(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __m512i b = _mm512_set_epi32(
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __mmask16 mask = 0xAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    /* 4. V8DImode - 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7
        );
        
        __m512i b = _mm512_set_epi64(
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    /* 5. V16SFmode - 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            seed+0.1f, seed+1.1f, seed+2.1f, seed+3.1f, seed+4.1f, seed+5.1f, 
            seed+6.1f, seed+7.1f, seed+8.1f, seed+9.1f, seed+10.1f, seed+11.1f,
            seed+12.1f, seed+13.1f, seed+14.1f, seed+15.1f
        );
        
        __m512 b = _mm512_set_ps(
            seed+16.1f, seed+17.1f, seed+18.1f, seed+19.1f, seed+20.1f, seed+21.1f,
            seed+22.1f, seed+23.1f, seed+24.1f, seed+25.1f, seed+26.1f, seed+27.1f,
            seed+28.1f, seed+29.1f, seed+30.1f, seed+31.1f
        );
        
        __mmask16 mask = 0x5555; /* Complementary pattern */
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        _mm512_storeu_ps((void*)g_storef, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef[i];
        }
    }
    
    /* 6. V8DFmode - 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(
            seed+0.1, seed+1.1, seed+2.1, seed+3.1, seed+4.1, seed+5.1, 
            seed+6.1, seed+7.1
        );
        
        __m512d b = _mm512_set_pd(
            seed+8.1, seed+9.1, seed+10.1, seed+11.1, seed+12.1, seed+13.1,
            seed+14.1, seed+15.1
        );
        
        __mmask8 mask = 0x55; /* Complementary pattern */
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        _mm512_storeu_pd((void*)g_stored, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_stored[i];
        }
    }
    
    /* 7. V32HFmode - 32 x half-precision floats (requires AVX-512-FP16) */
    #ifdef __AVX512FP16__
    {
        __m512h a = _mm512_set_ph(
            seed+0.1f, seed+1.1f, seed+2.1f, seed+3.1f, seed+4.1f, seed+5.1f,
            seed+6.1f, seed+7.1f, seed+8.1f, seed+9.1f, seed+10.1f, seed+11.1f,
            seed+12.1f, seed+13.1f, seed+14.1f, seed+15.1f, seed+16.1f, seed+17.1f,
            seed+18.1f, seed+19.1f, seed+20.1f, seed+21.1f, seed+22.1f, seed+23.1f,
            seed+24.1f, seed+25.1f, seed+26.1f, seed+27.1f, seed+28.1f, seed+29.1f,
            seed+30.1f, seed+31.1f
        );
        
        __m512h b = _mm512_set_ph(
            seed+32.1f, seed+33.1f, seed+34.1f, seed+35.1f, seed+36.1f, seed+37.1f,
            seed+38.1f, seed+39.1f, seed+40.1f, seed+41.1f, seed+42.1f, seed+43.1f,
            seed+44.1f, seed+45.1f, seed+46.1f, seed+47.1f, seed+48.1f, seed+49.1f,
            seed+50.1f, seed+51.1f, seed+52.1f, seed+53.1f, seed+54.1f, seed+55.1f,
            seed+56.1f, seed+57.1f, seed+58.1f, seed+59.1f, seed+60.1f, seed+61.1f,
            seed+62.1f, seed+63.1f
        );
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        /* Store and accumulate */
        _Float16 store_h[32];
        _mm512_storeu_ph(store_h, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)store_h[i];
        }
    }
    #endif
    
    /* 8. V32BFmode - 32 x brain floats (requires AVX-512-BF16) */
    #ifdef __AVX512BF16__
    {
        __m512bh a = _mm512_set_ph(
            seed+0.1f, seed+1.1f, seed+2.1f, seed+3.1f, seed+4.1f, seed+5.1f,
            seed+6.1f, seed+7.1f, seed+8.1f, seed+9.1f, seed+10.1f, seed+11.1f,
            seed+12.1f, seed+13.1f, seed+14.1f, seed+15.1f, seed+16.1f, seed+17.1f,
            seed+18.1f, seed+19.1f, seed+20.1f, seed+21.1f, seed+22.1f, seed+23.1f,
            seed+24.1f, seed+25.1f, seed+26.1f, seed+27.1f, seed+28.1f, seed+29.1f,
            seed+30.1f, seed+31.1f
        );
        
        __m512bh b = _mm512_set_ph(
            seed+32.1f, seed+33.1f, seed+34.1f, seed+35.1f, seed+36.1f, seed+37.1f,
            seed+38.1f, seed+39.1f, seed+40.1f, seed+41.1f, seed+42.1f, seed+43.1f,
            seed+44.1f, seed+45.1f, seed+46.1f, seed+47.1f, seed+48.1f, seed+49.1f,
            seed+50.1f, seed+51.1f, seed+52.1f, seed+53.1f, seed+54.1f, seed+55.1f,
            seed+56.1f, seed+57.1f, seed+58.1f, seed+59.1f, seed+60.1f, seed+61.1f,
            seed+62.1f, seed+63.1f
        );
        
        __mmask32 mask = 0x55555555; /* Complementary pattern */
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        /* Store and accumulate */
        __m128bh store_bh[4];
        _mm512_storeu_ph(store_bh, res);
        
        for (int i = 0; i < 4; i++) {
            checksum += ((int*)store_bh)[i];
        }
    }
    #endif
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    
    /* Use command line argument as seed if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call the test function */
    int result = test_avx512_blend(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
