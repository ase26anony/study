#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global volatile arrays to prevent dead code elimination
volatile int8_t g_result_i8[64] __attribute__((aligned(64)));
volatile int16_t g_result_i16[32] __attribute__((aligned(64)));
volatile int32_t g_result_i32[16] __attribute__((aligned(64)));
volatile int64_t g_result_i64[8] __attribute__((aligned(64)));
volatile float g_result_f32[16] __attribute__((aligned(64)));
volatile double g_result_f64[8] __attribute__((aligned(64)));
volatile uint16_t g_result_f16[32] __attribute__((aligned(64)));
volatile uint16_t g_result_bf16[32] __attribute__((aligned(64)));

// Main test function with all blend operations
__attribute__((noinline, target("avx512f,avx512bw,avx512vl")))
int test_avx512_blend(int seed) {
    int checksum = 0;
    
    // ========== V64QImode: 64 x 8-bit integers ==========
    {
        // Create non-uniform patterns
        __m512i a64qi = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        __m512i b64qi = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL; // Alternating pattern
        __m512i res64qi = _mm512_mask_blend_epi8(mask64, a64qi, b64qi);
        
        // Store to volatile memory to prevent elimination
        _mm512_store_si512((void*)g_result_i8, res64qi);
        
        // Accumulate checksum
        for (int i = 0; i < 64; i++) {
            checksum += g_result_i8[i];
        }
    }
    
    // ========== V32HImode: 32 x 16-bit integers ==========
    {
        __m512i a32hi = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b32hi = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __mmask32 mask32 = 0xAAAAAAAA; // Alternating pattern
        __m512i res32hi = _mm512_mask_blend_epi16(mask32, a32hi, b32hi);
        
        _mm512_store_si512((void*)g_result_i16, res32hi);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_i16[i];
        }
    }
    
    // ========== V16SImode: 16 x 32-bit integers ==========
    {
        __m512i a16si = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b16si = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        __mmask16 mask16 = 0xAAAA; // Alternating pattern
        __m512i res16si = _mm512_mask_blend_epi32(mask16, a16si, b16si);
        
        _mm512_store_si512((void*)g_result_i32, res16si);
        
        for (int i = 0; i < 16; i++) {
            checksum += g_result_i32[i];
        }
    }
    
    // ========== V8DImode: 8 x 64-bit integers ==========
    {
        __m512i a8di = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b8di = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        __mmask8 mask8 = 0xAA; // Alternating pattern
        __m512i res8di = _mm512_mask_blend_epi64(mask8, a8di, b8di);
        
        _mm512_store_si512((void*)g_result_i64, res8di);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_result_i64[i];
        }
    }
    
    // ========== V16SFmode: 16 x single-precision floats ==========
    {
        __m512 a16sf = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b16sf = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __mmask16 mask16f = 0xAAAA; // Alternating pattern
        __m512 res16sf = _mm512_mask_blend_ps(mask16f, a16sf, b16sf);
        
        _mm512_store_ps((void*)g_result_f32, res16sf);
        
        for (int i = 0; i < 16; i++) {
            checksum += (int)g_result_f32[i];
        }
    }
    
    // ========== V8DFmode: 8 x double-precision floats ==========
    {
        __m512d a8df = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b8df = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        __mmask8 mask8d = 0xAA; // Alternating pattern
        __m512d res8df = _mm512_mask_blend_pd(mask8d, a8df, b8df);
        
        _mm512_store_pd((void*)g_result_f64, res8df);
        
        for (int i = 0; i < 8; i++) {
            checksum += (int)g_result_f64[i];
        }
    }
    
    // ========== V32HFmode: 16-bit floating point (half precision) ==========
    // Note: Requires AVX-512-FP16 extension
    #ifdef __AVX512FP16__
    {
        __m512h a32hf = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __m512h b32hf = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        __mmask32 mask32hf = 0xAAAAAAAA; // Alternating pattern
        __m512h res32hf = _mm512_mask_blend_ph(mask32hf, a32hf, b32hf);
        
        _mm512_store_ph((void*)g_result_f16, res32hf);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_f16[i];
        }
    }
    #endif
    
    // ========== V32BFmode: bfloat16 ==========
    // Note: Requires AVX-512-BF16 extension
    #ifdef __AVX512BF16__
    {
        __m512bh a32bf = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh b32bf = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create alternating pattern
        __mmask32 mask32bf = 0xAAAAAAAA; // Alternating pattern
        __m512bh res32bf = _mm512_mask_blend_epi16(mask32bf, a32bf, b32bf);
        
        _mm512_store_si512((void*)g_result_bf16, res32bf);
        
        for (int i = 0; i < 32; i++) {
            checksum += g_result_bf16[i];
        }
    }
    #endif
    
    return checksum + seed; // Add seed to vary result
}

int main(int argc, char *argv[]) {
    // Use command line argument as seed if provided
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Call the test function
    int result = test_avx512_blend(seed);
    
    // Print result to prevent dead code elimination
    printf("Blend checksum: %d\n", result);
    
    return 0;
}
