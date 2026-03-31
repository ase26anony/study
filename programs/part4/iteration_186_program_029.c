#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Global volatile arrays to prevent optimization
volatile __m512i v64qi_result, v32hi_result, v16si_result, v8di_result;
volatile __m512 v16sf_result;
volatile __m512d v8df_result;
#ifdef __AVX512FP16__
volatile __m512h v32hf_result;
#endif
#ifdef __AVX512BF16__
volatile __m512bh v32bf_result;
#endif

// Function to print bits of a mask (for debugging)
void print_mask64(__mmask64 mask) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

void print_mask32(__mmask32 mask) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

void print_mask16(__mmask16 mask) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

void print_mask8(__mmask8 mask) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (mask >> i) & 1);
    }
    printf("\n");
}

int main() {
    int64_t final_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // ==================== V16SI (16 x 32-bit integers) ====================
    {
        __m512i a = _mm512_set_epi32(31, 30, 29, 28, 27, 26, 25, 24,
                                     23, 22, 21, 20, 19, 18, 17, 16);
        __m512i b = _mm512_set_epi32(100, 99, 98, 97, 96, 95, 94, 93,
                                     92, 91, 90, 89, 88, 87, 86, 85);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        v16si_result = result;
        
        // Extract first element and add to sum
        final_sum += _mm512_extract_epi32(result, 0);
        
        printf("V16SI mask: ");
        print_mask16(mask);
    }
    
    // ==================== V8DI (8 x 64-bit integers) ====================
    {
        __m512i a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
        __m512i b = _mm512_set_epi64(100, 99, 98, 97, 96, 95, 94, 93);
        
        // Create mask by checking if element is even
        __mmask8 mask = _mm512_test_epi64_mask(a, _mm512_set1_epi64(1));
        mask = ~mask; // Invert to select even elements from a
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        v8di_result = result;
        
        // Extract first element and add to sum
        final_sum += _mm512_extract_epi64(result, 0);
        
        printf("V8DI mask: ");
        print_mask8(mask);
    }
    
    // ==================== V16SF (16 x single-precision floats) ====================
    {
        __m512 a = _mm512_set_ps(31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
                                 23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f);
        __m512 b = _mm512_set_ps(100.0f, 99.0f, 98.0f, 97.0f, 96.0f, 95.0f, 94.0f, 93.0f,
                                 92.0f, 91.0f, 90.0f, 89.0f, 88.0f, 87.0f, 86.0f, 85.0f);
        
        // Create mask by comparing a > 20.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(20.0f), _CMP_GT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        v16sf_result = result;
        
        // Extract first element and add to sum (convert to int)
        final_sum += (int)_mm512_cvtss_f32(result);
        
        printf("V16SF mask: ");
        print_mask16(mask);
    }
    
    // ==================== V8DF (8 x double-precision floats) ====================
    {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(100.0, 99.0, 98.0, 97.0, 96.0, 95.0, 94.0, 93.0);
        
        // Create mask by comparing a < 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_LT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        v8df_result = result;
        
        // Extract first element and add to sum (convert to int)
        final_sum += (int)_mm512_cvtsd_f64(result);
        
        printf("V8DF mask: ");
        print_mask8(mask);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blend operations...\n");
    
    // ==================== V64QI (64 x 8-bit integers) ====================
    {
        // Create pattern: 0, 1, 2, ..., 63
        __m512i a = _mm512_set_epi8(
            63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
            47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create another pattern: 100, 101, 102, ...
        __m512i b = _mm512_set_epi8(
            163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148,
            147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132,
            131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116,
            115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100
        );
        
        // Create mask: select elements where a < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        v64qi_result = result;
        
        // Extract first element and add to sum
        final_sum += _mm512_extract_epi8(result, 0);
        
        printf("V64QI mask (first 16 bits): ");
        print_mask16(mask & 0xFFFF);
    }
    
    // ==================== V32HI (32 x 16-bit integers) ====================
    {
        // Create pattern: 0, 1, 2, ..., 31
        __m512i a = _mm512_set_epi16(
            31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        );
        
        // Create another pattern: 100, 101, 102, ...
        __m512i b = _mm512_set_epi16(
            131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116,
            115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100
        );
        
        // Create mask: select elements where a is odd
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        v32hi_result = result;
        
        // Extract first element and add to sum
        final_sum += _mm512_extract_epi16(result, 0);
        
        printf("V32HI mask (first 8 bits): ");
        print_mask8(mask & 0xFF);
    }
#endif // __AVX512BW__

#ifdef __AVX512FP16__
    printf("\nTesting AVX-512FP16 blend operations...\n");
    
    // ==================== V32HF (32 x half-precision floats) ====================
    {
        // Create pattern: 0.0, 1.0, 2.0, ..., 31.0
        _Float16 a_data[32];
        _Float16 b_data[32];
        
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)i;
            b_data[i] = (_Float16)(i + 100);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __mmask32 mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(16.0), _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        v32hf_result = result;
        
        // Extract first element and add to sum (convert to int)
        _Float16 first = _mm512_cvtsh_h(result);
        final_sum += (int)first;
        
        printf("V32HF mask (first 8 bits): ");
        print_mask8(mask & 0xFF);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("\nTesting AVX-512BF16 blend operations...\n");
    
    // ==================== V32BF (32 x brain float) ====================
    {
        // Brain float uses same intrinsics as half-precision
        // Create pattern: 0.0, 1.0, 2.0, ..., 31.0
        __m512bh a, b;
        __m512bh a_full, b_full;
        
        // Initialize using float arrays and convert
        float a_float[32], b_float[32];
        for (int i = 0; i < 32; i++) {
            a_float[i] = (float)i;
            b_float[i] = (float)(i + 100);
        }
        
        // Convert float to bfloat16
        __m512 a_f32 = _mm512_loadu_ps(a_float);
        __m512 b_f32 = _mm512_loadu_ps(b_float);
        
        a_full = (__m512bh)_mm512_cvtneps_pbh(a_f32);
        b_full = (__m512bh)_mm512_cvtneps_pbh(b_f32);
        
        // For blend, we need to use the same mask generation as FP16
        // Create mask by comparing original float values
        __mmask32 mask = _mm512_cmp_ps_mask(a_f32, _mm512_set1_ps(16.0), _CMP_LT_OQ);
        
        // Perform blend (using same intrinsic as FP16)
        __m512bh result = _mm512_mask_blend_ph(mask, a_full, b_full);
        v32bf_result = result;
        
        // Convert back to float to extract value
        __m512 result_f32 = _mm512_cvtpbh_ps((__m256bh)_mm512_extractf64x4_pd((__m512d)result, 0));
        float first = _mm512_cvtss_f32(result_f32);
        final_sum += (int)first;
        
        printf("V32BF mask (first 8 bits): ");
        print_mask8(mask & 0xFF);
    }
#endif // __AVX512BF16__

    printf("\nFinal aggregated sum: %ld\n", final_sum);
    printf("Test completed successfully!\n");
    
    return 0;
}
