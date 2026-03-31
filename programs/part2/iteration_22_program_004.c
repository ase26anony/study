/* Test program to cover AVX-512 blend expansion lines in i386-expand.cc */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int64_t g_store_i64[8];
volatile int32_t g_store_i32[16];
volatile int16_t g_store_i16[32];
volatile int8_t g_store_i8[64];
volatile float g_store_f32[16];
volatile double g_store_f64[8];
volatile uint16_t g_store_f16[32];

/* Main test function with all blend operations */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* V64QImode: 64 x 8-bit integers */
    {
        __m512i a = _mm512_set_epi8(
            0xF0 + seed, 0xE0 + seed, 0xD0 + seed, 0xC0 + seed,
            0xB0 + seed, 0xA0 + seed, 0x90 + seed, 0x80 + seed,
            0x70 + seed, 0x60 + seed, 0x50 + seed, 0x40 + seed,
            0x30 + seed, 0x20 + seed, 0x10 + seed, 0x00 + seed,
            0xF1 + seed, 0xE1 + seed, 0xD1 + seed, 0xC1 + seed,
            0xB1 + seed, 0xA1 + seed, 0x91 + seed, 0x81 + seed,
            0x71 + seed, 0x61 + seed, 0x51 + seed, 0x41 + seed,
            0x31 + seed, 0x21 + seed, 0x11 + seed, 0x01 + seed,
            0xF2 + seed, 0xE2 + seed, 0xD2 + seed, 0xC2 + seed,
            0xB2 + seed, 0xA2 + seed, 0x92 + seed, 0x82 + seed,
            0x72 + seed, 0x62 + seed, 0x52 + seed, 0x42 + seed,
            0x32 + seed, 0x22 + seed, 0x12 + seed, 0x02 + seed,
            0xF3 + seed, 0xE3 + seed, 0xD3 + seed, 0xC3 + seed,
            0xB3 + seed, 0xA3 + seed, 0x93 + seed, 0x83 + seed,
            0x73 + seed, 0x63 + seed, 0x53 + seed, 0x43 + seed,
            0x33 + seed, 0x23 + seed, 0x13 + seed, 0x03 + seed
        );
        
        __m512i b = _mm512_set_epi8(
            0x0F - seed, 0x1E - seed, 0x2D - seed, 0x3C - seed,
            0x4B - seed, 0x5A - seed, 0x69 - seed, 0x78 - seed,
            0x87 - seed, 0x96 - seed, 0xA5 - seed, 0xB4 - seed,
            0xC3 - seed, 0xD2 - seed, 0xE1 - seed, 0xF0 - seed,
            0x0E - seed, 0x1D - seed, 0x2C - seed, 0x3B - seed,
            0x4A - seed, 0x59 - seed, 0x68 - seed, 0x77 - seed,
            0x86 - seed, 0x95 - seed, 0xA4 - seed, 0xB3 - seed,
            0xC2 - seed, 0xD1 - seed, 0xE0 - seed, 0xEF - seed,
            0x0D - seed, 0x1C - seed, 0x2B - seed, 0x3A - seed,
            0x49 - seed, 0x58 - seed, 0x67 - seed, 0x76 - seed,
            0x85 - seed, 0x94 - seed, 0xA3 - seed, 0xB2 - seed,
            0xC1 - seed, 0xD0 - seed, 0xDF - seed, 0xEE - seed,
            0x0C - seed, 0x1B - seed, 0x2A - seed, 0x39 - seed,
            0x48 - seed, 0x57 - seed, 0x66 - seed, 0x75 - seed,
            0x84 - seed, 0x93 - seed, 0xA2 - seed, 0xB1 - seed,
            0xC0 - seed, 0xCF - seed, 0xDE - seed, 0xED - seed
        );
        
        __mmask64 mask = 0xAAAAAAAAAAAAAAAAULL;
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_epi8((void*)g_store_i8, res);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_store_i8[i];
        }
    }
    
    /* V32HImode: 32 x 16-bit integers */
    {
        __m512i a = _mm512_set_epi16(
            0x1000 + seed, 0x2000 + seed, 0x3000 + seed, 0x4000 + seed,
            0x5000 + seed, 0x6000 + seed, 0x7000 + seed, 0x8000 + seed,
            0x9000 + seed, 0xA000 + seed, 0xB000 + seed, 0xC000 + seed,
            0xD000 + seed, 0xE000 + seed, 0xF000 + seed, 0x0000 + seed,
            0x1100 + seed, 0x2200 + seed, 0x3300 + seed, 0x4400 + seed,
            0x5500 + seed, 0x6600 + seed, 0x7700 + seed, 0x8800 + seed,
            0x9900 + seed, 0xAA00 + seed, 0xBB00 + seed, 0xCC00 + seed,
            0xDD00 + seed, 0xEE00 + seed, 0xFF00 + seed, 0x0010 + seed
        );
        
        __m512i b = _mm512_set_epi16(
            0x0FFF - seed, 0x1EEE - seed, 0x2DDD - seed, 0x3CCC - seed,
            0x4BBB - seed, 0x5AAA - seed, 0x6999 - seed, 0x7888 - seed,
            0x8777 - seed, 0x9666 - seed, 0xA555 - seed, 0xB444 - seed,
            0xC333 - seed, 0xD222 - seed, 0xE111 - seed, 0xF000 - seed,
            0x0EEE - seed, 0x1DDD - seed, 0x2CCC - seed, 0x3BBB - seed,
            0x4AAA - seed, 0x5999 - seed, 0x6888 - seed, 0x7777 - seed,
            0x8666 - seed, 0x9555 - seed, 0xA444 - seed, 0xB333 - seed,
            0xC222 - seed, 0xD111 - seed, 0xE000 - seed, 0xEFFF - seed
        );
        
        __mmask32 mask = 0xAAAAAAAA;
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_epi16((void*)g_store_i16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store_i16[i];
        }
    }
    
    /* V16SImode: 16 x 32-bit integers */
    {
        __m512i a = _mm512_set_epi32(
            0x10000000 + seed, 0x20000000 + seed, 0x30000000 + seed, 0x40000000 + seed,
            0x50000000 + seed, 0x60000000 + seed, 0x70000000 + seed, 0x80000000 + seed,
            0x90000000 + seed, 0xA0000000 + seed, 0xB0000000 + seed, 0xC0000000 + seed,
            0xD0000000 + seed, 0xE0000000 + seed, 0xF0000000 + seed, 0x00000000 + seed
        );
        
        __m512i b = _mm512_set_epi32(
            0x0FFFFFFF - seed, 0x1EEEEEEE - seed, 0x2DDDDDDD - seed, 0x3CCCCCCC - seed,
            0x4BBBBBBB - seed, 0x5AAAAAAA - seed, 0x69999999 - seed, 0x78888888 - seed,
            0x87777777 - seed, 0x96666666 - seed, 0xA5555555 - seed, 0xB4444444 - seed,
            0xC3333333 - seed, 0xD2222222 - seed, 0xE1111111 - seed, 0xF0000000 - seed
        );
        
        __mmask16 mask = 0xAAAA;
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_epi32((void*)g_store_i32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store_i32[i];
        }
    }
    
    /* V8DImode: 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(
            0x1000000000000000LL + seed, 0x2000000000000000LL + seed,
            0x3000000000000000LL + seed, 0x4000000000000000LL + seed,
            0x5000000000000000LL + seed, 0x6000000000000000LL + seed,
            0x7000000000000000LL + seed, 0x8000000000000000LL + seed
        );
        
        __m512i b = _mm512_set_epi64(
            0x0FFFFFFFFFFFFFFFLL - seed, 0x1EEEEEEEEEEEEEEELL - seed,
            0x2DDDDDDDDDDDDDDDLL - seed, 0x3CCCCCCCCCCCCCCCLL - seed,
            0x4BBBBBBBBBBBBBBBLL - seed, 0x5AAAAAAAAAAAAAAALL - seed,
            0x6999999999999999LL - seed, 0x7888888888888888LL - seed
        );
        
        __mmask8 mask = 0xAA;
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_epi64((void*)g_store_i64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)(g_store_i64[i] & 0xFFFFFFFF);
        }
    }
    
    /* V16SFmode: 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            1.0f + seed * 0.1f, 2.0f + seed * 0.1f, 3.0f + seed * 0.1f, 4.0f + seed * 0.1f,
            5.0f + seed * 0.1f, 6.0f + seed * 0.1f, 7.0f + seed * 0.1f, 8.0f + seed * 0.1f,
            9.0f + seed * 0.1f, 10.0f + seed * 0.1f, 11.0f + seed * 0.1f, 12.0f + seed * 0.1f,
            13.0f + seed * 0.1f, 14.0f + seed * 0.1f, 15.0f + seed * 0.1f, 16.0f + seed * 0.1f
        );
        
        __m512 b = _mm512_set_ps(
            16.0f - seed * 0.1f, 15.0f - seed * 0.1f, 14.0f - seed * 0.1f, 13.0f - seed * 0.1f,
            12.0f - seed * 0.1f, 11.0f - seed * 0.1f, 10.0f - seed * 0.1f, 9.0f - seed * 0.1f,
            8.0f - seed * 0.1f, 7.0f - seed * 0.1f, 6.0f - seed * 0.1f, 5.0f - seed * 0.1f,
            4.0f - seed * 0.1f, 3.0f - seed * 0.1f, 2.0f - seed * 0.1f, 1.0f - seed * 0.1f
        );
        
        __mmask16 mask = 0xAAAA;
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps((void*)g_store_f32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_store_f32[i];
        }
    }
    
    /* V8DFmode: 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(
            1.0 + seed * 0.01, 2.0 + seed * 0.01, 3.0 + seed * 0.01, 4.0 + seed * 0.01,
            5.0 + seed * 0.01, 6.0 + seed * 0.01, 7.0 + seed * 0.01, 8.0 + seed * 0.01
        );
        
        __m512d b = _mm512_set_pd(
            8.0 - seed * 0.01, 7.0 - seed * 0.01, 6.0 - seed * 0.01, 5.0 - seed * 0.01,
            4.0 - seed * 0.01, 3.0 - seed * 0.01, 2.0 - seed * 0.01, 1.0 - seed * 0.01
        );
        
        __mmask8 mask = 0xAA;
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd((void*)g_store_f64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store_f64[i];
        }
    }
    
    /* V32HFmode: 32 x half-precision floats (requires AVX512-FP16) */
    #ifdef __AVX512FP16__
    {
        __m512h a = _mm512_set_ph(
            0x3C00 + seed, 0x4000 + seed, 0x4200 + seed, 0x4400 + seed,
            0x4500 + seed, 0x4600 + seed, 0x4700 + seed, 0x4800 + seed,
            0x4880 + seed, 0x4900 + seed, 0x4980 + seed, 0x4A00 + seed,
            0x4A80 + seed, 0x4B00 + seed, 0x4B80 + seed, 0x4C00 + seed,
            0x4C80 + seed, 0x4D00 + seed, 0x4D80 + seed, 0x4E00 + seed,
            0x4E80 + seed, 0x4F00 + seed, 0x4F80 + seed, 0x5000 + seed,
            0x5080 + seed, 0x5100 + seed, 0x5180 + seed, 0x5200 + seed,
            0x5280 + seed, 0x5300 + seed, 0x5380 + seed, 0x5400 + seed
        );
        
        __m512h b = _mm512_set_ph(
            0x5400 - seed, 0x5380 - seed, 0x5300 - seed, 0x5280 - seed,
            0x5200 - seed, 0x5180 - seed, 0x5100 - seed, 0x5080 - seed,
            0x5000 - seed, 0x4F80 - seed, 0x4F00 - seed, 0x4E80 - seed,
            0x4E00 - seed, 0x4D80 - seed, 0x4D00 - seed, 0x4C80 - seed,
            0x4C00 - seed, 0x4B80 - seed, 0x4B00 - seed, 0x4A80 - seed,
            0x4A00 - seed, 0x4980 - seed, 0x4900 - seed, 0x4880 - seed,
            0x4800 - seed, 0x4700 - seed, 0x4600 - seed, 0x4500 - seed,
            0x4400 - seed, 0x4200 - seed, 0x4000 - seed, 0x3C00 - seed
        );
        
        __mmask32 mask = 0xAAAAAAAA;
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph((void*)g_store_f16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store_f16[i];
        }
    }
    #endif
    
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
