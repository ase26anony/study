#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i volatile_result_i;
volatile __m512d volatile_result_d;
volatile __m512 volatile_result_f;
#ifdef __AVX512FP16__
volatile __m512h volatile_result_h;
#endif

// Function to print bits of a mask (for debugging)
void print_mask8(__mmask8 m) {
    for (int i = 7; i >= 0; i--) printf("%d", (m >> i) & 1);
}
void print_mask16(__mmask16 m) {
    for (int i = 15; i >= 0; i--) printf("%d", (m >> i) & 1);
}
void print_mask32(__mmask32 m) {
    for (int i = 31; i >= 0; i--) printf("%d", (m >> i) & 1);
}
void print_mask64(__mmask64 m) {
    for (int i = 63; i >= 0; i--) printf("%d", (m >> i) & 1);
}

int main() {
    int result_sum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // V16SI: 16 x 32-bit integers
    {
        __m512i a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
        __m512i b = _mm512_set_epi32(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0);
        
        // Create mask by comparing a > 20
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, _mm512_set1_epi32(20));
        
        __m512i res = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result in computation
        __m512i sum = _mm512_add_epi32(res, _mm512_set1_epi32(1));
        volatile_result_i = sum;
        result_sum += _mm512_extract_epi32(sum, 0);
    }
    
    // V8DI: 8 x 64-bit integers
    {
        __m512i a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
        
        // Create mask using equality test
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, _mm512_set1_epi64(3));
        
        __m512i res = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi64(res, _mm512_set1_epi64(1));
        volatile_result_i = sum;
        result_sum += _mm512_extract_epi64(sum, 0);
    }
    
    // V8DF: 8 x double-precision floats
    {
        __m512d a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
        __m512d b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        
        // Create mask: a > 3.5
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(3.5), _CMP_GT_OQ);
        
        __m512d res = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        __m512d sum = _mm512_add_pd(res, _mm512_set1_pd(1.0));
        volatile_result_d = sum;
        result_sum += (int)_mm512_reduce_add_pd(sum);
    }
    
    // V16SF: 16 x single-precision floats
    {
        __m512 a = _mm512_set_ps(15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                                 7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f);
        __m512 b = _mm512_set_ps(0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                                 8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f);
        
        // Create mask: a < 8.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(8.0f), _CMP_LT_OQ);
        
        __m512 res = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result
        __m512 sum = _mm512_add_ps(res, _mm512_set1_ps(1.0f));
        volatile_result_f = sum;
        result_sum += (int)_mm512_reduce_add_ps(sum);
    }
#endif

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // V64QI: 64 x 8-bit integers
    {
        // Create pattern: 0,1,2,3,...63
        __m512i a = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        // Reverse pattern
        __m512i b = _mm512_set_epi8(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
            32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63);
        
        // Create mask: select where a < 32
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, _mm512_set1_epi8(32));
        
        __m512i res = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi8(res, _mm512_set1_epi8(1));
        volatile_result_i = sum;
        
        // Extract and sum first 8 bytes
        uint64_t first_qword = _mm512_extract_epi64(sum, 0);
        for (int i = 0; i < 8; i++) {
            result_sum += (int)((first_qword >> (i * 8)) & 0xFF);
        }
    }
    
    // V32HI: 32 x 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        __m512i b = _mm512_set_epi16(
            0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
            16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);
        
        // Create mask: select where a is even
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        mask = ~mask;  // Invert to select even numbers
        
        __m512i res = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        __m512i sum = _mm512_add_epi16(res, _mm512_set1_epi16(1));
        volatile_result_i = sum;
        
        // Sum first 4 elements
        for (int i = 0; i < 4; i++) {
            result_sum += _mm512_extract_epi16(sum, i);
        }
    }
#endif

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // V32HF: 32 x half-precision floats
    {
        // Initialize with pattern
        _Float16 a_data[32];
        _Float16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = (_Float16)(31 - i);
            b_data[i] = (_Float16)i;
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask: select where a > 15.5
        __m512h threshold = _mm512_set1_ph((_Float16)15.5);
        __mmask32 mask = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
        
        __m512h res = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        __m512h sum = _mm512_add_ph(res, _mm512_set1_ph((_Float16)1.0));
        volatile_result_h = sum;
        
        // Extract and sum first 4 elements
        _Float16 result_array[32];
        _mm512_storeu_ph(result_array, sum);
        for (int i = 0; i < 4; i++) {
            result_sum += (int)result_array[i];
        }
    }
#endif

#ifdef __AVX512BF16__
    printf("Testing AVX-512BF16 blend operations...\n");
    
    // V32BF: 32 x brain float (using same intrinsic as half-precision)
    {
        // Note: bfloat16 uses same intrinsic as half-precision for blend
        __m512bh a, b;
        
        // Initialize with some pattern
        uint16_t a_data[32];
        uint16_t b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = i * 128;  // Simple pattern
            b_data[i] = (31 - i) * 128;
        }
        
        // Load as bfloat16 vectors
        a = _mm512_loadu_si512((__m512i*)a_data);
        b = _mm512_loadu_si512((__m512i*)b_data);
        
        // Convert to float for comparison
        __m512 a_f32 = _mm512_cvtneobf16_ps((__m512i)a_data);
        __m512 b_f32 = _mm512_cvtneobf16_ps((__m512i)b_data);
        
        // Create mask using float comparison
        __mmask16 mask32 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
        
        // Expand 16-bit mask to 32-bit mask for blend
        __mmask32 mask = 0;
        for (int i = 0; i < 16; i++) {
            mask |= ((__mmask32)((mask32 >> i) & 1) << (2*i));
            mask |= ((__mmask32)((mask32 >> i) & 1) << (2*i + 1));
        }
        
        // Blend using half-precision intrinsic (same for bfloat16)
        __m512bh res = _mm512_mask_blend_ph(mask, a, b);
        
        // Store and use result
        uint16_t result_array[32];
        _mm512_storeu_si512((__m512i*)result_array, (__m512i)res);
        
        for (int i = 0; i < 4; i++) {
            result_sum += result_array[i];
        }
    }
#endif

    printf("Final result sum: %d\n", result_sum);
    printf("All AVX-512 blend operations tested.\n");
    
    return 0;
}
