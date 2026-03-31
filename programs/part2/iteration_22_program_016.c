#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile arrays to prevent dead code elimination
volatile int g_checksum = 0;
volatile char g_store[1024];

// Main test function with all blend operations
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // V64QImode: 64 x 8-bit integers
    {
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
        
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAA;
        __m512i res64qi = _mm512_mask_blend_epi8(mask64, a64qi, b64qi);
        
        // Store to volatile memory and accumulate checksum
        _mm512_storeu_si512((void*)g_store, res64qi);
        for (int i = 0; i < 64; i++) {
            checksum += g_store[i];
        }
    }
    
    // V32HImode: 32 x 16-bit integers
    {
        __m512i a32hi = _mm512_set_epi16(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15,
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __m512i b32hi = _mm512_set_epi16(
            seed+32, seed+33, seed+34, seed+35, seed+36, seed+37, seed+38, seed+39,
            seed+40, seed+41, seed+42, seed+43, seed+44, seed+45, seed+46, seed+47,
            seed+48, seed+49, seed+50, seed+51, seed+52, seed+53, seed+54, seed+55,
            seed+56, seed+57, seed+58, seed+59, seed+60, seed+61, seed+62, seed+63
        );
        
        __mmask32 mask32 = 0xAAAAAAAA;
        __m512i res32hi = _mm512_mask_blend_epi16(mask32, a32hi, b32hi);
        
        _mm512_storeu_si512((void*)g_store, res32hi);
        for (int i = 0; i < 32; i++) {
            checksum += ((short*)g_store)[i];
        }
    }
    
    // V32HFmode: 32 x half-precision floats (using _Float16)
    #if defined(__AVX512FP16__)
    {
        _Float16 a32hf_arr[32], b32hf_arr[32];
        for (int i = 0; i < 32; i++) {
            a32hf_arr[i] = (_Float16)(seed + i);
            b32hf_arr[i] = (_Float16)(seed + i + 32);
        }
        
        __m512h a32hf = _mm512_loadu_ph(a32hf_arr);
        __m512h b32hf = _mm512_loadu_ph(b32hf_arr);
        
        __mmask32 mask32hf = 0xAAAAAAAA;
        __m512h res32hf = _mm512_mask_blend_ph(mask32hf, a32hf, b32hf);
        
        _mm512_storeu_ph((void*)g_store, res32hf);
        for (int i = 0; i < 32; i++) {
            checksum += (int)((_Float16*)g_store)[i];
        }
    }
    #endif
    
    // V16SImode: 16 x 32-bit integers
    {
        __m512i a16si = _mm512_set_epi32(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7,
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __m512i b16si = _mm512_set_epi32(
            seed+16, seed+17, seed+18, seed+19, seed+20, seed+21, seed+22, seed+23,
            seed+24, seed+25, seed+26, seed+27, seed+28, seed+29, seed+30, seed+31
        );
        
        __mmask16 mask16 = 0xAAAA;
        __m512i res16si = _mm512_mask_blend_epi32(mask16, a16si, b16si);
        
        _mm512_storeu_si512((void*)g_store, res16si);
        for (int i = 0; i < 16; i++) {
            checksum += ((int*)g_store)[i];
        }
    }
    
    // V8DImode: 8 x 64-bit integers
    {
        __m512i a8di = _mm512_set_epi64(
            seed+0, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7
        );
        
        __m512i b8di = _mm512_set_epi64(
            seed+8, seed+9, seed+10, seed+11, seed+12, seed+13, seed+14, seed+15
        );
        
        __mmask8 mask8 = 0xAA;
        __m512i res8di = _mm512_mask_blend_epi64(mask8, a8di, b8di);
        
        _mm512_storeu_si512((void*)g_store, res8di);
        for (int i = 0; i < 8; i++) {
            checksum += (int)(((long long*)g_store)[i] & 0xFFFFFFFF);
        }
    }
    
    // V16SFmode: 16 x single-precision floats
    {
        float a16sf_arr[16], b16sf_arr[16];
        for (int i = 0; i < 16; i++) {
            a16sf_arr[i] = (float)(seed + i);
            b16sf_arr[i] = (float)(seed + i + 16);
        }
        
        __m512 a16sf = _mm512_loadu_ps(a16sf_arr);
        __m512 b16sf = _mm512_loadu_ps(b16sf_arr);
        
        __mmask16 mask16sf = 0xAAAA;
        __m512 res16sf = _mm512_mask_blend_ps(mask16sf, a16sf, b16sf);
        
        _mm512_storeu_ps((void*)g_store, res16sf);
        for (int i = 0; i < 16; i++) {
            checksum += (int)((float*)g_store)[i];
        }
    }
    
    // V8DFmode: 8 x double-precision floats
    {
        double a8df_arr[8], b8df_arr[8];
        for (int i = 0; i < 8; i++) {
            a8df_arr[i] = (double)(seed + i);
            b8df_arr[i] = (double)(seed + i + 8);
        }
        
        __m512d a8df = _mm512_loadu_pd(a8df_arr);
        __m512d b8df = _mm512_loadu_pd(b8df_arr);
        
        __mmask8 mask8df = 0xAA;
        __m512d res8df = _mm512_mask_blend_pd(mask8df, a8df, b8df);
        
        _mm512_storeu_pd((void*)g_store, res8df);
        for (int i = 0; i < 8; i++) {
            checksum += (int)((double*)g_store)[i];
        }
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    volatile int seed = 1;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = test_avx512_blend(seed);
    
    // Print result to prevent dead code elimination
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
