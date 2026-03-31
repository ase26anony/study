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

int main() {
    int result_sum = 0;
    
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
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_v16sf = res;
        
        // Extract first element and add to sum
        float first = _mm512_cvtss_f32(res);
        result_sum += (int)first;
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
        
        // Create mask by comparing a < b
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        
        // Perform blend
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_v8df = res;
        
        // Extract first element
        double first = _mm512_cvtsd_f64(res);
        result_sum += (int)first;
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
        
        // Create mask by comparing a == b (will be mostly false)
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
        
        // Also add some bits to mask to ensure blend happens
        mask |= 0xAAAA;  // Alternating bits pattern
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_v16si = res;
        
        // Extract first element
        int first = _mm512_extract_epi32(res, 0);
        result_sum += first;
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(8, 7, 6, 5, 4, 3, 2, 1);
        
        // Create mask by comparing a > b
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, b);
        
        // Ensure some blending happens
        mask |= 0x55;  // 01010101 pattern
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_v8di = res;
        
        // Extract first element
        long long first = _mm512_extract_epi64(res, 0);
        result_sum += (int)first;
    }
#endif  // __AVX512F__

#ifdef __AVX512BW__
    // V64QI: 64 8-bit integers
    {
        // Create pattern data
        uint8_t a_data[64], b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = 63 - i;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask by comparing a > b
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, b);
        
        // Add pattern to ensure blending
        mask |= 0xAAAAAAAAAAAAAAAA;  // Alternating pattern
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        global_v64qi = res;
        
        // Extract first element
        int8_t first = _mm512_extract_epi8(res, 0);
        result_sum += first;
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                      17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
        __m512i b = _mm512_setr_epi16(32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,
                                      16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
        
        // Create mask by comparing a < b
        __mmask32 mask = _mm512_cmplt_epi16_mask(a, b);
        
        // Add pattern
        mask |= 0xAAAAAAAA;  // Alternating bits
        
        // Perform blend
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_v32hi = res;
        
        // Extract first element
        int16_t first = _mm512_extract_epi16(res, 0);
        result_sum += first;
    }
#endif  // __AVX512BW__

#ifdef __AVX512FP16__
    // V32HF: 32 half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(i + 1);
            b_data[i] = (_Float16)(32 - i);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a > b
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
        
        // Add pattern
        mask |= 0xAAAAAAAA;  // Alternating bits
        
        // Perform blend
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32hf = res;
        
        // Extract first element
        _Float16 first = _mm512_extract_ph(res, 0);
        result_sum += (int)first;
    }
#endif  // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF: 32 brain float values
    {
        // Initialize with pattern
        __bfloat16 a_data[32], b_data[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern
            a_data[i] = bfloat16_from_float((float)(i + 1));
            b_data[i] = bfloat16_from_float((float)(32 - i));
        }
        
        __m512bh a = _mm512_loadu_bf16(a_data);
        __m512bh b = _mm512_loadu_bf16(b_data);
        
        // For BF16, we need to convert to float for comparison
        __m512 a_f = _mm512_cvtpbh_ps(a);
        __m512 b_f = _mm512_cvtpbh_ps(b);
        
        // Create mask by comparing float versions
        __mmask16 mask_f = _mm512_cmp_ps_mask(a_f, b_f, _CMP_GT_OQ);
        
        // Expand 16-bit mask to 32-bit for BF16 blend
        __mmask32 mask = 0;
        for (int i = 0; i < 16; i++) {
            if (mask_f & (1 << i)) {
                mask |= (3 << (i * 2));  // Set both BF16 elements in the pair
            }
        }
        
        // Add pattern
        mask |= 0xAAAAAAAA;
        
        // Perform blend
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_v32bf = res;
        
        // Extract first element
        __bfloat16 first = _mm512_extract_bf16(res, 0);
        result_sum += bfloat16_to_float(first);
    }
#endif  // __AVX512BF16__

    printf("Result sum: %d\n", result_sum);
    return 0;
}
