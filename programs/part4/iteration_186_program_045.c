#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512d global_v8df;
volatile __m512 global_v16sf;

#ifdef __AVX512FP16__
volatile __m512h global_v32hf;
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf;
#endif

int main() {
    uint64_t final_sum = 0;
    
#ifdef __AVX512F__
    // V16SF: 16 single-precision floats
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Blend based on mask
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_v16sf = result;
        
        // Extract first element and add to sum
        float first = _mm512_cvtss_f32(result);
        final_sum += (uint64_t)first;
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Blend based on mask
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result to prevent optimization
        global_v8df = result;
        
        // Extract first element and add to sum
        double first = _mm512_cvtsd_f64(result);
        final_sum += (uint64_t)first;
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_v16si = result;
        
        // Extract first element and add to sum
        int32_t first = _mm512_cvtsi512_si32(result);
        final_sum += (uint64_t)first;
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask by checking odd/even
        __m512i mask_val = _mm512_set1_epi64(1);
        __m512i and_result = _mm512_and_epi64(a, mask_val);
        __mmask8 mask = _mm512_cmpeq_epi64_mask(and_result, mask_val);
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result to prevent optimization
        global_v8di = result;
        
        // Extract first element and add to sum
        int64_t first = _mm512_cvtsi512_si64(result);
        final_sum += (uint64_t)first;
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    // V64QI: 64 8-bit integers
    {
        __m512i a = _mm512_set1_epi8(1);
        __m512i b = _mm512_set1_epi8(100);
        
        // Create pattern: 01010101...
        for (int i = 0; i < 64; i++) {
            ((char*)&a)[i] = i % 2;
            ((char*)&b)[i] = 100 - i;
        }
        
        // Create mask by comparing with 0
        __m512i zero = _mm512_setzero_si512();
        __mmask64 mask = _mm512_cmpeq_epi8_mask(a, zero);
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result to prevent optimization
        global_v64qi = result;
        
        // Extract first element and add to sum
        int8_t first = _mm512_cvtsi512_si32(result) & 0xFF;
        final_sum += (uint64_t)first;
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_set1_epi16(1);
        __m512i b = _mm512_set1_epi16(1000);
        
        // Create pattern
        for (int i = 0; i < 32; i++) {
            ((short*)&a)[i] = i;
            ((short*)&b)[i] = 1000 + i;
        }
        
        // Create mask by comparing with threshold
        __m512i threshold = _mm512_set1_epi16(16);
        __mmask32 mask = _mm512_cmplt_epi16_mask(a, threshold);
        
        // Blend based on mask
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hi = result;
        
        // Extract first element and add to sum
        int16_t first = _mm512_cvtsi512_si32(result) & 0xFFFF;
        final_sum += (uint64_t)first;
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    // V32HF: 32 half-precision floats
    {
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)(100 + i);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __m512h threshold = _mm512_set1_ph(16.0);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_LT_OQ);
        
        // Blend based on mask
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result to prevent optimization
        global_v32hf = result;
        
        // Extract first element and add to sum
        _Float16 first = _mm512_cvtsh_h(result);
        final_sum += (uint64_t)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF: 32 brain float values
    {
        // Note: __m512bh is a mask register type, not a data register
        // We need to use __m512i for bfloat16 data
        __m512i a = _mm512_set1_epi16(0x3C00); // 1.0 in bfloat16
        __m512i b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create pattern
        for (int i = 0; i < 32; i++) {
            ((uint16_t*)&a)[i] = 0x3C00 + i; // 1.0 + small increment
            ((uint16_t*)&b)[i] = 0x4000 + i; // 2.0 + small increment
        }
        
        // For bfloat16, we need to use the same intrinsic as half-precision
        // but with bfloat16 data
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern 10101010...
        
        // Blend based on mask - using the same intrinsic as V32HF
        __m512i result_int = _mm512_mask_blend_epi16(mask, a, b);
        
        // Cast to __m512bh for storage
        __m512bh result = _mm512_castsi512_bh(result_int);
        
        // Use result to prevent optimization
        global_v32bf = result;
        
        // Extract first element and add to sum
        uint16_t first = _mm512_cvtsi512_si32(result_int) & 0xFFFF;
        final_sum += (uint64_t)first;
    }
#endif // __AVX512BF16__

    printf("Final sum: %lu\n", final_sum);
    return 0;
}
