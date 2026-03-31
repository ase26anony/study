/* Test program for AVX-512 blend expansion coverage in i386-expand.cc
 * Compile with: gcc -O2 -march=skylake-avx512 -fdump-rtl-expand avx512_blend_test.c
 */

#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef[16];
volatile double g_stored[8];
volatile uint16_t g_storehf[32];  /* For half-precision */
volatile uint16_t g_storebf[32];  /* For brain float */

/* Main test function with all blend operations */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* ===== V64QImode (64 x 8-bit integers) ===== */
    {
        /* Create non-uniform patterns using seed */
        __m512i a64qi = _mm512_set_epi8(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15,
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31,
            seed+32, seed+33, seed+34, seed+35, seed+36, seed+37, seed+38, seed+39,
            seed+40, seed+41, seed+42, seed+43, seed+44, seed+45, seed+46, seed+47,
            seed+48, seed+49, seed+50, seed+51, seed+52, seed+53, seed+54, seed+55,
            seed+56, seed+57, seed+58, seed+59, seed+60, seed+61, seed+62, seed+63
        );
        
        __m512i b64qi = _mm512_set_epi8(
            seed+64, seed+65, seed+66, seed+67, seed+68, seed+69, seed+70, seed+71,
            seed+72, seed+73, seed+74, seed+75, seed+76, seed+77, seed+78, seed+79,
            seed+80, seed+81, seed+82, seed+83, seed+84, seed+85, seed+86, seed+87,
            seed+88, seed+89, seed+90, seed+91, seed+92, seed+93, seed+94, seed+95,
            seed+96, seed+97, seed+98, seed+99, seed+100, seed+101, seed+102, seed+103,
            seed+104, seed+105, seed+106, seed+107, seed+108, seed+109, seed+110, seed+111,
            seed+112, seed+113, seed+114, seed+115, seed+116, seed+117, seed+118, seed+119,
            seed+120, seed+121, seed+122, seed+123, seed+124, seed+125, seed+126, seed+127
        );
        
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAA;  /* Alternating pattern */
        __m512i res64qi = _mm512_mask_blend_epi8(mask64, a64qi, b64qi);
        
        /* Store to volatile memory to prevent elimination */
        _mm512_storeu_si512((void*)g_store8, res64qi);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_store8[i];
        }
    }
    
    /* ===== V32HImode (32 x 16-bit integers) ===== */
    {
        __m512i a32hi = _mm512_set_epi16(
            seed+0, seed+10, seed+20, seed+30, seed+40, seed+50, seed+60, seed+70,
            seed+80, seed+90, seed+100, seed+110, seed+120, seed+130, seed+140, seed+150,
            seed+160, seed+170, seed+180, seed+190, seed+200, seed+210, seed+220, seed+230,
            seed+240, seed+250, seed+260, seed+270, seed+280, seed+290, seed+300, seed+310
        );
        
        __m512i b32hi = _mm512_set_epi16(
            seed+1000, seed+1010, seed+1020, seed+1030, seed+1040, seed+1050, seed+1060, seed+1070,
            seed+1080, seed+1090, seed+1100, seed+1110, seed+1120, seed+1130, seed+1140, seed+1150,
            seed+1160, seed+1170, seed+1180, seed+1190, seed+1200, seed+1210, seed+1220, seed+1230,
            seed+1240, seed+1250, seed+1260, seed+1270, seed+1280, seed+1290, seed+1300, seed+1310
        );
        
        __mmask32 mask32 = 0xAAAAAAAA;  /* Alternating pattern */
        __m512i res32hi = _mm512_mask_blend_epi16(mask32, a32hi, b32hi);
        
        _mm512_storeu_si512((void*)g_store16, res32hi);
        for (int i = 0; i < 32; i++) {
            checksum += g_store16[i];
        }
    }
    
    /* ===== V16SImode (16 x 32-bit integers) ===== */
    {
        __m512i a16si = _mm512_set_epi32(
            seed+0, seed+100, seed+200, seed+300, seed+400, seed+500, seed+600, seed+700,
            seed+800, seed+900, seed+1000, seed+1100, seed+1200, seed+1300, seed+1400, seed+1500
        );
        
        __m512i b16si = _mm512_set_epi32(
            seed+10000, seed+10100, seed+10200, seed+10300, seed+10400, seed+10500, seed+10600, seed+10700,
            seed+10800, seed+10900, seed+11000, seed+11100, seed+11200, seed+11300, seed+11400, seed+11500
        );
        
        __mmask16 mask16 = 0xAAAA;  /* Alternating pattern */
        __m512i res16si = _mm512_mask_blend_epi32(mask16, a16si, b16si);
        
        _mm512_storeu_si512((void*)g_store32, res16si);
        for (int i = 0; i < 16; i++) {
            checksum += g_store32[i];
        }
    }
    
    /* ===== V8DImode (8 x 64-bit integers) ===== */
    {
        __m512i a8di = _mm512_set_epi64(
            seed+0, seed+1000, seed+2000, seed+3000,
            seed+4000, seed+5000, seed+6000, seed+7000
        );
        
        __m512i b8di = _mm512_set_epi64(
            seed+100000, seed+101000, seed+102000, seed+103000,
            seed+104000, seed+105000, seed+106000, seed+107000
        );
        
        __mmask8 mask8 = 0xAA;  /* Alternating pattern */
        __m512i res8di = _mm512_mask_blend_epi64(mask8, a8di, b8di);
        
        _mm512_storeu_si512((void*)g_store64, res8di);
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_store64[i];
        }
    }
    
    /* ===== V16SFmode (16 x single-precision floats) ===== */
    {
        __m512 a16sf = _mm512_set_ps(
            seed+0.1f, seed+1.1f, seed+2.1f, seed+3.1f, seed+4.1f, seed+5.1f, seed+6.1f, seed+7.1f,
            seed+8.1f, seed+9.1f, seed+10.1f, seed+11.1f, seed+12.1f, seed+13.1f, seed+14.1f, seed+15.1f
        );
        
        __m512 b16sf = _mm512_set_ps(
            seed+100.1f, seed+101.1f, seed+102.1f, seed+103.1f, seed+104.1f, seed+105.1f, seed+106.1f, seed+107.1f,
            seed+108.1f, seed+109.1f, seed+110.1f, seed+111.1f, seed+112.1f, seed+113.1f, seed+114.1f, seed+115.1f
        );
        
        __mmask16 mask16f = 0x5555;  /* Different pattern from integer case */
        __m512 res16sf = _mm512_mask_blend_ps(mask16f, a16sf, b16sf);
        
        _mm512_storeu_ps((void*)g_storef, res16sf);
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_storef[i];
        }
    }
    
    /* ===== V8DFmode (8 x double-precision floats) ===== */
    {
        __m512d a8df = _mm512_set_pd(
            seed+0.01, seed+1.01, seed+2.01, seed+3.01,
            seed+4.01, seed+5.01, seed+6.01, seed+7.01
        );
        
        __m512d b8df = _mm512_set_pd(
            seed+100.01, seed+101.01, seed+102.01, seed+103.01,
            seed+104.01, seed+105.01, seed+106.01, seed+107.01
        );
        
        __mmask8 mask8d = 0x55;  /* Different pattern */
        __m512d res8df = _mm512_mask_blend_pd(mask8d, a8df, b8df);
        
        _mm512_storeu_pd((void*)g_stored, res8df);
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_stored[i];
        }
    }
    
    /* ===== V32HFmode (32 x half-precision floats) ===== */
    {
        /* Note: _mm512_mask_blend_ph requires AVX512-FP16 */
        #ifdef __AVX512FP16__
        __m512h a32hf = _mm512_set_ph(
            seed+0.1f, seed+0.2f, seed+0.3f, seed+0.4f, seed+0.5f, seed+0.6f, seed+0.7f, seed+0.8f,
            seed+0.9f, seed+1.0f, seed+1.1f, seed+1.2f, seed+1.3f, seed+1.4f, seed+1.5f, seed+1.6f,
            seed+1.7f, seed+1.8f, seed+1.9f, seed+2.0f, seed+2.1f, seed+2.2f, seed+2.3f, seed+2.4f,
            seed+2.5f, seed+2.6f, seed+2.7f, seed+2.8f, seed+2.9f, seed+3.0f, seed+3.1f, seed+3.2f
        );
        
        __m512h b32hf = _mm512_set_ph(
            seed+10.1f, seed+10.2f, seed+10.3f, seed+10.4f, seed+10.5f, seed+10.6f, seed+10.7f, seed+10.8f,
            seed+10.9f, seed+11.0f, seed+11.1f, seed+11.2f, seed+11.3f, seed+11.4f, seed+11.5f, seed+11.6f,
            seed+11.7f, seed+11.8f, seed+11.9f, seed+12.0f, seed+12.1f, seed+12.2f, seed+12.3f, seed+12.4f,
            seed+12.5f, seed+12.6f, seed+12.7f, seed+12.8f, seed+12.9f, seed+13.0f, seed+13.1f, seed+13.2f
        );
        
        __mmask32 mask32hf = 0x55555555;  /* Different pattern */
        __m512h res32hf = _mm512_mask_blend_ph(mask32hf, a32hf, b32hf);
        
        _mm512_storeu_ph((void*)g_storehf, res32hf);
        for (int i = 0; i < 32; i++) {
            checksum += g_storehf[i];
        }
        #endif
    }
    
    /* ===== V32BFmode (32 x brain floats) ===== */
    {
        /* Note: Brain float (bfloat16) uses same intrinsics as half-precision */
        #ifdef __AVX512BF16__
        __m512bh a32bf = _mm512_set_ph(  /* Using same set function */
            seed+0.1f, seed+0.2f, seed+0.3f, seed+0.4f, seed+0.5f, seed+0.6f, seed+0.7f, seed+0.8f,
            seed+0.9f, seed+1.0f, seed+1.1f, seed+1.2f, seed+1.3f, seed+1.4f, seed+1.5f, seed+1.6f,
            seed+1.7f, seed+1.8f, seed+1.9f, seed+2.0f, seed+2.1f, seed+2.2f, seed+2.3f, seed+2.4f,
            seed+2.5f, seed+2.6f, seed+2.7f, seed+2.8f, seed+2.9f, seed+3.0f, seed+3.1f, seed+3.2f
        );
        
        __m512bh b32bf = _mm512_set_ph(
            seed+10.1f, seed+10.2f, seed+10.3f, seed+10.4f, seed+10.5f, seed+10.6f, seed+10.7f, seed+10.8f,
            seed+10.9f, seed+11.0f, seed+11.1f, seed+11.2f, seed+11.3f, seed+11.4f, seed+11.5f, seed+11.6f,
            seed+11.7f, seed+11.8f, seed+11.9f, seed+12.0f, seed+12.1f, seed+12.2f, seed+12.3f, seed+12.4f,
            seed+12.5f, seed+12.6f, seed+12.7f, seed+12.8f, seed+12.9f, seed+13.0f, seed+13.1f, seed+13.2f
        );
        
        __mmask32 mask32bf = 0xAAAAAAAA;  /* Different pattern */
        __m512bh res32bf = _mm512_mask_blend_ph(mask32bf, a32bf, b32bf);
        
        _mm512_storeu_ph((void*)g_storebf, res32bf);
        for (int i = 0; i < 32; i++) {
            checksum += g_storebf[i];
        }
        #endif
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    volatile int seed = 42;  /* Use volatile to prevent constant propagation */
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    
    printf("Blend test checksum: %d\n", result);
    
    return 0;
}
