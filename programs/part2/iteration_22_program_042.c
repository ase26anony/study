/* Test program to cover AVX-512 blend expansion lines in i386-expand.cc
 * Compile with: gcc -O2 -march=skylake-avx512 -fdump-rtl-expand avx512_blend_test.c
 */

#include <immintrin.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int8_t  g_result_i8[64]  __attribute__((aligned(64)));
volatile int16_t g_result_i16[32] __attribute__((aligned(64)));
volatile int32_t g_result_i32[16] __attribute__((aligned(64)));
volatile int64_t g_result_i64[8]  __attribute__((aligned(64)));
volatile float   g_result_f32[16] __attribute__((aligned(64)));
volatile double  g_result_f64[8]  __attribute__((aligned(64)));
volatile __fp16  g_result_f16[32] __attribute__((aligned(64)));
#ifdef __bf16
volatile __bf16  g_result_bf16[32] __attribute__((aligned(64)));
#endif

/* Complex test function with all blend operations - marked noinline to prevent optimization */
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
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_store_si512((void*)g_result_i8, result);
        
        /* Accumulate checksum */
        for (int i = 0; i < 64; i++) {
            checksum += g_result_i8[i];
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
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_store_si512((void*)g_result_i16, result);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_i16[i];
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
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_store_si512((void*)g_result_i32, result);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_result_i32[i];
        }
    }
    
    /* 4. V8DImode - 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(
            seed+0, seed+1, seed+2, seed+3,
            seed+4, seed+5, seed+6, seed+7
        );
        
        __m512i b = _mm512_set_epi64(
            seed+8, seed+9, seed+10, seed+11,
            seed+12, seed+13, seed+14, seed+15
        );
        
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_store_si512((void*)g_result_i64, result);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_result_i64[i];
        }
    }
    
    /* 5. V16SFmode - 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            seed+0.1f, seed+1.1f, seed+2.1f, seed+3.1f,
            seed+4.1f, seed+5.1f, seed+6.1f, seed+7.1f,
            seed+8.1f, seed+9.1f, seed+10.1f, seed+11.1f,
            seed+12.1f, seed+13.1f, seed+14.1f, seed+15.1f
        );
        
        __m512 b = _mm512_set_ps(
            seed+16.1f, seed+17.1f, seed+18.1f, seed+19.1f,
            seed+20.1f, seed+21.1f, seed+22.1f, seed+23.1f,
            seed+24.1f, seed+25.1f, seed+26.1f, seed+27.1f,
            seed+28.1f, seed+29.1f, seed+30.1f, seed+31.1f
        );
        
        __mmask16 mask = 0xAAAA; /* Alternating pattern */
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        _mm512_store_ps((void*)g_result_f32, result);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_result_f32[i];
        }
    }
    
    /* 6. V8DFmode - 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(
            seed+0.1, seed+1.1, seed+2.1, seed+3.1,
            seed+4.1, seed+5.1, seed+6.1, seed+7.1
        );
        
        __m512d b = _mm512_set_pd(
            seed+8.1, seed+9.1, seed+10.1, seed+11.1,
            seed+12.1, seed+13.1, seed+14.1, seed+15.1
        );
        
        __mmask8 mask = 0xAA; /* Alternating pattern */
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        _mm512_store_pd((void*)g_result_f64, result);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_result_f64[i];
        }
    }
    
    /* 7. V32HFmode - 32 x half-precision floats (if supported) */
#ifdef __AVX512FP16__
    {
        __m512h a = _mm512_set_ph(
            (__fp16)(seed+0.1f), (__fp16)(seed+1.1f), (__fp16)(seed+2.1f), (__fp16)(seed+3.1f),
            (__fp16)(seed+4.1f), (__fp16)(seed+5.1f), (__fp16)(seed+6.1f), (__fp16)(seed+7.1f),
            (__fp16)(seed+8.1f), (__fp16)(seed+9.1f), (__fp16)(seed+10.1f), (__fp16)(seed+11.1f),
            (__fp16)(seed+12.1f), (__fp16)(seed+13.1f), (__fp16)(seed+14.1f), (__fp16)(seed+15.1f),
            (__fp16)(seed+16.1f), (__fp16)(seed+17.1f), (__fp16)(seed+18.1f), (__fp16)(seed+19.1f),
            (__fp16)(seed+20.1f), (__fp16)(seed+21.1f), (__fp16)(seed+22.1f), (__fp16)(seed+23.1f),
            (__fp16)(seed+24.1f), (__fp16)(seed+25.1f), (__fp16)(seed+26.1f), (__fp16)(seed+27.1f),
            (__fp16)(seed+28.1f), (__fp16)(seed+29.1f), (__fp16)(seed+30.1f), (__fp16)(seed+31.1f)
        );
        
        __m512h b = _mm512_set_ph(
            (__fp16)(seed+32.1f), (__fp16)(seed+33.1f), (__fp16)(seed+34.1f), (__fp16)(seed+35.1f),
            (__fp16)(seed+36.1f), (__fp16)(seed+37.1f), (__fp16)(seed+38.1f), (__fp16)(seed+39.1f),
            (__fp16)(seed+40.1f), (__fp16)(seed+41.1f), (__fp16)(seed+42.1f), (__fp16)(seed+43.1f),
            (__fp16)(seed+44.1f), (__fp16)(seed+45.1f), (__fp16)(seed+46.1f), (__fp16)(seed+47.1f),
            (__fp16)(seed+48.1f), (__fp16)(seed+49.1f), (__fp16)(seed+50.1f), (__fp16)(seed+51.1f),
            (__fp16)(seed+52.1f), (__fp16)(seed+53.1f), (__fp16)(seed+54.1f), (__fp16)(seed+55.1f),
            (__fp16)(seed+56.1f), (__fp16)(seed+57.1f), (__fp16)(seed+58.1f), (__fp16)(seed+59.1f),
            (__fp16)(seed+60.1f), (__fp16)(seed+61.1f), (__fp16)(seed+62.1f), (__fp16)(seed+63.1f)
        );
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_store_ph((void*)g_result_f16, result);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_result_f16[i];
        }
    }
#endif
    
    /* 8. V32BFmode - 32 x brain floats (if supported) */
#ifdef __AVX512BF16__
    {
        __m512bh a = _mm512_set_epi16(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15,
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __m512bh b = _mm512_set_epi16(
            seed+32, seed+33, seed+34, seed+35, seed+36, seed+37, seed+38, seed+39,
            seed+40, seed+41, seed+42, seed+43, seed+44, seed+45, seed+46, seed+47,
            seed+48, seed+49, seed+50, seed+51, seed+52, seed+53, seed+54, seed+55,
            seed+56, seed+57, seed+58, seed+59, seed+60, seed+61, seed+62, seed+63
        );
        
        __mmask32 mask = 0xAAAAAAAA; /* Alternating pattern */
        __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_store_si512((void*)g_result_bf16, result);
        
        for (int i = 0; i < 32; i++) {
            checksum += (int)g_result_bf16[i];
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
    
    /* Call the complex blend test function */
    int result = test_avx512_blend(seed);
    
    /* Print result to prevent dead code elimination */
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
