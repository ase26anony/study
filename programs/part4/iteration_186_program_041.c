#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i v64qi_result;
volatile __m512i v32hi_result;
volatile __m512i v16si_result;
volatile __m512i v8di_result;
volatile __m512 v16sf_result;
volatile __m512d v8df_result;

#ifdef __AVX512FP16__
volatile __m512h v32hf_result;
#endif

#ifdef __AVX512BF16__
volatile __m512bh v32bf_result;
#endif

int main() {
    int sum = 0;
    
#ifdef __AVX512BW__
    // Test V64QImode - 64-byte vectors of 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        __m512i b = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask by comparing a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        v64qi_result = result;
        
        // Use result in computation
        sum += _mm512_extract_epi8(result, 0);
    }
    
    // Test V32HImode - 32-word vectors of 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask by comparing a > 15
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        v32hi_result = result;
        
        // Use result in computation
        sum += _mm512_extract_epi16(result, 0);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    // Test V16SImode - 16-dword vectors of 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask by comparing a > 7
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(7));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        v16si_result = result;
        
        // Use result in computation
        sum += _mm512_extract_epi32(result, 0);
    }
    
    // Test V8DImode - 8-qword vectors of 64-bit integers
    {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask by comparing a > 3
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(3));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        v8di_result = result;
        
        // Use result in computation
        sum += (int)_mm512_extract_epi64(result, 0);
    }
    
    // Test V16SFmode - 16-dword vectors of single-precision floats
    {
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // Create mask by comparing a > 7.5f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(7.5f), _CMP_GT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        v16sf_result = result;
        
        // Use result in computation
        sum += (int)_mm512_cvtss_f32(result);
    }
    
    // Test V8DFmode - 8-qword vectors of double-precision floats
    {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask by comparing a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        v8df_result = result;
        
        // Use result in computation
        sum += (int)_mm512_cvtsd_f64(result);
    }
#endif // __AVX512F__

#ifdef __AVX512FP16__
    // Test V32HFmode - 32-word vectors of half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)i;
            b_data[31 - i] = (_Float16)i;
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a > 15.5
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(15.5f), _CMP_GT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        v32hf_result = result;
        
        // Use result in computation
        _Float16 first = _mm512_cvtph_f16(result);
        sum += (int)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // Test V32BFmode - 32-word vectors of brain float
    {
        // Initialize with pattern
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = bfloat16_from_float((float)i);
            b_data[31 - i] = bfloat16_from_float((float)i);
        }
        
        __m512bh a = _mm512_loadu_bh(a_data);
        __m512bh b = _mm512_loadu_bh(b_data);
        
        // Create mask by comparing a > 15.5 (need to convert to float for comparison)
        __m512 a_f32 = _mm512_cvtpbh_ps(a);
        __m512 b_f32 = _mm512_cvtpbh_ps(b);
        __mmask16 mask = _mm512_cmp_ps_mask(a_f32, _mm512_set1_ps(15.5f), _CMP_GT_OQ);
        
        // Perform blend - use the same intrinsic as V32HF
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        v32bf_result = result;
        
        // Use result in computation
        __m128 result_low = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castbh_si512(result)));
        sum += (int)_mm_cvtss_f32(result_low);
    }
#endif // __AVX512BF16__

    printf("Result sum: %d\n", sum);
    
    // Additional computation to ensure all results are used
    int final_check = 0;
    
#ifdef __AVX512BW__
    final_check += _mm512_extract_epi8(v64qi_result, 63);
    final_check += _mm512_extract_epi16(v32hi_result, 31);
#endif
    
#ifdef __AVX512F__
    final_check += _mm512_extract_epi32(v16si_result, 15);
    final_check += (int)_mm512_extract_epi64(v8di_result, 7);
    final_check += (int)_mm512_cvtss_f32(v16sf_result);
    final_check += (int)_mm512_cvtsd_f64(v8df_result);
#endif
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
