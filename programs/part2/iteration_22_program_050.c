/* Test program to cover AVX-512 blend expansion lines in i386-expand.cc
 * Compile with: gcc -O2 -march=skylake-avx512 -fdump-rtl-expand
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global volatile arrays to prevent dead code elimination */
volatile int8_t g_store8[64];
volatile int16_t g_store16[32];
volatile int32_t g_store32[16];
volatile int64_t g_store64[8];
volatile float g_storef[16];
volatile double g_stored[8];
volatile _Float16 g_storeh[32];
volatile __bf16 g_storeb[32];

/* Mark function with required AVX-512 features */
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    /* V64QImode: 64 x 8-bit integers */
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
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
        __m512i res = _mm512_mask_blend_epi8(mask64, a, b);
        _mm512_storeu_si512((void*)g_store8, res);
        
        /* Accumulate some elements for checksum */
        checksum += g_store8[0] + g_store8[63];
    }
    
    /* V32HImode: 32 x 16-bit integers */
    {
        __m512i a = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        __m512i b = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask32 mask32 = 0xAAAAAAAA;
        __m512i res = _mm512_mask_blend_epi16(mask32, a, b);
        _mm512_storeu_si512((void*)g_store16, res);
        
        checksum += g_store16[0] + g_store16[31];
    }
    
    /* V32HFmode: 32 x half-precision floats */
    {
        _Float16 ha[32], hb[32];
        for (int i = 0; i < 32; i++) {
            ha[i] = (_Float16)(i + seed);
            hb[i] = (_Float16)(31 - i + seed);
        }
        __m512h a = _mm512_loadu_ph(ha);
        __m512h b = _mm512_loadu_ph(hb);
        __mmask32 mask32 = 0x55555555;
        __m512h res = _mm512_mask_blend_ph(mask32, a, b);
        _mm512_storeu_ph((void*)g_storeh, res);
        
        checksum += (int)g_storeh[0] + (int)g_storeh[31];
    }
    
    /* V32BFmode: 32 x brain floats (if supported) */
#ifdef __AVX512BF16__
    {
        __m512bh a = _mm512_set1_epi16(0x3F80); /* 1.0 in bfloat16 */
        __m512bh b = _mm512_set1_epi16(0x4000); /* 2.0 in bfloat16 */
        __mmask32 mask32 = 0xAAAAAAAA;
        __m512bh res = _mm512_mask_blend_ph(mask32, a, b);
        _mm512_storeu_epi16((void*)g_storeb, (__m512i)res);
        
        checksum += g_storeb[0] + g_storeb[31];
    }
#endif
    
    /* V16SImode: 16 x 32-bit integers */
    {
        __m512i a = _mm512_set_epi32(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        );
        __m512i b = _mm512_set_epi32(
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __mmask16 mask16 = 0xAAAA;
        __m512i res = _mm512_mask_blend_epi32(mask16, a, b);
        _mm512_storeu_si512((void*)g_store32, res);
        
        checksum += g_store32[0] + g_store32[15];
    }
    
    /* V8DImode: 8 x 64-bit integers */
    {
        __m512i a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        __m512i b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __mmask8 mask8 = 0xAA;
        __m512i res = _mm512_mask_blend_epi64(mask8, a, b);
        _mm512_storeu_si512((void*)g_store64, res);
        
        checksum += (int)(g_store64[0] + g_store64[7]);
    }
    
    /* V16SFmode: 16 x single-precision floats */
    {
        __m512 a = _mm512_set_ps(
            0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
            8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
        );
        __m512 b = _mm512_set_ps(
            15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
            7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
        );
        __mmask16 mask16 = 0x5555;
        __m512 res = _mm512_mask_blend_ps(mask16, a, b);
        _mm512_storeu_ps((void*)g_storef, res);
        
        checksum += (int)g_storef[0] + (int)g_storef[15];
    }
    
    /* V8DFmode: 8 x double-precision floats */
    {
        __m512d a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        __m512d b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __mmask8 mask8 = 0x55;
        __m512d res = _mm512_mask_blend_pd(mask8, a, b);
        _mm512_storeu_pd((void*)g_stored, res);
        
        checksum += (int)g_stored[0] + (int)g_stored[7];
    }
    
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
