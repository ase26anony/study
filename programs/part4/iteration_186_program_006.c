#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

// Volatile global to prevent optimization
volatile __m512i global_vi;
volatile __m512  global_vf;
volatile __m512d global_vd;
volatile __m512h global_vh;

// Function to print results
void print_result(const char* name, long long result) {
    printf("%s: %lld\n", name, result);
}

int main() {
    long long total_sum = 0;
    
#ifdef __AVX512F__
    // V16SF: 16 single-precision floats
    {
        __m512 a = _mm512_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                                  9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
        __m512 b = _mm512_setr_ps(100.0f, 200.0f, 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f,
                                  900.0f, 1000.0f, 1100.0f, 1200.0f, 1300.0f, 1400.0f, 1500.0f, 1600.0f);
        
        // Create mask by comparing a < 10.0f
        __mmask16 mask = _mm512_cmp_ps_mask(a, _mm512_set1_ps(10.0f), _CMP_LT_OQ);
        
        // Perform blend
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        
        // Use result to prevent optimization
        global_vf = result;
        
        // Horizontal sum
        float sum = _mm512_reduce_add_ps(result);
        total_sum += (long long)sum;
        print_result("V16SF", (long long)sum);
    }
    
    // V8DF: 8 double-precision floats
    {
        __m512d a = _mm512_setr_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
        __m512d b = _mm512_setr_pd(100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0);
        
        // Create mask by comparing a > 4.0
        __mmask8 mask = _mm512_cmp_pd_mask(a, _mm512_set1_pd(4.0), _CMP_GT_OQ);
        
        // Perform blend
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        
        // Use result
        global_vd = result;
        
        // Horizontal sum
        double sum = _mm512_reduce_add_pd(result);
        total_sum += (long long)sum;
        print_result("V8DF", (long long)sum);
    }
    
    // V16SI: 16 32-bit integers
    {
        __m512i a = _mm512_setr_epi32(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        __m512i b = _mm512_setr_epi32(100, 200, 300, 400, 500, 600, 700, 800, 
                                      900, 1000, 1100, 1200, 1300, 1400, 1500, 1600);
        
        // Create mask by comparing a == some values
        __m512i cmp_val = _mm512_set1_epi32(8);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, cmp_val);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Extract and sum first 4 elements
        int sum = _mm512_reduce_add_epi32(result);
        total_sum += sum;
        print_result("V16SI", sum);
    }
    
    // V8DI: 8 64-bit integers
    {
        __m512i a = _mm512_setr_epi64(1, 2, 3, 4, 5, 6, 7, 8);
        __m512i b = _mm512_setr_epi64(100, 200, 300, 400, 500, 600, 700, 800);
        
        // Create mask using comparison
        __m512i cmp_val = _mm512_set1_epi64(3);
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, cmp_val);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Extract and sum first 4 elements
        long long sum = _mm512_reduce_add_epi64(result);
        total_sum += sum;
        print_result("V8DI", sum);
    }
#endif // __AVX512F__

#ifdef __AVX512BW__
    // V64QI: 64 8-bit integers
    {
        // Initialize with pattern
        uint8_t a_data[64];
        uint8_t b_data[64];
        for (int i = 0; i < 64; i++) {
            a_data[i] = i;
            b_data[i] = 100 + i;
        }
        
        __m512i a = _mm512_loadu_si512((const __m512i*)a_data);
        __m512i b = _mm512_loadu_si512((const __m512i*)b_data);
        
        // Create mask: select where a < 32
        __m512i cmp_val = _mm512_set1_epi8(32);
        __mmask64 mask = _mm512_cmplt_epi8_mask(a, cmp_val);
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Sum all elements
        int sum = 0;
        uint8_t res_data[64];
        _mm512_storeu_si512((__m512i*)res_data, result);
        for (int i = 0; i < 64; i++) {
            sum += res_data[i];
        }
        total_sum += sum;
        print_result("V64QI", sum);
    }
    
    // V32HI: 32 16-bit integers
    {
        __m512i a = _mm512_setr_epi16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                      17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32);
        __m512i b = _mm512_setr_epi16(100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 
                                      1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000,
                                      2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000, 3100, 3200);
        
        // Create mask: select where a is even
        __m512i zero = _mm512_setzero_si512();
        __mmask32 mask = _mm512_test_epi16_mask(a, _mm512_set1_epi16(1));
        mask = ~mask;  // Invert to select even numbers
        
        // Perform blend
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        
        // Use result
        global_vi = result;
        
        // Sum all elements
        int sum = 0;
        int16_t res_data[32];
        _mm512_storeu_si512((__m512i*)res_data, result);
        for (int i = 0; i < 32; i++) {
            sum += res_data[i];
        }
        total_sum += sum;
        print_result("V32HI", sum);
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
            b_data[i] = (_Float16)((i + 1) * 10);
        }
        
        __m512h a = _mm512_loadu_ph(a_data);
        __m512h b = _mm512_loadu_ph(b_data);
        
        // Create mask by comparing a < 16.0
        __m512h cmp_val = _mm512_set1_ph(16.0);
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_vh = result;
        
        // Convert to float and sum
        float sum = 0;
        _Float16 res_data[32];
        _mm512_storeu_ph(res_data, result);
        for (int i = 0; i < 32; i++) {
            sum += (float)res_data[i];
        }
        total_sum += (long long)sum;
        print_result("V32HF", (long long)sum);
    }
#endif // __AVX512FP16__

#ifdef __AVX512BF16__
    // V32BF: 32 brain float
    // Note: BF16 uses the same intrinsics as FP16 for blend operations
    {
        // Initialize with pattern
        __bfloat16 a_data[32];
        __bfloat16 b_data[32];
        for (int i = 0; i < 32; i++) {
            a_data[i] = bfloat16_from_float((float)(i + 1));
            b_data[i] = bfloat16_from_float((float)((i + 1) * 10));
        }
        
        // Load as FP16 vectors (same storage)
        __m512h a = _mm512_loadu_ph((const _Float16*)a_data);
        __m512h b = _mm512_loadu_ph((const _Float16*)b_data);
        
        // Create mask by comparing (convert to float for comparison)
        __m512h cmp_val = _mm512_set1_ph(bfloat16_to_float(bfloat16_from_float(16.0)));
        __mmask32 mask = _mm512_cmp_ph_mask(a, cmp_val, _CMP_LT_OQ);
        
        // Perform blend
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        
        // Use result
        global_vh = result;
        
        // Convert to float and sum
        float sum = 0;
        __bfloat16 res_data[32];
        _mm512_storeu_ph((_Float16*)res_data, result);
        for (int i = 0; i < 32; i++) {
            sum += bfloat16_to_float(res_data[i]);
        }
        total_sum += (long long)sum;
        print_result("V32BF", (long long)sum);
    }
#endif // __AVX512BF16__

    printf("Total sum: %lld\n", total_sum);
    return 0;
}
