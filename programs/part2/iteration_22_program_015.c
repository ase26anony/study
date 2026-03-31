/* Test program to cover AVX-512 blend expansion lines in i386-expand.cc */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Volatile arrays to prevent dead code elimination */
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

/* Noinline function with all AVX-512 blend operations */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* V64QImode - 64 x 8-bit integers */
    {
        __m512i a = _mm512_set_epi8(
            seed+63, seed+62, seed+61, seed+60, seed+59, seed+58, seed+57, seed+56,
            seed+55, seed+54, seed+53, seed+52, seed+51, seed+50, seed+49, seed+48,
            seed+47, seed+46, seed+45, seed+44, seed+43, seed+42, seed+41, seed+40,
            seed+39, seed+38, seed+37, seed+36, seed+35, seed+34, seed+33, seed+32,
            seed+31, seed+30, seed+29, seed+28, seed+27, seed+26, seed+25, seed+24,
            seed+23, seed+22, seed+21, seed+20, seed+19, seed+18, seed+17, seed+16,
            seed+15, seed+14, seed+13, seed+12, seed+11, seed+10, seed+9,  seed+8,
            seed+7,  seed+6,  seed+5,  seed+4,  seed+3,  seed+2,  seed+1,  seed+0
        );
        
        __m512i b = _mm512_set_epi8(
            seed+127, seed+126, seed+125, seed+124, seed+123, seed+122, seed+121, seed+120,
            seed+119, seed+118, seed+117, seed+116, seed+115, seed+114, seed+113, seed+112,
            seed+111, seed+110, seed+109, seed+108, seed+107, seed+106, seed+105, seed+104,
            seed+103, seed+102, seed+101, seed+100, seed+99,  seed+98,  seed+97,  seed+96,
            seed+95,  seed+94,  seed+93,  seed+92,  seed+91,  seed+90,  seed+89,  seed+88,
            seed+87,  seed+86,  seed+85,  seed+84,  seed+83,  seed+82,  seed+81,  seed+80,
            seed+79,  seed+78,  seed+77,  seed+76,  seed+75,  seed+74,  seed+73,  seed+72,
            seed+71,  seed+70,  seed+69,  seed+68,  seed+67,  seed+66,  seed+65,  seed+64
        );
        
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi8(mask64, a, b);
        
        /* Store to volatile array to prevent elimination */
        _mm512_storeu_si512((void*)g_store8, res);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    /* V32HImode - 32 x 16-bit integers */
    {
        __m512i a = _mm512_set_epi16(
            seed+31, seed+30, seed+29, seed+28, seed+27, seed+26, seed+25, seed+24,
            seed+23, seed+22, seed+21, seed+20, seed+19, seed+18, seed+17, seed+16,
            seed+15, seed+14, seed+13, seed+12, seed+11, seed+10, seed+9,  seed+8,
            seed+7,  seed+6,  seed+5,  seed+4,  seed+3,  seed+2,  seed+1,  seed+0
        );
        
        __m512i b = _mm512_set_epi16(
            seed+63, seed+62, seed+61, seed+60, seed+59, seed+58, seed+57, seed+56,
            seed+55, seed+54, seed+53, seed+52, seed+51, seed+50, seed+49, seed+48,
            seed+47, seed+46, seed+45, seed+44, seed+43, seed+42, seed+41, seed+40,
            seed+39, seed+38, seed+37, seed+36, seed+35, seed+34, seed+33, seed+32
        );
        
        __mmask32 mask32 = 0xAAAAAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi16(mask32, a, b);
        
        _mm512_storeu_si512((void*)g_store16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    /* V16SImode - 16 x 32-bit integers */
    {
        __m512i a = _mm512_set_epi32(
            seed+15, seed+14, seed+13, seed+12, seed+11, seed+10, seed+9, seed+8,
            seed+7, seed+6, seed+5, seed+4, seed+3, seed+2, seed+1, seed+0
        );
        
        __m512i b = _mm512_set_epi32(
            seed+31, seed+30, seed+29, seed+28, seed+27, seed+26, seed+25, seed+24,
            seed+23, seed+22, seed+21, seed+20, seed+19, seed+18, seed+17, seed+16
        );
        
        __mmask16 mask16 = 0xAAAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi32(mask16, a, b);
        
        _mm512_storeu_si512((void*)g_store32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    /* V8DImode - 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(
            seed+7, seed+6, seed+5, seed+4, seed+3, seed+2, seed+1, seed+0
        );
        
        __m512i b = _mm512_set_epi64(
            seed+15, seed+14, seed+13, seed+12, seed+11, seed+10, seed+9, seed+8
        );
        
        __mmask8 mask8 = 0xAA; /* Alternating pattern */
        __m512i res = _mm512_mask_blend_epi64(mask8, a, b);
        
        _mm512_storeu_si512((void*)g_store64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    /* V16SFmode - 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            seed+15.0f, seed+14.0f, seed+13.0f, seed+12.0f,
            seed+11.0f, seed+10.0f, seed+9.0f,  seed+8.0f,
            seed+7.0f,  seed+6.0f,  seed+5.0f,  seed+4.0f,
            seed+3.0f,  seed+2.0f,  seed+1.0f,  seed+0.0f
        );
        
        __m512 b = _mm512_set_ps(
            seed+31.0f, seed+30.0f, seed+29.0f, seed+28.0f,
            seed+27.0f, seed+26.0f, seed+25.0f, seed+24.0f,
            seed+23.0f, seed+22.0f, seed+21.0f, seed+20.0f,
            seed+19.0f, seed+18.0f, seed+17.0f, seed+16.0f
        );
        
        __mmask16 mask16 = 0xAAAA; /* Alternating pattern */
        __m512 res = _mm512_mask_blend_ps(mask16, a, b);
        
        _mm512_storeu_ps((void*)g_storef32, res);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef32[i];
        }
    }
    
    /* V8DFmode - 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(
            seed+7.0, seed+6.0, seed+5.0, seed+4.0,
            seed+3.0, seed+2.0, seed+1.0, seed+0.0
        );
        
        __m512d b = _mm512_set_pd(
            seed+15.0, seed+14.0, seed+13.0, seed+12.0,
            seed+11.0, seed+10.0, seed+9.0,  seed+8.0
        );
        
        __mmask8 mask8 = 0xAA; /* Alternating pattern */
        __m512d res = _mm512_mask_blend_pd(mask8, a, b);
        
        _mm512_storeu_pd((void*)g_storedf64, res);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_storedf64[i];
        }
    }
    
    /* V32HFmode - 32 x half-precision floats (if supported) */
#ifdef __AVX512FP16__
    {
        _Float16 a_vals[32], b_vals[32];
        for (int i = 0; i < 32; i++) {
            a_vals[i] = (seed + i) * 0.5f;
            b_vals[i] = (seed + 32 + i) * 0.5f;
        }
        
        __m512h a = _mm512_set_ph(
            a_vals[31], a_vals[30], a_vals[29], a_vals[28],
            a_vals[27], a_vals[26], a_vals[25], a_vals[24],
            a_vals[23], a_vals[22], a_vals[21], a_vals[20],
            a_vals[19], a_vals[18], a_vals[17], a_vals[16],
            a_vals[15], a_vals[14], a_vals[13], a_vals[12],
            a_vals[11], a_vals[10], a_vals[9],  a_vals[8],
            a_vals[7],  a_vals[6],  a_vals[5],  a_vals[4],
            a_vals[3],  a_vals[2],  a_vals[1],  a_vals[0]
        );
        
        __m512h b = _mm512_set_ph(
            b_vals[31], b_vals[30], b_vals[29], b_vals[28],
            b_vals[27], b_vals[26], b_vals[25], b_vals[24],
            b_vals[23], b_vals[22], b_vals[21], b_vals[20],
            b_vals[19], b_vals[18], b_vals[17], b_vals[16],
            b_vals[15], b_vals[14], b_vals[13], b_vals[12],
            b_vals[11], b_vals[10], b_vals[9],  b_vals[8],
            b_vals[7],  b_vals[6],  b_vals[5],  b_vals[4],
            b_vals[3],  b_vals[2],  b_vals[1],  b_vals[0]
        );
        
        __mmask32 mask32 = 0xAAAAAAAA; /* Alternating pattern */
        __m512h res = _mm512_mask_blend_ph(mask32, a, b);
        
        _mm512_storeu_ph((void*)g_storef16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_storef16[i];
        }
    }
#endif
    
    /* V32BFmode - 32 x brain floats (if supported) */
#ifdef __AVX512BF16__
    {
        __m512bh a = _mm512_set1_epi16(seed);
        __m512bh b = _mm512_set1_epi16(seed + 256);
        
        __mmask32 mask32 = 0xAAAAAAAA; /* Alternating pattern */
        __m512bh res = _mm512_mask_blend_epi32(mask32, a, b);
        
        _mm512_storeu_si512((void*)g_storebf16, res);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_storebf16[i];
        }
    }
#endif
    
    return checksum;
}

int main(int argc, char *argv[]) {
    volatile int seed = 42; /* Use volatile to prevent constant folding */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
