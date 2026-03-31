#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i global_v64qi __attribute__((used));
volatile __m512i global_v32hi __attribute__((used));
volatile __m512i global_v16si __attribute__((used));
volatile __m512i global_v8di __attribute__((used));
volatile __m512 global_v16sf __attribute__((used));
volatile __m512d global_v8df __attribute__((used));

#ifdef __AVX512FP16__
volatile __m512h global_v32hf __attribute__((used));
#endif

#ifdef __AVX512BF16__
volatile __m512bh global_v32bf __attribute__((used));
#endif

// Function to print results (prevents dead code elimination)
void use_result(int64_t result) {
    printf("Result: %ld\n", result);
}

int main() {
    int64_t final_result = 0;
    
#ifdef __AVX512F__
    // V16SF: 16 single-precision floats
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
                                  8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
        
        // Create mask by comparing a > b
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_v16sf = result;
        
        // Horizontal sum of first 4 elements
        __m128 sum4 = _mm_add_ps(_mm512_extractf32x4_ps(result, 0),
                                _mm512_extractf32x4_ps(result, 1));
        sum4 = _mm_add_ps(sum4, _mm512_extractf32x4_ps(result, 2));
        sum4 = _mm_add_ps(sum4, _mm512_extractf32x4_ps(result, 3));
        sum4 = _mm_hadd_ps(sum4, sum4);
        sum4 = _mm_hadd_ps(sum4, sum4);
        final_result += (int64_t)_mm_cvtss_f32(sum4);
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
        
        // Create mask by comparing a < b
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_v8df = result;
        
        // Horizontal sum
        __m256d sum4 = _mm256_add_pd(_mm512_extractf64x4_pd(result, 0),
                                    _mm512_extractf64x4_pd(result, 1));
        __m128d sum2 = _mm_add_pd(_mm256_extractf128_pd(sum4, 0),
                                 _mm256_extractf128_pd(sum4, 1));
        sum2 = _mm_hadd_pd(sum2, sum2);
        final_result += (int64_t)_mm_cvtsd_f64(sum2);
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8,
                                      9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(16, 15, 14, 13, 12, 11, 10, 9,
                                      8, 7, 6, 5, 4, 3, 2, 1);
        
        // Create mask by comparing a == b (will be false for all, giving mask=0)
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
        
        // Add some pattern to mask - blend where a elements are even
        __m512i even_mask_vec = _mm512_and_si512(a, _mm512_set1_epi32(1));
        mask = _mm512_cmpeq_epi32_mask(even_mask_vec, _mm512_setzero_si512());
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_v16si = result;
        
        // Sum first element
        final_result += _mm512_extract_epi32(result, 0);
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(8, 7, 6, 5, 4, 3, 2, 1);
        
        // Create mask by comparing a > b
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_v8di = result;
        
        // Sum first element
        final_result += _mm512_extract_epi64(result, 0);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    // V64QI: 64 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
        
        __m512i b = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63);
        
        // Create mask: blend where a > 31
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, _mm512_set1_epi8(31));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        global_v64qi = result;
        
        // Sum first 8 elements
        int8_t temp[64];
        _mm512_storeu_si512((void*)temp, result);
        for (int i = 0; i < 8; i++) {
            final_result += temp[i];
        }
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
        
        __m512i b = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31);
        
        // Create mask: blend where a > 15
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, _mm512_set1_epi16(15));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_v32hi = result;
        
        // Sum first 4 elements
        int16_t temp[32];
        _mm512_storeu_si512((void*)temp, result);
        for (int i = 0; i < 4; i++) {
            final_result += temp[i];
        }
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    // V32HF: 32 half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)(32 - i);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a > b
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32hf = result;
        
        // Extract and sum first 4 elements
        _Float16 temp[32];
        _mm512_storeu_ph(temp, result);
        for (int i = 0; i < 4; i++) {
            final_result += (int64_t)temp[i];
        }
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF: 32 brain float values
    {
        // Initialize with pattern
        __m512bh a = _mm512_set1_epi16(0x3C00); // 1.0 in bfloat16
        __m512bh b = _mm512_set1_epi16(0x4000); // 2.0 in bfloat16
        
        // Create alternating pattern in a
        uint16_t a_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (i % 2 == 0) ? 0x3C00 : 0x4000; // Alternating 1.0 and 2.0
        }
        a = _mm512_loadu_epi16(a_data);
        
        // Create mask: blend where element index is even
        __mmask32 mask = 0xAAAAAAAA; // Alternating bits: 10101010...
        
        // Perform blend
        __m512bh result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_v32bf = result;
        
        // Extract first element
        uint16_t temp[32];
        _mm512_storeu_epi16(temp, (__m512i)result);
        final_result += temp[0];
    }
#endif // __AVX512BF16__

    // Print final result to prevent optimization
    use_result(final_result);
    
    return 0;
}
