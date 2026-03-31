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

// Function to print bits of a mask (for debugging)
void print_mask8(__mmask8 m) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (m >> i) & 1);
    }
}

void print_mask16(__mmask16 m) {
    for (int i = 15; i >= 0; i--) {
        printf("%d", (m >> i) & 1);
    }
}

void print_mask32(__mmask32 m) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (m >> i) & 1);
    }
}

void print_mask64(__mmask64 m) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (m >> i) & 1);
    }
}

int main() {
    int total_sum = 0;
    
#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend operations...\n");
    
    // Test 1: V64QImode - 64-byte vectors of 8-bit integers
    {
        __m512i a = _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        );
        
        __m512i b = _mm512_set_epi8(
            100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
            116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131,
            132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147,
            148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163
        );
        
        // Create a non-trivial mask by comparing elements
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask = _mm512_cmpgt_epi8_mask(a, cmp_val);
        
        // Perform the blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use the result to prevent optimization
        v64qi_result = result;
        
        // Extract and sum first few elements
        uint8_t res_arr[64];
        _mm512_storeu_si512((void*)res_arr, result);
        for (int i = 0; i < 8; i++) {
            total_sum += res_arr[i];
        }
        
        printf("  V64QI blend completed (mask bits for first 8 elements): ");
        for (int i = 0; i < 8; i++) {
            printf("%d", (mask >> i) & 1);
        }
        printf("\n");
    }
    
    // Test 2: V32HImode - 32-word vectors of 16-bit integers
    {
        __m512i a = _mm512_set_epi16(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
        );
        
        __m512i b = _mm512_set_epi16(
            100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
            116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131
        );
        
        // Create mask by comparing elements
        __m512i cmp_val = _mm512_set1_epi16(15);
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a, cmp_val);
        
        // Perform the blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use the result
        v32hi_result = result;
        
        // Extract and sum first few elements
        int16_t res_arr[32];
        _mm512_storeu_si512((void*)res_arr, result);
        for (int i = 0; i < 4; i++) {
            total_sum += res_arr[i];
        }
        
        printf("  V32HI blend completed\n");
    }
#endif // __AVX512BW__

#ifdef __AVX512F__
    printf("Testing AVX-512F blend operations...\n");
    
    // Test 3: V16SImode - 16-dword vectors of 32-bit integers
    {
        __m512i a = _mm512_set_epi32(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
        );
        
        __m512i b = _mm512_set_epi32(
            100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115
        );
        
        // Create mask by comparing elements
        __m512i cmp_val = _mm512_set1_epi32(7);
        __mmask16 mask = _mm512_cmpgt_epi32_mask(a, cmp_val);
        
        // Perform the blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use the result
        v16si_result = result;
        
        // Extract and sum first few elements
        int32_t res_arr[16];
        _mm512_storeu_si512((void*)res_arr, result);
        for (int i = 0; i < 4; i++) {
            total_sum += res_arr[i];
        }
        
        printf("  V16SI blend completed\n");
    }
    
    // Test 4: V8DImode - 8-qword vectors of 64-bit integers
    {
        __m512i a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i b = _mm512_set_epi64(100, 101, 102, 103, 104, 105, 106, 107);
        
        // Create mask by comparing elements
        __m512i cmp_val = _mm512_set1_epi64(3);
        __mmask8 mask = _mm512_cmpgt_epi64_mask(a, cmp_val);
        
        // Perform the blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use the result
        v8di_result = result;
        
        // Extract and sum first few elements
        int64_t res_arr[8];
        _mm512_storeu_si512((void*)res_arr, result);
        for (int i = 0; i < 2; i++) {
            total_sum += (int)res_arr[i];
        }
        
        printf("  V8DI blend completed\n");
    }
    
    // Test 5: V8DFmode - 8-qword vectors of double-precision floats
    {
        __m512d a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
        __m512d b = _mm512_set_pd(10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0);
        
        // Create mask by comparing elements
        __m512d cmp_val = _mm512_set1_pd(3.5);
        __mmask8 mask = _mm512_cmp_pd_mask(a, cmp_val, _CMP_GT_OQ);
        
        // Perform the blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use the result
        v8df_result = result;
        
        // Extract and sum first few elements
        double res_arr[8];
        _mm512_storeu_pd(res_arr, result);
        for (int i = 0; i < 2; i++) {
            total_sum += (int)res_arr[i];
        }
        
        printf("  V8DF blend completed\n");
    }
    
    // Test 6: V16SFmode - 16-dword vectors of single-precision floats
    {
        __m512 a = _mm512_set_ps(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
        );
        
        __m512 b = _mm512_set_ps(
            100.0f, 101.0f, 102.0f, 103.0f, 104.0f, 105.0f, 106.0f, 107.0f,
            108.0f, 109.0f, 110.0f, 111.0f, 112.0f, 113.0f, 114.0f, 115.0f
        );
        
        // Create mask by comparing elements
        __m512 cmp_val = _mm512_set1_ps(7.5f);
        __mmask16 mask = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
        
        // Perform the blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use the result
        v16sf_result = result;
        
        // Extract and sum first few elements
        float res_arr[16];
        _mm512_storeu_ps(res_arr, result);
        for (int i = 0; i < 4; i++) {
            total_sum += (int)res_arr[i];
        }
        
        printf("  V16SF blend completed\n");
    }
#endif // __AVX512F__

#ifdef __AVX512FP16__
    printf("Testing AVX-512FP16 blend operations...\n");
    
    // Test 7: V32HFmode - 32-word vectors of half-precision floats
    {
        // Initialize arrays for _Float16 values
        _Float16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            a_arr[i] = (_Float16)i;
            b_arr[i] = (_Float16)(i + 100);
        }
        
        __m512h a = _mm512_loadu_ph(a_arr);
        __m512h b = _mm512_loadu_ph(b_arr);
        
        // Create mask by comparing elements
        __m512h cmp_val = _mm512_set1_ph((_Float16)15.5);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
        
        // Perform the blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use the result
        v32hf_result = result;
        
        // Extract and sum first few elements
        _Float16 res_arr[32];
        _mm512_storeu_ph(res_arr, result);
        for (int i = 0; i < 4; i++) {
            total_sum += (int)res_arr[i];
        }
        
        printf("  V32HF blend completed\n");
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    printf("Testing AVX512-BF16 blend operations...\n");
    
    // Test 8: V32BFmode - 32-word vectors of brain float
    {
        // Initialize arrays for __bf16 values
        __bf16 a_arr[32], b_arr[32];
        for (int i = 0; i < 32; i++) {
            // Simple pattern for bfloat16
            uint16_t val = (i << 8) | (i & 0xFF);  // Create some pattern
            a_arr[i] = *(__bf16*)&val;
            
            val = ((i + 100) << 8) | ((i + 100) & 0xFF);
            b_arr[i] = *(__bf16*)&val;
        }
        
        __m512bh a = _mm512_loadu_bf16(a_arr);
        __m512bh b = _mm512_loadu_bf16(b_arr);
        
        // For BF16, we need to use the same intrinsic as FP16
        // Create mask by converting to float and comparing
        __m512 a_f32 = _mm512_cvtpbh_ps(a);
        __m512 b_f32 = _mm512_cvtpbh_ps(b);
        __m512 cmp_val = _mm512_set1_ps(15.5f);
        __mmask16 mask = _mm512_cmp_ps_mask(a_f32, cmp_val, _CMP_GT_OQ);
        
        // Extend mask from 16 to 32 bits for blend
        __mmask32 mask32 = _cvtu32_mask32(_cvtmask16_u32(mask));
        mask32 = mask32 | (mask32 << 16);  // Duplicate for upper and lower halves
        
        // Perform the blend using the FP16 intrinsic (same instruction)
        __m512bh result = _mm512_mask_blend_ph(mask32, a, b);
        
        // Use the result
        v32bf_result = result;
        
        // Extract and process first few elements
        __bf16 res_arr[32];
        _mm512_storeu_bf16(res_arr, result);
        for (int i = 0; i < 4; i++) {
            total_sum += (int)(*(uint16_t*)&res_arr[i]);
        }
        
        printf("  V32BF blend completed\n");
    }
#endif // __AVX512BF16__

    printf("Total sum (for preventing optimization): %d\n", total_sum);
    printf("All AVX-512 blend operations tested.\n");
    
    return 0;
}
