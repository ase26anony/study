#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i global_v64qi;
volatile __m512i global_v32hi;
volatile __m512i global_v16si;
volatile __m512i global_v8di;
volatile __m512 global_v16sf;
volatile __m512d global_v8df;

#ifdef __AVX512FP16__
volatile __m512h global_v32hf;
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf;
#endif

int main() {
    uint64_t final_sum = 0;
    
#ifdef __AVX512BW__
    // V64QImode: 64-byte vectors of 8-bit integers
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
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result to prevent optimization
        global_v64qi = result;
        
        // Extract first element and add to sum
        final_sum += (uint8_t)_mm512_extract_epi8(result, 0);
    }
    
    // V32HImode: 32-word vectors of 16-bit integers
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
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_v32hi = result;
        final_sum += (uint16_t)_mm512_extract_epi16(result, 0);
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    // V16SImode: 16-dword vectors of 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create mask by comparing a > 7
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(7));
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_v16si = result;
        final_sum += (uint32_t)_mm512_extract_epi32(result, 0);
    }
    
    // V8DImode: 8-qword vectors of 64-bit integers
    {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        
        // Create mask by comparing a > 3
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, _mm512_set1_epi64(3));
        
        // Blend operation
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_v8di = result;
        final_sum += (uint64_t)_mm512_extract_epi64(result, 0);
    }
    
    // V8DFmode: 8-qword vectors of double-precision floats
    {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        
        // Create mask by comparing a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        // Blend operation
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_v8df = result;
        final_sum += (uint64_t)_mm512_cvttsd_u64(result);
    }
    
    // V16SFmode: 16-dword vectors of single-precision floats
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
        
        // Blend operation
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        global_v16sf = result;
        final_sum += (uint32_t)_mm512_cvttps_epu32(result)[0];
    }
#endif // __AVX512F__

#ifdef __AVX512FP16__
    // V32HFmode: 32-word vectors of half-precision floats
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
        __mmask32 mask = _mm512_cmp_ph_mask(
            a, 
            _mm512_set1_ph((_Float16)15.5), 
            _CMP_GT_OQ
        );
        
        // Blend operation
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32hf = result;
        
        // Extract first element
        _Float16 first;
        _mm512_storeu_ph(&first, result);
        final_sum += (uint16_t)first;
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BFmode: 32-word vectors of brain float
    {
        // Initialize with pattern
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = bfloat16_from_float((float)i);
            b_data[31 - i] = bfloat16_from_float((float)i);
        }
        
        __m512bh a = _mm512_loadu_ph((void*)a_data);
        __m512bh b = _mm512_loadu_ph((void*)b_data);
        
        // Create mask by comparing (as floats) a > 15.5
        // Note: We need to convert to float for comparison
        float a_float[32];
        float b_float[32];
        
        for (int i = 0; i < 32; i++) {
            a_float[i] = bfloat16_to_float(a_data[i]);
            b_float[i] = bfloat16_to_float(b_data[i]);
        }
        
        __m512 a_f = _mm512_loadu_ps(a_float);
        __m512 b_f = _mm512_loadu_ps(b_float);
        
        __mmask32 mask = _mm512_cmp_ps_mask(
            a_f, 
            _mm512_set1_ps(15.5f), 
            _CMP_GT_OQ
        );
        
        // Blend operation
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32bf = result;
        
        // Extract first element
        __bfloat16 first;
        _mm512_storeu_ph((void*)&first, result);
        final_sum += (uint16_t)first;
    }
#endif // __AVX512BF16__

    printf("Final checksum: %lu\n", final_sum);
    return 0;
}
