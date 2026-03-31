#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile array to prevent optimization
volatile __m512i global_vi;
volatile __m512 global_vf;
volatile __m512d global_vd;
volatile __m512h global_vh;

// Function to print results
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

int main() {
    long long total_result = 0;
    
#ifdef __AVX512F__
    // V16SI - 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result to prevent optimization
        global_vi = result;
        total_result += _mm512_extract_epi32(result, 0);
    }
    
    // V8DI - 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask by checking odd/even
        __mmask8 mask = _mm512_cmpeq_epi64_mask(
            _mm512_and_epi64(a, _mm512_set1_epi64(1)),
            _mm512_set1_epi64(0)
        );
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        global_vi = result;
        total_result += _mm512_extract_epi64(result, 0);
    }
    
    // V8DF - 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
        __m512d b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        global_vd = result;
        total_result += (long long)_mm512_cvtsd_f64(result);
    }
    
    // V16SF - 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512 b = _mm512_set_ps(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        
        // Create mask: a < 8.0
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0), _CMP_LT_OQ);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        global_vf = result;
        total_result += (long long)_mm512_cvtss_f32(result);
    }
    
#ifdef __AVX512BW__
    // V64QI - 64 x 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
        );
        
        // Create mask: a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        global_vi = result;
        total_result += _mm512_extract_epi8(result, 0);
    }
    
    // V32HI - 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
        );
        
        // Create mask: a < 16
        __mmask32 mask = _mm512_cmplt_epi16_mask(a, _mm512_set1_epi16(16));
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        global_vi = result;
        total_result += _mm512_extract_epi16(result, 0);
    }
    
#ifdef __AVX512FP16__
    // V32HF - 32 x half-precision floats
    {
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(31 - i);
            b_data[i] = (_Float16)i;
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: a > 15.5
        __mmask32 mask = _mm512_cmp_ph_mask(
            a, 
            _mm512_set1_ph((_Float16)15.5), 
            _CMP_GT_OQ
        );
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        global_vh = result;
        
        // Extract first element
        _Float16 first;
        _mm_store_sh(&first, _mm512_extractf32x4_ps(_mm512_castph_ps(result), 0));
        total_result += (long long)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF - 32 x brain float (using same intrinsic as half-precision)
    {
        // BF16 uses same intrinsics as FP16 but with different types
        __m512bh a = _mm512_set1_epi16(0x3F80); // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create alternating pattern
        __mmask32 mask = 0xAAAAAAAA; // 1010... pattern
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Store to prevent optimization
        uint16_t bf_data[32];
        _mm512_storeu_epi16(bf_data, _mm512_castpbh_si512(result));
        total_result += bf_data[0];
    }
#endif // __AVX512BF16__

#endif // __AVX512BW__
#endif // __AVX512F__
    
    printf("Total result: %lld\n", total_result);
    
    // Force use of all global variables to prevent optimization
    asm volatile("" : : "m"(global_vi), "m"(global_vf), "m"(global_vd), "m"(global_vh));
    
    return 0;
}
